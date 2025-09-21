package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type installCmd struct {
	celer      *configs.Celer
	buildType  string
	dev        bool
	force      bool
	recurse    bool
	storeCache bool
	cacheToken string
}

func (i installCmd) Command() *cobra.Command {
	i.celer = configs.NewCeler()
	if err := i.celer.Init(); err != nil {
		configs.PrintError(err, "init celer error.")
		os.Exit(1)
	}

	command := &cobra.Command{
		Use:   "install",
		Short: "Install a package.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			if err := i.celer.Platform().Setup(); err != nil {
				configs.PrintError(err, "setup platform error.")
				os.Exit(1)
			}

			if err := i.celer.GenerateToolchainFile(false); err != nil {
				configs.PrintError(err, "generate toolchain file error.")
				os.Exit(1)
			}

			// Set offline mode.
			buildtools.Offline = i.celer.Global.Offline
			configs.Offline = i.celer.Global.Offline

			i.install(args[0])
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().StringVarP(&i.buildType, "build-type", "b", i.celer.Global.BuildType, "Install with build type.")
	command.Flags().BoolVarP(&i.dev, "dev", "d", false, "Install in dev mode.")
	command.Flags().BoolVarP(&i.force, "force", "f", false, "Try to uninstall before installation.")
	command.Flags().BoolVarP(&i.recurse, "recurse", "r", false, "Combine with --force, recursively reinstall dependencies.")
	command.Flags().BoolVarP(&i.storeCache, "store-cache", "s", false, "Store artifact into cache after installation.")
	command.Flags().StringVarP(&i.cacheToken, "cache-token", "t", "", "Combine with --store-cache, specify cache token.")

	return command
}

func (i installCmd) install(nameVersion string) {
	// Use build_type from `celer.toml` if not specified.
	if i.buildType == "" {
		i.buildType = i.celer.Global.BuildType
	}

	if err := i.celer.Platform().Setup(); err != nil {
		configs.PrintError(err, "setup platform error:")
		os.Exit(1)
	}

	// In Windows PowerShell, when handling completion,
	// "`" is automatically added as an escape character before the "@".
	// We need to remove this escape character.
	nameVersion = strings.ReplaceAll(nameVersion, "`", "")

	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		configs.PrintError(fmt.Errorf("invalid name and version"), "install %s failed.", nameVersion)
		os.Exit(1)
	}

	portInProject := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().Name, parts[0], parts[1], "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		configs.PrintError(fmt.Errorf("port %s is not found", nameVersion), "%s install failed.", nameVersion)
		os.Exit(1)
	}

	// Install the port.
	var port configs.Port
	port.DevDep = i.dev
	port.Reinstall = i.force
	port.Recurse = i.recurse
	port.StoreCache = i.storeCache
	port.CacheToken = i.cacheToken
	if err := port.Init(i.celer, nameVersion, i.buildType); err != nil {
		configs.PrintError(err, "init %s failed.", nameVersion)
		os.Exit(1)
	}

	// Check circular dependence.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(i.celer, port); err != nil {
		configs.PrintError(err, "check circular dependence failed.")
		os.Exit(1)
	}

	// Check version conflict.
	if err := depcheck.CheckConflict(i.celer, port); err != nil {
		configs.PrintError(err, "check version conflict failed.")
		os.Exit(1)
	}

	// Do install.
	fromWhere, err := port.Install()
	if err != nil {
		configs.PrintError(err, "install %s failed.", nameVersion)
		os.Exit(1)
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

func (i installCmd) buildSuggestions(suggestions *[]string, portDir string, toComplete string) {
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
		configs.PrintError(err, "failed to read %s: %s.\n", portDir, err)
		os.Exit(1)
	}
}

func (i installCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	if fileio.PathExists(dirs.PortsDir) {
		i.buildSuggestions(&suggestions, dirs.PortsDir, toComplete)
	}

	projectPortsDir := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().Name)
	if fileio.PathExists(projectPortsDir) {
		i.buildSuggestions(&suggestions, projectPortsDir, toComplete)
	}

	// Support flags completion.
	commands := []string{
		"--dev", "-d",
		"--build-type", "-b",
		"--force", "-f",
		"--recurse", "-r",
		"--store-cache", "-s",
	}

	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
