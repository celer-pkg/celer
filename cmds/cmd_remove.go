package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"errors"
	"os"
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
		Short: "Uninstall a package.",
		Args:  cobra.MinimumNArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			if len(args) == 0 {
				configs.PrintError(errors.New("no package specified"), "failed to remove package.")
				return
			}

			r.buildType, _ = cmd.Flags().GetString("build-type")
			r.recurse, _ = cmd.Flags().GetBool("recurse")
			r.purge, _ = cmd.Flags().GetBool("purge")
			r.dev, _ = cmd.Flags().GetBool("dev")
			r.removeCache, _ = cmd.Flags().GetBool("remove-cache")

			// Use build_type from `celer.toml` if not specified.
			if r.buildType == "" {
				r.buildType = r.celer.Settings.BuildType
			}

			if err := r.remove(args); err != nil {
				configs.PrintError(err, "failed to remove %s.", strings.Join(args, ", "))
				return
			}

			configs.PrintSuccess("remove %s successfully.", strings.Join(args, ", "))
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			return r.completion(toComplete)
		},
	}

	// Register flags.
	command.Flags().String("build-type", "", "uninstall package with specified build type.")
	command.Flags().Bool("remove-cache", false, "uninstall package along with build cache.")
	command.Flags().Bool("recurse", false, "uninstall package along with its depedencies.")
	command.Flags().Bool("purge", false, "uninstall package along with its package file.")
	command.Flags().Bool("dev", false, "uninstall package from dev mode.")

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

func (r removeCmd) completion(toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	var buildtreesDir = dirs.BuildtreesDir

	if fileio.PathExists(buildtreesDir) {
		entities, err := os.ReadDir(buildtreesDir)
		if err != nil {
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		}

		for _, entity := range entities {
			if entity.IsDir() && strings.HasPrefix(entity.Name(), toComplete) {
				suggestions = append(suggestions, entity.Name())
			}
		}

		// Support flags completion.
		for _, flag := range []string{"--build-type", "--remove-cache", "--recurse", "--purge", "--dev"} {
			if strings.HasPrefix(flag, toComplete) {
				suggestions = append(suggestions, flag)
			}
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
