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
  • Package cache integration
  • Parallel build support
  • Circular dependency detection
  • Version conflict checking

FLAGS:
  -d, --dev         Install as development dependency
  -f, --force       Force reinstallation (uninstall first if exists)
  -r, --recursive   With --force, recursively reinstall dependencies
  -s, --store-cache Store build artifacts in package cache after installation
  -t, --cache-token Authentication token for package cache operations
  -j, --jobs        Number of parallel build jobs (default: system cores)
  -v, --verbose     Enable verbose output for debugging

EXAMPLES:
  celer install opencv@4.8.0
  celer install --dev gtest@1.12.1
  celer install --force --recursive boost@1.82.0
  celer install --store-cache --cache-token abc123 eigen@3.4.0
  celer install --jobs 8 --verbose opencv@4.8.0`,
		Args: cobra.ExactArgs(1),
		RunE: func(cmd *cobra.Command, args []string) error {
			return i.runInstall(args[0])
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

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (i *installCmd) runInstall(nameVersion string) error {
	if err := i.celer.Init(); err != nil {
		return configs.PrintError(err, "failed to initialize celer.")
	}

	// Validate and clean input.
	cleanedNameVersion, err := i.validateAndCleanInput(nameVersion)
	if err != nil {
		return configs.PrintError(err, "invalid package specification.")
	}

	return i.install(cleanedNameVersion)
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
	// Display install header.
	color.Println(color.Title, "=======================================================================")
	color.Printf(color.Title, "🚀 start to install %s\n", nameVersion)
	color.Printf(color.Title, "🛠️  platform: %s\n", i.celer.Global.Platform)
	color.Println(color.Title, "=======================================================================")

	// Check git first as it's needed for cloning and reading commit hashes
	if err := buildtools.CheckTools(i.celer, "git"); err != nil {
		return configs.PrintError(err, "failed to check build tool: git")
	}

	// Overwrite global config.
	if i.jobs != i.celer.Global.Jobs {
		i.celer.Global.Jobs = i.jobs
	}
	i.celer.Global.Verbose = i.verbose

	// Parse name and version (already validated)
	parts := strings.Split(nameVersion, "@")
	name, version := parts[0], parts[1]

	portInProject := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().GetName(), name, version, "port.toml")
	portInPorts := dirs.GetPortPath(name, version)
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		err := fmt.Errorf("port %s is not yet available in the ports collection.\n ⭐⭐⭐ Welcome to contribute to the ports. ⭐⭐⭐", nameVersion)
		return configs.PrintError(err, "failed to install %s.", nameVersion)
	}

	// Install the port.
	var port configs.Port
	port.DevDep = i.dev
	if err := port.Init(i.celer, nameVersion); err != nil {
		return configs.PrintError(err, "failed to init %s.", nameVersion)
	}

	// Check circular dependence.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(i.celer, port); err != nil {
		return configs.PrintError(err, "failed to check circular dependence.")
	}

	// Check version conflict.
	if err := depcheck.CheckConflict(i.celer, port); err != nil {
		return configs.PrintError(err, "failed to check version conflict.")
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
		return configs.PrintError(err, "failed to install %s.", nameVersion)
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
		configs.PrintError(err, "failed to read %s -> %s.\n", portDir, err)
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
