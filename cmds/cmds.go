package cmds

import (
	"celer/configs"
	"strings"

	"github.com/spf13/cobra"
)

const (
	test_conf_repo_url    = "https://github.com/celer-pkg/test-conf.git"
	test_conf_repo_branch = ""
)

// Interface for command.
type Command interface {
	Command(celer *configs.Celer) *cobra.Command
}

var rootCmd = &cobra.Command{
	Use:   "celer",
	Short: "A super lightweight package manager for C/C++.",
	Run: func(cmd *cobra.Command, args []string) {
		cmd.Println("welcome to celer.")
	},
	ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		var commands = []string{"version", "init", "create", "configure", "install", "remove", "integrate", "update"}
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
func Execute() error {
	commands := []Command{
		&versionCmd{},
		&initCmd{},
		&updateCmd{},
		&createCmd{},
		&configureCmd{},
		&installCmd{},
		&removeCmd{},
		&integrateCmd{},
		&deployCmd{},
		&treeCmd{},
		&cleanCmd{},
		&autoremoveCmd{},
		&reverseCmd{},
		&searchCmd{},
	}

	// Create celer but init it in command.
	celer := configs.NewCeler()

	// Register commands.
	for _, cmd := range commands {
		rootCmd.AddCommand(cmd.Command(celer))
	}

	return rootCmd.Execute()
}
