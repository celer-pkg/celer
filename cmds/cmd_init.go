package cmds

import (
	"celer/configs"
	"strings"

	"github.com/spf13/cobra"
)

type initCmd struct {
	celer  *configs.Celer
	url    string
	branch string
}

func (i initCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "init",
		Short: "Init celer with conf repo.",
		Run: func(cmd *cobra.Command, args []string) {
			celer := configs.NewCeler()
			if err := celer.CloneConf(i.url, i.branch); err != nil {
				configs.PrintError(err, "failed to init celer: %s.", err)
				return
			}

			configs.PrintSuccess("init celer successfully.")
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().StringVarP(&i.url, "url", "u", "", "init with conf repo url.")
	command.Flags().StringVarP(&i.branch, "branch", "b", "", "init with conf repo branch.")

	return command
}

func (i initCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	for _, flag := range []string{"--url", "-u", "--branch", "-b"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
