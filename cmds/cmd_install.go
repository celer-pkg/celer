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
			nameVersion := args[0]
			i.dev, _ = cmd.Flags().GetBool("dev")
			i.buildType, _ = cmd.Flags().GetString("build-type")
			i.force, _ = cmd.Flags().GetBool("force")
			i.recurse, _ = cmd.Flags().GetBool("recurse")
			i.install(nameVersion)
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			return i.completion(toComplete)
		},
	}

	// Register flags.
	command.Flags().Bool("dev", false, "install package as runtime dev mode.")
	command.Flags().String("build-type", "", "install package with build type.")
	command.Flags().BoolP("force", "f", false, "uninstall package before install again.")
	command.Flags().BoolP("recurse", "r", false, "uninstall package recursively before install again.")

	return command
}

func (i installCmd) install(nameVersion string) {
	// Init celer.
	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		configs.PrintError(err, "failed to init celer.")
		return
	}

	// Use build_type from `celer.toml` if not specified.
	if i.buildType == "" {
		i.buildType = celer.Settings.BuildType
	}

	if err := celer.Platform().Setup(); err != nil {
		configs.PrintError(err, "setup platform failed: %s", err)
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

	portInProject := filepath.Join(dirs.ConfProjectsDir, celer.Project().Name, parts[0], parts[1], "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		configs.PrintError(fmt.Errorf("port %s is not found", nameVersion), "%s install failed.", nameVersion)
		return
	}

	// Install the port.
	var port configs.Port
	port.DevDep = i.dev
	if err := port.Init(celer, nameVersion, i.buildType); err != nil {
		configs.PrintError(err, "init %s failed.", nameVersion)
		return
	}

	// Uninstall the port if force flag is `ON`.
	if i.force {
		port.Remove(i.recurse, true, true)
	}

	// Check circular dependence.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(celer, port); err != nil {
		configs.PrintError(err, "check circular dependence failed.")
		return
	}

	// Check version conflict.
	if err := depcheck.CheckConflict(celer, port); err != nil {
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

func (i installCmd) completion(toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	var portsDir = dirs.PortsDir

	if fileio.PathExists(portsDir) {
		filepath.WalkDir(portsDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !d.IsDir() && strings.HasSuffix(d.Name(), ".toml") {
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
		for _, flag := range []string{"--dev", "--build-type", "--force", "-f"} {
			if strings.HasPrefix(flag, toComplete) {
				suggestions = append(suggestions, flag)
			}
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
