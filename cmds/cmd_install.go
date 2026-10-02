package cmds

import (
	"fmt"
	"io/fs"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/dag"
	"github.com/celer-pkg/celer/depcheck"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"

	"github.com/spf13/cobra"
)

type installCmd struct {
	celer          *configs.Celer
	dev            bool
	force          bool
	recursive      bool
	cleanSource    bool
	prefer         string
	dag            string
	jobs           int
	verbose        bool
	jobsChanged    bool
	verboseChanged bool

	// The plan given by --dag: the workspace it was exported for, the sources to
	// pin and the build hashes to expect.
	dagReport *dag.Report

	// Build hashes expected by --dag, keyed by name@version.
	dagHashes map[string]string

	// Nodes an exported DAG reports as already cached (not scheduled).
	dagCached map[string]bool
}

func (i *installCmd) Command(celer *configs.Celer) *cobra.Command {
	i.celer = celer
	command := &cobra.Command{
		Use:   "install",
		Short: "Install package(s).",
		Long: `Install package(s).

This command installs packages from available ports, either from the global
ports repository or from project-specific ports. The package name must be
specified in name@version format.

FEATURES:
  • Install packages with dependency resolution
  • Support for development dependencies
  • Force reinstallation with dependency handling
  • Best-effort package cache storing by default
  • Parallel build support
  • Circular dependency detection
  • Version conflict checking

FLAGS:
  -d, --dev          Install as development dependency
  -f, --force        Force reinstallation (uninstall first if exists). The
                     rebuilt artifact overwrites the pkgcache entry.
      --clean-source With --force, also reset the source repo (discards
                     uncommitted changes).
      --prefer       Install from one path only: source, package, pkgcache or
                     devcache.
      --dag          Install a node of the plan from 'celer deploy --dag'.
  -r, --recursive    With --force, recursively reinstall dependencies
  -j, --jobs         Number of parallel build jobs (default: system cores)
  -v, --verbose      Enable verbose output for debugging

EXAMPLES:
  celer install opencv@4.8.0
  celer install opencv@4.8.0 eigen@3.4.0
  celer install --dev gtest@1.12.1
  celer install --force --recursive boost@1.82.0
  celer install --force --clean-source opencv@4.8.0
  celer install --jobs=8 --verbose opencv@4.8.0
  celer install --prefer=pkgcache opencv@4.8.0  # restore only, fail on a miss
  celer install --prefer=source opencv@4.8.0    # build only, then store
  celer install --dag=out/dag.json opencv@4.8.0 # follow an exported DAG`,
		Args: cobra.MinimumNArgs(1),
		RunE: func(cmd *cobra.Command, args []string) error {
			i.jobsChanged = cmd.Flags().Changed("jobs")
			i.verboseChanged = cmd.Flags().Changed("verbose")
			return i.runInstall(args)
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	flags := command.Flags()
	flags.BoolVarP(&i.dev, "dev", "d", false, "install in dev mode.")
	flags.BoolVarP(&i.force, "force", "f", false, "try to uninstall before installation.")
	flags.BoolVarP(&i.recursive, "recursive", "r", false, "combine with --force, recursively reinstall dependencies.")
	flags.BoolVarP(&i.cleanSource, "clean-source", "", false, "combine with --force, also reset the source repo (discards uncommitted changes).")
	flags.StringVar(&i.prefer, "prefer", "", "install from one path only: source, package, pkgcache or devcache.")
	flags.StringVar(&i.dag, "dag", "", "install a node of the plan from 'celer deploy --dag'.")
	flags.IntVarP(&i.jobs, "jobs", "j", i.celer.Jobs(), "the number of jobs to run in parallel.")
	flags.BoolVarP(&i.verbose, "verbose", "v", false, "verbose detail information.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (i *installCmd) runInstall(nameVersions []string) error {
	// Validate and clean input before initialization so input errors are reported first.
	cleanedNameVersions := make([]string, 0, len(nameVersions))
	for _, nameVersion := range nameVersions {
		cleanedNameVersion, err := i.validateAndCleanInput(nameVersion)
		if err != nil {
			return logger.PrintError(err, "invalid package specification: %s", nameVersion)
		}
		cleanedNameVersions = append(cleanedNameVersions, cleanedNameVersion)
	}

	// A plan carries the workspace it was exported for: adopt it before celer is
	// initialized, so the toolchain, the devcache, the expression variables and
	// every cache key below are derived from the plan instead of celer.toml.
	if i.dag != "" {
		report, err := dag.Parse(i.dag, i.celer.Version())
		if err != nil {
			return logger.PrintError(err, "failed to read the build plan %s.", i.dag)
		}

		// Adopt the identity of the plan, one setter per property. Each setter saves
		// celer.toml, so a failure halfway leaves the identity partly adopted.
		if err := i.celer.SetPlatform(report.Platform); err != nil {
			return logger.PrintError(err, "failed to adopt the platform of %s.", i.dag)
		}
		if err := i.celer.SetProject(report.Project); err != nil {
			return logger.PrintError(err, "failed to adopt the project of %s.", i.dag)
		}
		if err := i.celer.SetBuildType(report.BuildType); err != nil {
			return logger.PrintError(err, "failed to adopt the build type of %s.", i.dag)
		}
		logger.PrintInfo("build identity taken from %s: platform %s, project %s, build type %s",
			i.dag, report.Platform, report.Project, report.BuildType)
		i.dagReport = report
	}

	if err := i.celer.Init(); err != nil {
		return logger.PrintError(err, "failed to initialize celer.")
	}

	// Check git first as it's needed for cloning and reading commit hashes,
	// and must check tool after celer initialized, since "downloads" will be assign value after init.
	if err := buildtools.CheckTools(i.celer, "git"); err != nil {
		return logger.PrintError(err, "failed to check build tool: git")
	}

	if err := i.overrideFlags(); err != nil {
		return logger.PrintError(err, "invalid install options.")
	}

	// Pin the sources of the plan, now that celer is initialized and before
	// anything is installed.
	if i.dag != "" {
		if err := i.applyDAG(); err != nil {
			return logger.PrintError(err, "failed to apply the build DAG.")
		}
	}

	// Install port one by one.
	for _, nameVersion := range cleanedNameVersions {
		if err := i.install(nameVersion); err != nil {
			return err
		}
	}

	return nil
}

// validateAndCleanInput validates and cleans the package name@version input.
func (i *installCmd) validateAndCleanInput(nameVersion string) (string, error) {
	if strings.TrimSpace(nameVersion) == "" {
		return "", fmt.Errorf("package name cannot be empty")
	}

	// In Windows PowerShell, when handling completion,
	// "`" is automatically added as an escape character before the "@".
	// We need to remove this escape character.
	cleaned := strings.ReplaceAll(nameVersion, "`", "")
	cleaned = strings.TrimSpace(cleaned)

	parts := strings.Split(cleaned, "@")
	if len(parts) != 2 {
		return "", fmt.Errorf("package must be specified in name@version format (e.g., opencv@4.8.0)")
	}

	name := strings.TrimSpace(parts[0])
	version := strings.TrimSpace(parts[1])

	if name == "" {
		return "", fmt.Errorf("package name cannot be empty")
	}
	if version == "" {
		return "", fmt.Errorf("package version cannot be empty")
	}

	return name + "@" + version, nil
}

func (i *installCmd) install(nameVersion string) error {
	platformName := expr.If(i.celer.Platform().GetName() != "", i.celer.Platform().GetName(), "native")

	// Display install header.
	logger.Println(logger.Title, "=======================================================================")
	logger.Printf(logger.Title, "🚀 start to install %s\n", nameVersion)
	logger.Printf(logger.Title, "📌 platform: %s\n", platformName)
	logger.Printf(logger.Title, "📌 product: %s\n", i.celer.Project().GetName())
	logger.Println(logger.Title, "=======================================================================")

	// Init the port.
	var port = configs.Port{
		DevDep: i.dev,
	}
	if err := port.Init(i.celer, nameVersion); err != nil {
		if errors.Is(err, errors.ErrPortNotFound) {
			format := "port %s is not yet available - consider adding it now ?"
			return logger.PrintError(fmt.Errorf(format, nameVersion), "failed to install %s", nameVersion)
		}
		return logger.PrintError(err, "failed to init %s", nameVersion)
	}

	// Check circular dependence and version conclict.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(i.celer, port); err != nil {
		return logger.PrintError(err, "failed to check circular dependence.")
	}
	if err := depcheck.CheckConflict(i.celer, port); err != nil {
		return logger.PrintError(err, "failed to check version conflict.")
	}

	// With a DAG the expected build hash of this node is known up front: a
	// mismatch means the plan no longer describes this workspace, so report it
	// instead of silently building a different revision.
	if i.dag != "" {
		expected, ok := i.dagHashes[nameVersion]
		if !ok {
			if i.dagCached[nameVersion] {
				return logger.PrintError(fmt.Errorf("%s is already in the pkgcache, it is not scheduled in %s", nameVersion, i.dag),
					"failed to install %s", nameVersion)
			}
			return logger.PrintError(fmt.Errorf("%s is not part of %s", nameVersion, i.dag),
				"failed to install %s", nameVersion)
		}
		buildhash, err := port.BuildHash()
		if err != nil {
			return logger.PrintError(err, "failed to calculate the build hash of %s", nameVersion)
		}
		if buildhash != expected {
			return logger.PrintError(
				fmt.Errorf("build hash mismatch: %s expects %s but this workspace computes %s", i.dag, expected, buildhash),
				"failed to install %s", nameVersion)
		}
	}

	// Parse the strict install path; empty means the default fallback chain.
	prefer, err := configs.ParseInstallPrefer(i.prefer)
	if err != nil {
		return logger.PrintError(err, "invalid --prefer value: %s", i.prefer)
	}

	// Do install.
	options := configs.InstallOptions{
		Force:       i.force,
		Recursive:   i.recursive,
		CleanSource: i.cleanSource,
		Prefer:      prefer,
	}
	fromWhere, err := port.Install(options)
	if err != nil {
		return logger.PrintError(err, "failed to install %s", nameVersion)
	}

	// An explicitly requested path that could not serve this port is a failure:
	// the default chain is the only mode that may fall back to another path.
	if prefer != configs.PreferNone && fromWhere == "" {
		return logger.PrintError(fmt.Errorf("%s is not available from %s", nameVersion, i.prefer),
			"failed to install %s", nameVersion)
	}

	if fromWhere != "" {
		if port.DevDep {
			logger.PrintSuccess("install %s from %s as dev successfully.", nameVersion, fromWhere)
		} else {
			logger.PrintSuccess("install %s from %s successfully.", nameVersion, fromWhere)
		}
	} else {
		if port.DevDep {
			logger.PrintSuccess("install %s as dev successfully.", nameVersion)
		} else {
			logger.PrintSuccess("install %s successfully.", nameVersion)
		}
	}

	return nil
}

// applyDAG pins the source revisions recorded in a DAG exported by
// `celer deploy --dag`, so every agent builds the exact revisions that were
// resolved at export time instead of whatever it happens to find locally.
func (i *installCmd) applyDAG() error {
	// The plan is all a build follows: the source revision to pin per port, the
	// build hash expected per scheduled node, and which nodes pkgcache supplies
	// instead of building them.
	pins := i.dagReport.PinSources()
	configs.PinSources(pins)
	i.dagHashes = i.dagReport.DagHashs()
	i.dagCached = i.dagReport.DagCached()

	logger.PrintInfo("pinned %d source revisions from %s", len(pins), i.dag)
	return nil
}

func (i *installCmd) overrideFlags() error {
	if i.cleanSource && !i.force {
		return fmt.Errorf("--clean-source must be used together with --force")
	}

	if i.jobsChanged {
		if i.jobs <= 0 {
			return fmt.Errorf("--jobs must be greater than 0")
		}
		i.celer.SetJobs(i.jobs)
	}

	if i.verboseChanged {
		i.celer.SetVerbose(i.verbose)
	}

	return nil
}

func (i *installCmd) buildSuggestions(suggestions *[]string, portDir string, toComplete string) {
	err := filepath.WalkDir(portDir, func(path string, entity fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if !entity.IsDir() && entity.Name() == "port.toml" {
			// For example: ports/t/testlib/1.0.0/port.toml
			portDir := filepath.Dir(path)                   // ports/t/testlib/1.0.0
			libVersion := filepath.Base(portDir)            // 1.0.0
			libName := filepath.Base(filepath.Dir(portDir)) // testlib
			nameVersion := libName + "@" + libVersion

			if strings.HasPrefix(nameVersion, toComplete) {
				*suggestions = append(*suggestions, nameVersion)
			}
		}

		return nil
	})
	if err != nil {
		logger.PrintError(err, "failed to read %s -> %s.\n", portDir, err)
		return
	}
}

func (i *installCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	if fileio.PathExists(dirs.PortsDir) {
		i.buildSuggestions(&suggestions, dirs.PortsDir, toComplete)
	}

	projectName := i.celer.GetProjectName()
	if projectName != "" {
		projectPortsDir := filepath.Join(dirs.ConfProjectsDir, projectName)
		if fileio.PathExists(projectPortsDir) {
			i.buildSuggestions(&suggestions, projectPortsDir, toComplete)
		}
	}

	// Support flags completion.
	commands := []string{
		"--dev", "-d",
		"--force", "-f",
		"--clean-source",
		"--recursive", "-r",
		"--jobs", "-j",
		"--verbose", "-v",
	}

	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
