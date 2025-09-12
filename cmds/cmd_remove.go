package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type removeCmd struct {
	celer       *configs.Celer
	buildType   string
	dev         bool
	purge       bool
	recurse     bool
	removeCache bool
}

func (r removeCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "remove",
		Short: "Remove a package.",
		Args:  cobra.MinimumNArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			r.celer = configs.NewCeler()
			if err := r.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			// Set offline mode.
			buildtools.Offline = r.celer.Global.Offline
			configs.Offline = r.celer.Global.Offline

			// Use build_type from `celer.toml` if not specified.
			if r.buildType == "" {
				r.buildType = r.celer.BuildType()
			}

			if err := r.remove(args); err != nil {
				configs.PrintError(err, "failed to remove %s.", strings.Join(args, ", "))
				return
			}

			configs.PrintSuccess("remove %s successfully.", strings.Join(args, ", "))
		},
		ValidArgsFunction: r.completion,
	}

	// Register flags.
	command.Flags().StringVarP(&r.buildType, "build-type", "b", "release", "uninstall package with specified build type.")
	command.Flags().BoolVarP(&r.removeCache, "remove-cache", "c", false, "uninstall package along with build cache.")
	command.Flags().BoolVarP(&r.recurse, "recurse", "r", false, "uninstall package along with its depedencies.")
	command.Flags().BoolVarP(&r.purge, "purge", "p", false, "uninstall package along with its package files.")
	command.Flags().BoolVarP(&r.dev, "dev", "d", false, "uninstall package for dev mode.")

	return command
}

func (r removeCmd) remove(nameVersions []string) error {
	for _, nameVersion := range nameVersions {
		var port configs.Port
		port.DevDep = r.dev

		if err := port.Init(r.celer, nameVersion, r.buildType); err != nil {
			return err
		}
		if err := port.Remove(r.recurse, r.purge, r.removeCache); err != nil {
			return err
		}
	}

	return nil
}

func (r removeCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	traceDir := filepath.Join(dirs.InstalledDir, "celer", "trace")
	if fileio.PathExists(traceDir) {
		entities, err := os.ReadDir(traceDir)
		if err != nil {
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		}

		for _, entity := range entities {
			parts := strings.Split(entity.Name(), "@")
			nameVersion := parts[0] + "@" + parts[1]
			if strings.HasPrefix(nameVersion, toComplete) {
				suggestions = append(suggestions, nameVersion)
			}
		}

		flags := []string{
			"--build-type", "-b",
			"--remove-cache", "-c",
			"--recurse", "-r",
			"--purge", "-p",
			"--dev", "-d",
		}

		for _, flag := range flags {
			if strings.HasPrefix(flag, toComplete) {
				suggestions = append(suggestions, flag)
			}
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
