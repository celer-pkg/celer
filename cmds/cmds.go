package cmds

import (
	"celer/configs"
	"strings"

	"github.com/spf13/cobra"
)

// Interface for command.
type Command interface {
	Command() *cobra.Command
}

var rootCmd = &cobra.Command{
	Use:   "celer",
	Short: "A pkg-manager for C/C++, it's simply a supplement to CMake.",
	Run: func(cmd *cobra.Command, args []string) {
		cmd.Println("welcome to celer.")
	},
	ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		var commands = []string{"about", "init", "create", "configure", "install", "remove", "integrate", "update"}
		var suggestions = []string{}

		for _, c := range commands {
			if strings.HasPrefix(strings.ToLower(c), strings.ToLower(toComplete)) {
				suggestions = append(suggestions, c)
			}
		}

		// Return filtered commands.
		if len(suggestions) == 0 {
			return commands, cobra.ShellCompDirectiveNoFileComp
		}

		// Return all available commands.
		return suggestions, cobra.ShellCompDirectiveNoFileComp
	},
	// Hide competion command.
	CompletionOptions: cobra.CompletionOptions{
		DisableDefaultCmd: true,
	},
}

// Execute register all commands and executes the command.
func Execute(celer *configs.Celer) error {
	commands := []Command{
		aboutCmd{celer: celer},
		initCmd{celer: celer},
		updateCmd{celer: celer},
		createCmd{celer: celer},
		configureCmd{celer: celer},
		installCmd{celer: celer},
		removeCmd{celer: celer},
		integrateCmd{celer: celer},
		deployCmd{celer: celer},
		treeCmd{celer: celer},
		cleanCmd{celer: celer},
		autoremoveCmd{celer: celer},
	}

	// Register commands.
	for _, cmd := range commands {
		rootCmd.AddCommand(cmd.Command())
	}

	return rootCmd.Execute()
}
