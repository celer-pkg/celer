package cmds

import (
	"celer/configs"
	"strings"

	"github.com/spf13/cobra"
)

type initCmd struct {
	celer *configs.Celer
}

func (i initCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "init",
		Short: "Init with conf repo.",
		Run: func(cmd *cobra.Command, args []string) {
			url, _ := cmd.Flags().GetString("url")

			celer := configs.NewCeler()
			if err := celer.CloneConf(url); err != nil {
				if url == "" {
					configs.PrintError(err, "failed to init celer.")
				} else {
					configs.PrintError(err, "failed to init celer with --url=%s.", url)
				}
				return
			}

			configs.PrintSuccess("init celer successfully.")
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			var suggestions []string

			for _, flag := range []string{"--url", "--branch"} {
				if strings.HasPrefix(flag, toComplete) {
					suggestions = append(suggestions, flag)
				}
			}
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		},
	}

	// Register flags.
	command.Flags().String("url", "", "init with conf repository url.")
	command.Flags().String("branch", "master", "init with conf repository branch.")

	return command
}
