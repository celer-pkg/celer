package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type updateCmd struct{}

func (u updateCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "update",
		Short: "Update port's repo.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			celer := configs.NewCeler()
			if err := celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			nameVersion := args[0]
			force, _ := cmd.Flags().GetBool("force")

			if err := celer.UpdatePortRepo(nameVersion, force); err != nil {
				color.Printf(color.Red, "%s\n", err.Error())
			}
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			return u.completion(toComplete)
		},
	}

	// Register flags.
	command.Flags().BoolP("force", "f", false, "update port's repo forcibly")

	return command
}

func (u updateCmd) completion(toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	var buildtreesDir = dirs.BuildtreesDir

	// Support port completion.
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
	}

	// Support project completion.
	if fileio.PathExists(dirs.ConfProjectsDir) {
		entities, err := os.ReadDir(dirs.ConfProjectsDir)
		if err != nil {
			configs.PrintError(err, "failed to read %s: %s.\n", dirs.ConfProjectsDir, err)
			os.Exit(1)
		}

		for _, entity := range entities {
			if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".toml") {
				fileName := strings.TrimSuffix(entity.Name(), ".toml")
				if strings.HasPrefix(fileName, toComplete) {
					suggestions = append(suggestions, fileName)
				}
			}
		}
	}

	// Support flags completion.
	for _, flag := range []string{"--force", "-f"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
