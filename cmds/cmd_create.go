package cmds

import (
	"celer/configs"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type createCmd struct {
	celer    *configs.Celer
	platform string
	project  string
	port     string
}

func (c createCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "create",
		Short: "Create a platform, project, or port.",
		Run: func(cmd *cobra.Command, args []string) {
			if c.celer.HandleInitError() {
				os.Exit(1)
			}

			if c.platform != "" {
				c.createPlatform(c.platform)
			} else if c.project != "" {
				c.createProject(c.project)
			} else if c.port != "" {
				c.createPort(c.port)
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().StringVar(&c.platform, "platform", "", "create a new platform.")
	command.Flags().StringVar(&c.project, "project", "", "create a new project.")
	command.Flags().StringVar(&c.port, "port", "", "create a new port.")

	command.MarkFlagsMutuallyExclusive("platform", "project", "port")
	return command
}

func (c createCmd) createPlatform(platformName string) {
	if err := c.celer.CreatePlatform(platformName); err != nil {
		configs.PrintError(err, "%s could not be created.", platformName)
		os.Exit(1)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", platformName)
}

func (c createCmd) createProject(projectName string) {
	if err := c.celer.CreateProject(projectName); err != nil {
		configs.PrintSuccess("%s could not be created.", projectName)
		os.Exit(1)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", projectName)
}

func (c createCmd) createPort(nameVersion string) {
	if err := c.celer.CreatePort(nameVersion); err != nil {
		configs.PrintError(err, "%s could not be created.", nameVersion)
		os.Exit(1)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", nameVersion)
}

func (c createCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--platform", "--project", "--port"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
