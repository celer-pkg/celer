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
	powershell bool
	bash       bool
	zsh        bool
	unregister bool

	bashCompletion       completion.BashCompletion
	zshCompletion        completion.ZshCompletion
	powershellCompletion completion.PowerShellCompletion
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

			i.bashCompletion = completion.NewBashCompletion()
			i.zshCompletion = completion.NewZshCompletion()
			i.powershellCompletion = completion.NewPowerShellCompletion()

			if i.unregister {
				if err := i.doUnregister(homeDir); err != nil {
					configs.PrintError(err, "tab completion unregister failed.")
					os.Exit(1)
				}
				configs.PrintSuccess("tab completion is unregistered.")
			} else {
				if err := i.doRegister(homeDir); err != nil {
					configs.PrintError(err, "tab completion install failed.")
					os.Exit(1)
				}
				configs.PrintSuccess("tab completion is integrated.")
			}
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().BoolVar(&i.unregister, "unregister", false, "unregister tab completion.")
	command.Flags().BoolVar(&i.powershell, "powershell", false, "integrate tab completion for powershell.")
	command.Flags().BoolVar(&i.bash, "bash", false, "integrate tab completion for bash.")
	command.Flags().BoolVar(&i.zsh, "zsh", false, "integrate tab completion for zsh.")

	command.MarkFlagsMutuallyExclusive("powershell", "bash", "zsh")

	return command
}

func (i integrateCmd) doUnregister(homeDir string) error {
	switch {
	case i.powershell:
		return i.powershellCompletion.Unregister(homeDir)

	case i.bash:
		return i.bashCompletion.Unregister(homeDir)

	case i.zsh:
		return i.zshCompletion.Unregister(homeDir)

	default:
		return fmt.Errorf("no --bash, --zsh or --powershell specified to integrate")
	}
}

func (i integrateCmd) doRegister(homeDir string) error {
	switch {
	case i.bash:
		return i.bashCompletion.Register(homeDir)

	case i.zsh:
		return i.zshCompletion.Register(homeDir)

	case i.powershell:
		return i.powershellCompletion.Register(homeDir)

	default:
		return fmt.Errorf("no --bash, --zsh or --powershell specified to integrate")
	}

	return nil
}

func (i integrateCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--powershell", "--bash", "--zsh", "--unregister"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
