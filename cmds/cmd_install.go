package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"io/fs"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type installCmd struct {
	celer      *configs.Celer
	dev        bool
	force      bool
	recursive  bool
	storeCache bool
	cacheToken string
	jobs       int
	verbose    bool
}

func (i *installCmd) Command(celer *configs.Celer) *cobra.Command {
	i.celer = celer
	command := &cobra.Command{
		Use:   "install",
		Short: "Install a package.",
		Long: `Install a package.

This command installs packages from available ports, either from the global
ports repository or from project-specific ports. The package name must be
specified in name@version format.

FEATURES:
  • Install packages with dependency resolution
  • Support for development dependencies
  • Force reinstallation with dependency handling
  • Binary cache integration
  • Parallel build support
  • Circular dependency detection
  • Version conflict checking

FLAGS:
  -d, --dev         Install as development dependency
  -f, --force       Force reinstallation (uninstall first if exists)
  -r, --recursive   With --force, recursively reinstall dependencies
  -s, --store-cache Store build artifacts in binary cache after installation
  -t, --cache-token Authentication token for binary cache operations
  -j, --jobs        Number of parallel build jobs (default: system cores)
  -v, --verbose     Enable verbose output for debugging

EXAMPLES:
  celer install opencv@4.8.0
  celer install --dev gtest@1.12.1
  celer install --force --recursive boost@1.82.0
  celer install --store-cache --cache-token abc123 eigen@3.4.0
  celer install --jobs 8 --verbose opencv@4.8.0`,
		Args: cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			i.runInstall(args[0])
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	flags := command.Flags()
	flags.BoolVarP(&i.dev, "dev", "d", false, "install in dev mode.")
	flags.BoolVarP(&i.force, "force", "f", false, "try to uninstall before installation.")
	flags.BoolVarP(&i.recursive, "recursive", "r", false, "combine with --force, recursively reinstall dependencies.")
	flags.BoolVarP(&i.storeCache, "store-cache", "s", false, "store artifact into cache after installation.")
	flags.StringVarP(&i.cacheToken, "cache-token", "t", "", "combine with --store-cache, specify cache token.")
	flags.IntVarP(&i.jobs, "jobs", "j", i.celer.Jobs(), "the number of jobs to run in parallel.")
	flags.BoolVarP(&i.verbose, "verbose", "v", false, "verbose detail information.")

	return command
}

func (i *installCmd) runInstall(nameVersion string) {
	if err := i.celer.Init(); err != nil {
		configs.PrintError(err, "Failed to initialize celer.")
		return
	}

	// Validate and clean input.
	cleanedNameVersion, err := i.validateAndCleanInput(nameVersion)
	if err != nil {
		configs.PrintError(err, "Invalid package specification.")
		return
	}

	i.install(cleanedNameVersion)
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

func (i *installCmd) install(nameVersion string) {
	// Display install header.
	fmt.Printf("%s %s %s %s\n",
		color.Color("Install", color.Blue, color.Bold),
		color.Color(nameVersion, color.BrightMagenta, color.Bold),
		color.Color("with platform", color.Blue, color.Bold),
		color.Color(i.celer.Global.Platform, color.BrightMagenta, color.Bold),
	)
	titleLen := len(fmt.Sprintf("Install %s with platform %s",
		nameVersion, i.celer.Global.Platform,
	))
	color.Println(color.Line, strings.Repeat("-", titleLen))

	// Overwrite global config.
	if i.jobs != i.celer.Global.Jobs {
		i.celer.Global.Jobs = i.jobs
	}
	i.celer.Global.Verbose = i.verbose

	if err := i.celer.Setup(); err != nil {
		configs.PrintError(err, "Failed to setup celer.")
		return
	}

	// Parse name and version (already validated)
	parts := strings.Split(nameVersion, "@")
	name, version := parts[0], parts[1]

	portInProject := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().GetName(), name, version, "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, name, version, "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		configs.PrintError(fmt.Errorf("port %s not found in available repositories", nameVersion), "Failed to install %s.", nameVersion)
		return
	}

	// Install the port.
	var port configs.Port
	port.DevDep = i.dev
	if err := port.Init(i.celer, nameVersion); err != nil {
		configs.PrintError(err, "failed to init %s.", nameVersion)
		return
	}

	// Check circular dependence.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(i.celer, port); err != nil {
		configs.PrintError(err, "failed to check circular dependence.")
		return
	}

	// Check version conflict.
	if err := depcheck.CheckConflict(i.celer, port); err != nil {
		configs.PrintError(err, "failed to check version conflict.")
		return
	}

	if err := buildtools.CheckTools(i.celer, "git"); err != nil {
		configs.PrintError(err, "failed to check build tool: git.")
		return
	}

	// Do install.
	options := configs.InstallOptions{
		Force:      i.force,
		Recursive:  i.recursive,
		StoreCache: i.storeCache,
		CacheToken: i.cacheToken,
	}
	fromWhere, err := port.Install(options)
	if err != nil {
		configs.PrintError(err, "failed to install %s.", nameVersion)
		return
	}
	if fromWhere != "" {
		if port.DevDep {
			configs.PrintSuccess("install %s from %s as dev successfully.", nameVersion, fromWhere)
		} else {
			configs.PrintSuccess("install %s from %s successfully.", nameVersion, fromWhere)
		}
	} else {
		if port.DevDep {
			configs.PrintSuccess("install %s as dev successfully.", nameVersion)
		} else {
			configs.PrintSuccess("install %s successfully.", nameVersion)
		}
	}
}

func (i *installCmd) buildSuggestions(suggestions *[]string, portDir string, toComplete string) {
	err := filepath.WalkDir(portDir, func(path string, entity fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if !entity.IsDir() && entity.Name() == "port.toml" {
			libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
			libVersion := filepath.Base(filepath.Dir(path))
			nameVersion := libName + "@" + libVersion

			if strings.HasPrefix(nameVersion, toComplete) {
				*suggestions = append(*suggestions, nameVersion)
			}
		}

		return nil
	})
	if err != nil {
		configs.PrintError(err, "failed to read %s.\n %s.\n", portDir, err)
		return
	}
}

func (i *installCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	if fileio.PathExists(dirs.PortsDir) {
		i.buildSuggestions(&suggestions, dirs.PortsDir, toComplete)
	}

	projectPortsDir := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().GetName())
	if fileio.PathExists(projectPortsDir) {
		i.buildSuggestions(&suggestions, projectPortsDir, toComplete)
	}

	// Support flags completion.
	commands := []string{
		"--dev", "-d",
		"--force", "-f",
		"--recursive", "-r",
		"--store-cache", "-s",
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
