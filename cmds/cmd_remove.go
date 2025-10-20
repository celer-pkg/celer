package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type removeCmd struct {
	celer      *configs.Celer
	dev        bool
	purge      bool
	recurse    bool
	buildCache bool
}

func (r removeCmd) Command(celer *configs.Celer) *cobra.Command {
	r.celer = celer
	command := &cobra.Command{
		Use:   "remove",
		Short: "Remove a package.",
		Args:  cobra.MinimumNArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			if err := r.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				os.Exit(1)
			}

			if err := r.remove(args); err != nil {
				configs.PrintError(err, "failed to remove %s.", strings.Join(args, ", "))
				os.Exit(1)
			}

			configs.PrintSuccess("remove %s successfully.", strings.Join(args, ", "))
		},
		ValidArgsFunction: r.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&r.buildCache, "build-cache", "c", false, "uninstall package along with build cache.")
	command.Flags().BoolVarP(&r.recurse, "recurse", "r", false, "uninstall package along with its depedencies.")
	command.Flags().BoolVarP(&r.purge, "purge", "p", false, "uninstall package along with its package files.")
	command.Flags().BoolVarP(&r.dev, "dev", "d", false, "uninstall package for dev mode.")

	return command
}

func (r removeCmd) remove(nameVersions []string) error {
	removeOptions := configs.RemoveOptions{
		Purge:      r.purge,
		Recurse:    r.recurse,
		BuildCache: r.buildCache,
	}

	for _, nameVersion := range nameVersions {
		var port configs.Port
		port.DevDep = r.dev

		if err := port.Init(r.celer, nameVersion); err != nil {
			return err
		}
		if err := port.Remove(removeOptions); err != nil {
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
			"--build-cache", "-c",
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
