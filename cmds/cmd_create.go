package cmds

import (
	"celer/configs"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type createCmd struct {
	celer *configs.Celer
}

func (c createCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "create",
		Short: "Create new [platform|project|port].",
		Run: func(cmd *cobra.Command, args []string) {
			platform, _ := cmd.Flags().GetString("platform")
			project, _ := cmd.Flags().GetString("project")
			port, _ := cmd.Flags().GetString("port")

			if platform != "" {
				c.createPlatform(platform)
			} else if project != "" {
				c.createProject(project)
			} else if port != "" {
				c.createPort(port)
			}
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			// Support flags completion.
			var suggestions []string
			for _, flag := range []string{"--platform", "--project", "--port"} {
				if strings.HasPrefix(flag, toComplete) {
					suggestions = append(suggestions, flag)
				}
			}
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		},
	}

	// Register flags.
	command.Flags().String("platform", "", "create new platform.")
	command.Flags().String("project", "", "create new project.")
	command.Flags().String("port", "", "create new port.")

	return command
}

func (c createCmd) createPlatform(platformName string) {
	celer := configs.NewCeler()
	if err := celer.CreatePlatform(platformName); err != nil {
		configs.PrintError(err, "%s could not be created.", platformName)
		os.Exit(1)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", platformName)
}

func (c createCmd) createProject(projectName string) {
	celer := configs.NewCeler()
	if err := celer.CreateProject(projectName); err != nil {
		configs.PrintSuccess("%s could not be created.", projectName)
		os.Exit(1)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", projectName)
}

func (c createCmd) createPort(nameVersion string) {
	celer := configs.NewCeler()
	if err := celer.CreatePort(nameVersion); err != nil {
		configs.PrintError(err, "%s could not be created.", nameVersion)
		os.Exit(1)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", nameVersion)
}
