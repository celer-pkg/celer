package cmds

import (
	"celer/configs"
	"fmt"
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
		Long: `Create a new platform, project, or port configuration.

This command helps you scaffold new configurations for different components
in the celer package manager. You must specify exactly one type of component
to create using the mutually exclusive flags.

COMPONENT TYPES:
  --platform    Create a new platform configuration (e.g., windows-amd64, linux-x64)
  --project     Create a new project configuration
  --port        Create a new port with name@version format

EXAMPLES:
  celer create --platform windows-amd64-msvc
  celer create --project my-awesome-project
  celer create --port opencv@4.8.0`,
		Run: func(cmd *cobra.Command, args []string) {
			c.doCreate(cmd)
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
		configs.PrintError(err, "%s could not be created.", projectName)
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

func (c *createCmd) doCreate(cmd *cobra.Command) {
	if err := c.celer.Init(); err != nil {
		configs.PrintError(err, "Failed to initialize celer.")
		os.Exit(1)
	}

	// Check that exactly one flag is provided
	flags := cmd.Flags()
	provided := 0
	if flags.Changed("platform") {
		provided++
	}
	if flags.Changed("project") {
		provided++
	}
	if flags.Changed("port") {
		provided++
	}

	if provided == 0 {
		configs.PrintError(nil, "You must specify exactly one component to create (--platform, --project, or --port).")
		os.Exit(1)
	}

	// Validate inputs and create
	if c.platform != "" {
		if err := c.validatePlatformName(c.platform); err != nil {
			configs.PrintError(err, "Invalid platform name.")
			os.Exit(1)
		}
		c.createPlatform(c.platform)
	} else if c.project != "" {
		if err := c.validateProjectName(c.project); err != nil {
			configs.PrintError(err, "Invalid project name.")
			os.Exit(1)
		}
		c.createProject(c.project)
	} else if c.port != "" {
		if err := c.validatePortName(c.port); err != nil {
			configs.PrintError(err, "Invalid port name.")
			os.Exit(1)
		}
		c.createPort(c.port)
	}
}

// validatePlatformName validates platform name format
func (c *createCmd) validatePlatformName(name string) error {
	if strings.TrimSpace(name) == "" {
		return fmt.Errorf("platform name cannot be empty")
	}
	if strings.Contains(name, " ") {
		return fmt.Errorf("platform name cannot contain spaces")
	}
	return nil
}

// validateProjectName validates project name format
func (c *createCmd) validateProjectName(name string) error {
	if strings.TrimSpace(name) == "" {
		return fmt.Errorf("project name cannot be empty")
	}
	return nil
}

// validatePortName validates port name@version format
func (c *createCmd) validatePortName(nameVersion string) error {
	if strings.TrimSpace(nameVersion) == "" {
		return fmt.Errorf("port name cannot be empty")
	}
	if !strings.Contains(nameVersion, "@") {
		return fmt.Errorf("port must be in name@version format (e.g., opencv@4.8.0)")
	}
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 || parts[0] == "" || parts[1] == "" {
		return fmt.Errorf("invalid port format, expected name@version")
	}
	return nil
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
