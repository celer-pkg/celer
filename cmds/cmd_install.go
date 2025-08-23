package cmds

import (
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"io/fs"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type installCmd struct {
	celer     *configs.Celer
	buildType string
	dev       bool
	force     bool
	recurse   bool
}

func (i installCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "install",
		Short: "Install a package.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			i.install(args[0])
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&i.dev, "dev", "d", false, "install package as runtime dev mode.")
	command.Flags().StringVarP(&i.buildType, "build-type", "b", "release", "install package with build type.")
	command.Flags().BoolVarP(&i.force, "force", "f", false, "uninstall package before install again.")
	command.Flags().BoolVarP(&i.recurse, "recurse", "r", false, "uninstall package recursively before install again.")

	return command
}

func (i installCmd) install(nameVersion string) {
	// Init celer.
	i.celer = configs.NewCeler()
	if err := i.celer.Init(); err != nil {
		configs.PrintError(err, "failed to init celer.")
		return
	}

	// Use build_type from `celer.toml` if not specified.
	if i.buildType == "" {
		i.buildType = i.celer.Global.BuildType
	}

	if err := i.celer.Platform().Setup(); err != nil {
		configs.PrintError(err, "setup platform error: %s", err)
		return
	}

	// In Windows PowerShell, when handling completion,
	// "`" is automatically added as an escape character before the "@".
	// We need to remove this escape character.
	nameVersion = strings.ReplaceAll(nameVersion, "`", "")

	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		configs.PrintError(fmt.Errorf("invalid name and version"), "install %s failed.", nameVersion)
		return
	}

	portInProject := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().Name, parts[0], parts[1], "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		configs.PrintError(fmt.Errorf("port %s is not found", nameVersion), "%s install failed.", nameVersion)
		return
	}

	// Install the port.
	var port configs.Port
	port.DevDep = i.dev
	if err := port.Init(i.celer, nameVersion, i.buildType); err != nil {
		configs.PrintError(err, "init %s failed.", nameVersion)
		return
	}

	if i.force {
		// Remove pacakge(installed + package + buildcache).
		if err := port.Remove(i.recurse, true, true); err != nil {
			configs.PrintError(err, "uninstall %s failed before reinstall.", nameVersion)
			return
		}

		// Remove all caches for the port.
		cacheDir := i.celer.CacheDir()
		if cacheDir != nil {
			if err := cacheDir.Remove(i.celer.Platform().Name, i.celer.Project().Name, i.buildType, port.NameVersion()); err != nil {
				configs.PrintError(err, "remove cache for %s failed before reinstall.", nameVersion)
				return
			}
		}
	}

	// Check circular dependence.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(i.celer, port); err != nil {
		configs.PrintError(err, "check circular dependence failed.")
		return
	}

	// Check version conflict.
	if err := depcheck.CheckConflict(i.celer, port); err != nil {
		configs.PrintError(err, "check version conflict failed.")
		return
	}

	// Do install.
	fromWhere, err := port.Install()
	if err != nil {
		configs.PrintError(err, "install %s failed.", nameVersion)
		return
	}
	if fromWhere != "" {
		configs.PrintSuccess("install %s from %s successfully.", nameVersion, fromWhere)
	} else {
		configs.PrintSuccess("install %s successfully.", nameVersion)
	}
}

func (i installCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	projectDir := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().Name)
	if fileio.PathExists(dirs.PortsDir) || fileio.PathExists(projectDir) {
		filepath.WalkDir(dirs.PortsDir, func(path string, entity fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !entity.IsDir() && entity.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})

		filepath.WalkDir(projectDir, func(path string, entity fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !entity.IsDir() && entity.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})

		// Support flags completion.
		for _, flag := range []string{"--dev", "-d", "--build-type", "-b", "--force", "-f"} {
			if strings.HasPrefix(flag, toComplete) {
				suggestions = append(suggestions, flag)
			}
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
