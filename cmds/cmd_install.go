package cmds

import (
	"fmt"
	"io/fs"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/dag"
	"github.com/celer-pkg/celer/pkgs/dirs"
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
	jobs           int
	verbose        bool
	jobsChanged    bool
	verboseChanged bool

	exportDag string
	applyDag  string
	dagInfo   *dag.DagInfo
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
      --export-dag   Export the build DAG of a port.
      --apply-dag    Install a node of the schedule from 'celer deploy --export-dag'.
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
  celer install --export-dag=out/dag.json opencv@4.8.0 # export DAG for a port
  celer install --apply-dag=out/dag.json opencv@4.8.0 # follow an exported DAG
  `,
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
	flags.IntVarP(&i.jobs, "jobs", "j", i.celer.Jobs(), "the number of jobs to run in parallel.")
	flags.BoolVarP(&i.verbose, "verbose", "v", false, "verbose detail information.")
	flags.StringVar(&i.exportDag, "export-dag", "", "export a build DAG of a port.")
	flags.StringVar(&i.applyDag, "apply-dag", "", "install a port with a DAG.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (i *installCmd) runInstall(nameVersions []string) error {
	// Validate and clean input before initialization so input errors are reported first.
	roots := make([]string, 0, len(nameVersions))
	for _, nameVersion := range nameVersions {
		port, err := i.validateAndClean(nameVersion)
		if err != nil {
			return logger.PrintError(err, "invalid package specification: %s", nameVersion)
		}
		roots = append(roots, port)
	}

	if err := i.celer.Init(); err != nil {
		return logger.PrintError(err, "failed to initialize celer.")
	}

	// Export Dag: Instead of installation, celer export a DAG of ports that you want to install.
	// Apply Dag: Set build params that defined in DAG.
	if i.exportDag != "" {
		return dag.Export(i.celer, roots, i.force, i.exportDag)
	} else if i.applyDag != "" {
		dagInfo, err := dag.Import(i.celer, i.applyDag)
		if err != nil {
			return logger.PrintError(err, "failed to read the build DAG %s.", i.applyDag)
		}
		i.dagInfo = dagInfo
	}

	// Check git first as it's needed for cloning and reading commit hashes,
	// and must check tool after celer initialized, since "downloads" will
	// be assign value after init.
	if err := buildtools.CheckTools(i.celer, "git"); err != nil {
		return logger.PrintError(err, "failed to check build tool: git")
	}

	if err := i.overrideFlags(); err != nil {
		return logger.PrintError(err, "invalid install options.")
	}

	// Install port one by one.
	for _, nameVersion := range roots {
		if err := i.install(nameVersion); err != nil {
			return err
		}
	}
	return nil
}

// validateAndClean validates and cleans the package name@version input.
func (i *installCmd) validateAndClean(nameVersion string) (string, error) {
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
	prefer, err := configs.ParseInstallPrefer(i.prefer)
	if err != nil {
		return logger.PrintError(err, "invalid --prefer value: %s", i.prefer)
	}

	options := configs.InstallOptions{
		Force:       i.force,
		Recursive:   i.recursive,
		CleanSource: i.cleanSource,
		Prefer:      prefer,
	}

	expect := dag.Expectation{Banner: true}
	if i.dagInfo != nil {
		// A node of the schedule is either one of its scheduled nodes, with the build
		// hash it expects, or a node pkgcache already had, which is not scheduled
		// and cannot be installed here.
		expected, ok := i.dagInfo.DagHashes[nameVersion]
		if !ok {
			if i.dagInfo.DagCached[nameVersion] {
				return logger.PrintError(fmt.Errorf("%s is already in the pkgcache, it is not scheduled in %s", nameVersion, i.applyDag),
					"failed to install %s", nameVersion)
			}
			return logger.PrintError(fmt.Errorf("%s is not part of %s", nameVersion, i.applyDag),
				"failed to install %s", nameVersion)
		}
		expect.DagPath = i.applyDag
		expect.BuildHash = expected
	}

	_, err = dag.Install(i.celer, nameVersion, i.dev, options, expect)
	return err
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
		"--export-dag",
		"--apply-dag",
	}

	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
