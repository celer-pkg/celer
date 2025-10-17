package cmds

import (
	"celer/cmds/completion"
	"celer/configs"
	"fmt"
	"os"
	"runtime"
	"strings"

	"github.com/spf13/cobra"
)

type integrateCmd struct {
	powershell bool
	bash       bool
	zsh        bool
	unregister bool

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

			if i.unregister {
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
	command.Flags().BoolVar(&i.powershell, "powershell", false, "integrate tab completion for powershell.")
	command.Flags().BoolVar(&i.bash, "bash", false, "integrate tab completion for bash.")
	command.Flags().BoolVar(&i.zsh, "zsh", false, "integrate tab completion for zsh.")
	command.Flags().BoolVar(&i.unregister, "unregister", false, "unregister tab completion.")

	command.MarkFlagsMutuallyExclusive("powershell", "bash", "zsh")

	return command
}

func (i integrateCmd) doUnregister() error {
	switch {
	case i.powershell:
		return i.psCompletion.Unregister()

	case i.bash:
		return i.bashCompletion.Unregister()

	case i.zsh:
		return i.zshCompletion.Unregister()

	default:
		return fmt.Errorf("no --bash, --zsh or --powershell specified to unregister")
	}
}

func (i integrateCmd) doRegister() error {
	switch {
	case i.bash:
		if runtime.GOOS != "linux" {
			return fmt.Errorf("bash completion is only supported on linux")
		}
		return i.bashCompletion.Register()

	case i.zsh:
		if runtime.GOOS != "linux" && runtime.GOOS != "darwin" {
			return fmt.Errorf("zsh completion is only supported on linux and macOS")
		}

		return i.zshCompletion.Register()

	case i.powershell:
		if runtime.GOOS != "windows" {
			return fmt.Errorf("powershell completion is only supported on windows")
		}
		return i.psCompletion.Register()

	default:
		return fmt.Errorf("no --bash, --zsh or --powershell specified to integrate")
	}
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
