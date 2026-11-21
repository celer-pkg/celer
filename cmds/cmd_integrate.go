package cmds

import (
	"fmt"
	"os"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/completion"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/internal/logger"

	"github.com/spf13/cobra"
)

type IntegrateCmd struct {
	remove bool

	bashCompletion completion.Completion
	zshCompletion  completion.Completion
	psCompletion   completion.Completion
}

var CurrentShellFn = completion.CurrentShell

func (i *IntegrateCmd) Command(celer *configs.Celer) *cobra.Command {
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
			return i.Execute()
		},
		ValidArgsFunction: i.Completion,
	}

	// Register flags.
	command.Flags().BoolVar(&i.remove, "remove", false, "remove shell tab completion")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

// execute performs the main logic for integration.
func (i *IntegrateCmd) Execute() error {
	shell := i.DetectShell()

	// Validate environment.
	if err := i.ValidateEnvironment(shell); err != nil {
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

func (i *IntegrateCmd) DetectShell() (shell completion.ShellType) {
	shell = completion.NotSupported
	defer func() {
		if recover() != nil {
			shell = completion.NotSupported
		}
	}()
	return CurrentShellFn()
}

func (i *IntegrateCmd) ValidateEnvironment(shell completion.ShellType) error {
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
func (i *IntegrateCmd) initializeCompletions() error {
	homeDir, err := os.UserHomeDir()
	if err != nil {
		return fmt.Errorf("failed to get user home directory -> %w", err)
	}

	i.bashCompletion = completion.NewBashCompletion(homeDir, rootCmd)
	i.zshCompletion = completion.NewZshCompletion(homeDir, rootCmd)
	i.psCompletion = completion.NewPowerShellCompletion(homeDir, rootCmd)

	return nil
}

func (i *IntegrateCmd) handleRegister(shell completion.ShellType) error {
	if err := i.doRegister(shell); err != nil {
		return fmt.Errorf("failed to register %s completion -> %w", i.getShellName(shell), err)
	}

	logger.PrintSuccess("%s tab completion has been integrated successfully", i.getShellName(shell))
	return nil
}

func (i *IntegrateCmd) handleUnregister(shell completion.ShellType) error {
	if err := i.doUnregister(shell); err != nil {
		return fmt.Errorf("failed to unregister %s completion -> %w", i.getShellName(shell), err)
	}

	logger.PrintSuccess("%s tab completion has been removed successfully", i.getShellName(shell))
	return nil
}

func (i *IntegrateCmd) getShellName(shell completion.ShellType) string {
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

func (i *IntegrateCmd) doUnregister(shell completion.ShellType) error {
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

func (i *IntegrateCmd) doRegister(shell completion.ShellType) error {
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

func (i *IntegrateCmd) Completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--remove"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
