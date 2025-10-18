package cmds

import (
	"celer/cmds/completion"
	"celer/configs"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type integrateCmd struct {
	remove bool

	bashCompletion completion.Completion
	zshCompletion  completion.Completion
	psCompletion   completion.Completion
}

func (i integrateCmd) Command(celer *configs.Celer) *cobra.Command {
	command := &cobra.Command{
		Use:   "integrate",
		Short: "Integrate tab completion.",
		Run: func(cobraCmd *cobra.Command, args []string) {
			homeDir, err := os.UserHomeDir()
			if err != nil {
				configs.PrintError(err, "failed to get home dir.")
				os.Exit(1)
			}

			i.bashCompletion = completion.NewBashCompletion(homeDir, rootCmd)
			i.zshCompletion = completion.NewZshCompletion(homeDir, rootCmd)
			i.psCompletion = completion.NewPowerShellCompletion(homeDir, rootCmd)

			if i.remove {
				if err := i.doUnregister(); err != nil {
					configs.PrintError(err, "tab completion unregister failed.")
					os.Exit(1)
				}
				configs.PrintSuccess("tab completion is unregistered.")
			} else {
				if err := i.doRegister(); err != nil {
					configs.PrintError(err, "tab completion install failed.")
					os.Exit(1)
				}
				configs.PrintSuccess("tab completion is integrated.")
			}
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().BoolVar(&i.remove, "remove", false, "remove tab completion.")

	return command
}

func (i integrateCmd) doUnregister() error {
	switch completion.CurrentShell() {
	case completion.BashShell:
		return i.bashCompletion.Unregister()

	case completion.ZshShell:
		return i.zshCompletion.Unregister()

	case completion.TypePowerShell:
		return i.psCompletion.Unregister()

	default:
		return fmt.Errorf("unsupported shell: %v", completion.CurrentShell())
	}
}

func (i integrateCmd) doRegister() error {
	switch completion.CurrentShell() {
	case completion.BashShell:
		return i.bashCompletion.Register()

	case completion.ZshShell:
		return i.zshCompletion.Register()

	case completion.TypePowerShell:
		return i.psCompletion.Register()

	default:
		return fmt.Errorf("unsupported shell: %v", completion.CurrentShell())
	}
}

func (i integrateCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--remove"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
