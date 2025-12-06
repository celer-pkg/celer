package cmds

import (
	"celer/configs"
	"fmt"
	"strings"

	"github.com/spf13/cobra"
)

type initCmd struct {
	celer  *configs.Celer
	url    string
	branch string
	force  bool
}

func (i *initCmd) Command(celer *configs.Celer) *cobra.Command {
	i.celer = celer
	command := &cobra.Command{
		Use:   "init",
		Short: "Init celer with configuration repository.",
		Long: `Initialize celer with configuration repository.

This command initializes celer in the current directory by creating a
celer.toml configuration file and cloning configuration files from a
remote Git repository.

Examples:
  celer init --url https://github.com/example/conf       	# Initialize with conf repo
  celer init -u https://github.com/example/conf -b main  	# With specific branch
  celer init --url https://github.com/example/conf --force	# Force re-initialize`,
		Run: func(cmd *cobra.Command, args []string) {
			i.doInit()
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().StringVarP(&i.url, "url", "u", "", "URL of the configuration repository")
	command.Flags().StringVarP(&i.branch, "branch", "b", "", "Branch of the configuration repository (default: repository's default branch)")
	command.Flags().BoolVarP(&i.force, "force", "f", false, "Force re-initialize even if configuration exists")

	// Mark url as required.
	command.MarkFlagRequired("url")

	return command
}

func (i *initCmd) doInit() {
	// Initialize celer configuration
	if err := i.celer.Init(); err != nil {
		configs.PrintError(err, "Failed to initialize celer.")
		return
	}

	// Trim whitespace from URL
	i.url = strings.TrimSpace(i.url)

	// Setup configuration repository.
	if err := i.validateURL(i.url); err != nil {
		configs.PrintError(err, "Invalid URL.")
		return
	}

	if err := i.celer.CloneConf(i.url, i.branch, i.force); err != nil {
		configs.PrintError(err, "Failed to setup configuration repository.")
		return
	}

	if i.branch != "" {
		configs.PrintSuccess("Successfully initialized celer with configuration repository: %s --branch %s", i.url, i.branch)
	} else {
		configs.PrintSuccess("Successfully initialized celer with configuration repository: %s", i.url)
	}
}

// validateURL performs basic validation on the provided URL.
func (i *initCmd) validateURL(url string) error {
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

func (i *initCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Provide flag completion
	for _, flag := range []string{"--url", "-u", "--branch", "-b", "--force", "-f"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
