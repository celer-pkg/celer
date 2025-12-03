package cmds

import (
	"celer/configs"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type initCmd struct {
	celer  *configs.Celer
	url    string
	branch string
}

func (i initCmd) Command(celer *configs.Celer) *cobra.Command {
	i.celer = celer
	command := &cobra.Command{
		Use:   "init",
		Short: "Initialize celer with optional configuration repository.",
		Long: `Initialize celer by creating a celer.toml configuration file.
Optionally, you can specify a Git repository URL to clone configuration files
from a remote repository.

Examples:
  celer init                                             # Initialize without conf repo
  celer init --url https://github.com/example/conf       # Initialize with conf repo
  celer init -u https://github.com/example/conf -b main  # With specific branch`,
		Run: func(cmd *cobra.Command, args []string) {
			i.runInit()
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().StringVarP(&i.url, "url", "u", "", "URL of the configuration repository")
	command.Flags().StringVarP(&i.branch, "branch", "b", "", "Branch of the configuration repository (default: repository's default branch)")

	return command
}

func (i *initCmd) runInit() {
	// Initialize celer configuration
	if err := i.celer.Init(); err != nil {
		configs.PrintError(err, "Failed to initialize celer.")
		os.Exit(1)
	}

	// Trim whitespace from URL
	i.url = strings.TrimSpace(i.url)

	// Setup configuration repository if URL is provided
	if i.url != "" {
		if err := i.validateURL(i.url); err != nil {
			configs.PrintError(err, "Invalid URL.")
			os.Exit(1)
		}

		if err := i.celer.SetConfRepo(i.url, i.branch); err != nil {
			configs.PrintError(err, "Failed to setup configuration repository.")
			os.Exit(1)
		}

		configs.PrintSuccess("Successfully initialized celer with configuration repository: %s", i.url)
		if i.branch != "" {
			fmt.Printf("Using branch: %s\n", i.branch)
		}
	} else {
		configs.PrintSuccess("Initialize successfully.")
	}
}

// validateURL performs basic validation on the provided URL
func (i *initCmd) validateURL(url string) error {
	if url == "" {
		return fmt.Errorf("URL cannot be empty")
	}

	// Basic URL validation - check for common protocols
	if !strings.HasPrefix(url, "http://") &&
		!strings.HasPrefix(url, "https://") &&
		!strings.HasPrefix(url, "git://") &&
		!strings.HasPrefix(url, "ssh://") &&
		!strings.Contains(url, "@") { // SSH format like git@github.com
		return fmt.Errorf("URL must use http://, https://, git://, ssh:// protocol or SSH format")
	}

	return nil
}

func (i initCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Provide flag completion
	for _, flag := range []string{"--url", "-u", "--branch", "-b"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	// Provide URL suggestions when completing URL values
	if strings.HasPrefix(toComplete, "https://") {
		suggestions = append(suggestions,
			"https://github.com/",
			"https://gitlab.com/",
			"https://bitbucket.org/",
		)
		return suggestions, cobra.ShellCompDirectiveNoSpace
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
