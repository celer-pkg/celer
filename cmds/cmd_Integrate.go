package cmds

import (
	"celer/completion"
	"celer/configs"
	"fmt"
	"os"
	"runtime"
	"strings"

	"github.com/spf13/cobra"
)

type integrateCmd struct {
	remove bool

	bashCompletion completion.Completion
	zshCompletion  completion.Completion
	psCompletion   completion.Completion
}

var currentShellFn = completion.CurrentShell

func (i *integrateCmd) Command(celer *configs.Celer) *cobra.Command {
	command := &cobra.Command{
		Use:   "integrate",
		Short: "Integrate shell tab completion.",
		Long: `Integrate shell tab completion for celer commands.

This command will install or remove shell completion scripts.
On Linux, celer integrates completion for your current shell (bash or zsh).
On Windows, celer integrates PowerShell completion.

Examples:
  celer integrate          # Install tab completion
  celer integrate --remove # Remove tab completion`,
		Args: cobra.NoArgs,
		RunE: func(cobraCmd *cobra.Command, args []string) error {
			if err := i.execute(); err != nil {
				return configs.PrintError(err, "integration failed")
			}
			return nil
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().BoolVar(&i.remove, "remove", false, "remove shell tab completion")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

// execute performs the main logic for integration.
func (i *integrateCmd) execute() error {
	shell := i.detectShell()

	// Validate environment.
	if err := i.validateEnvironment(shell); err != nil {
		return fmt.Errorf("environment validation failed -> %w", err)
	}

	// Initialize completions.
	if err := i.initializeCompletions(); err != nil {
		return fmt.Errorf("failed to initialize completion handlers -> %w", err)
	}

	// Execute the requested operation.
	if i.remove {
		return i.handleUnregister(shell)
	}
	return i.handleRegister(shell)
}

func (i *integrateCmd) detectShell() (shell completion.ShellType) {
	shell = completion.NotSupported
	defer func() {
		if recover() != nil {
			shell = completion.NotSupported
		}
	}()
	return currentShellFn()
}

func (i *integrateCmd) validateEnvironment(shell completion.ShellType) error {
	if shell == completion.NotSupported {
		if runtime.GOOS == "windows" {
			return fmt.Errorf("unsupported shell environment, on Windows only powershell is supported")
		} else {
			return fmt.Errorf("unsupported shell environment, supported shells: bash, zsh")
		}
	}

	return nil
}

// initializeCompletions sets up completion handlers
func (i *integrateCmd) initializeCompletions() error {
	homeDir, err := os.UserHomeDir()
	if err != nil {
		return fmt.Errorf("failed to get user home directory -> %w", err)
	}

	i.bashCompletion = completion.NewBashCompletion(homeDir, rootCmd)
	i.zshCompletion = completion.NewZshCompletion(homeDir, rootCmd)
	i.psCompletion = completion.NewPowerShellCompletion(homeDir, rootCmd)

	return nil
}

func (i *integrateCmd) handleRegister(shell completion.ShellType) error {
	if err := i.doRegister(shell); err != nil {
		return fmt.Errorf("failed to register %s completion -> %w", i.getShellName(shell), err)
	}

	configs.PrintSuccess("%s tab completion has been integrated successfully", i.getShellName(shell))
	return nil
}

func (i *integrateCmd) handleUnregister(shell completion.ShellType) error {
	if err := i.doUnregister(shell); err != nil {
		return fmt.Errorf("failed to unregister %s completion -> %w", i.getShellName(shell), err)
	}

	configs.PrintSuccess("%s tab completion has been removed successfully", i.getShellName(shell))
	return nil
}

func (i *integrateCmd) getShellName(shell completion.ShellType) string {
	switch shell {
	case completion.BashShell:
		return "bash"
	case completion.ZshShell:
		return "zsh"
	case completion.TypePowerShell:
		return "PowerShell"
	default:
		return "unknown"
	}
}

func (i *integrateCmd) doUnregister(shell completion.ShellType) error {
	switch shell {
	case completion.BashShell:
		return i.bashCompletion.Unregister()
	case completion.ZshShell:
		return i.zshCompletion.Unregister()
	case completion.TypePowerShell:
		return i.psCompletion.Unregister()
	default:
		return fmt.Errorf("unsupported shell: %s", i.getShellName(shell))
	}
}

func (i *integrateCmd) doRegister(shell completion.ShellType) error {
	switch shell {
	case completion.BashShell:
		return i.bashCompletion.Register()
	case completion.ZshShell:
		return i.zshCompletion.Register()
	case completion.TypePowerShell:
		return i.psCompletion.Register()
	default:
		return fmt.Errorf("unsupported shell: %s", i.getShellName(shell))
	}
}

func (i *integrateCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--remove"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
