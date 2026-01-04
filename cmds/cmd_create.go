package cmds

import (
	"celer/configs"
	"fmt"
	"strings"

	"github.com/spf13/cobra"
)

type createCmd struct {
	celer    *configs.Celer
	platform string
	project  string
	port     string
}

func (c *createCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "create",
		Short: "Create a platform, project or port.",
		Long: `Create a platform, project or port.

This command helps you scaffold new configurations for different components
in the celer package manager. You must specify exactly one type of component
to create using the mutually exclusive flags.

COMPONENT TYPES:
  --platform    Create a new platform configuration (e.g., windows-x86_64, linux-x64)
  --project     Create a new project configuration
  --port        Create a new port with name@version format

EXAMPLES:
  celer create --platform windows-x86_64-msvc
  celer create --project my-awesome-project
  celer create --port opencv@4.8.0`,
		RunE: func(cmd *cobra.Command, args []string) error {
			return c.doCreate(cmd)
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().StringVar(&c.platform, "platform", "", "create a new platform.")
	command.Flags().StringVar(&c.project, "project", "", "create a new project.")
	command.Flags().StringVar(&c.port, "port", "", "create a new port.")

	command.MarkFlagsMutuallyExclusive("platform", "project", "port")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (c *createCmd) createPlatform(platformName string) error {
	if err := c.celer.CreatePlatform(platformName); err != nil {
		return configs.PrintError(err, "%s could not be created.", platformName)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", platformName)
	return nil
}

func (c *createCmd) createProject(projectName string) error {
	if err := c.celer.CreateProject(projectName); err != nil {
		return configs.PrintError(err, "%s could not be created.", projectName)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", projectName)
	return nil
}

func (c *createCmd) createPort(nameVersion string) error {
	if err := c.celer.CreatePort(nameVersion); err != nil {
		return configs.PrintError(err, "%s could not be created.", nameVersion)
	}

	configs.PrintSuccess("%s is created, please proceed with its refinement.", nameVersion)
	return nil
}

func (c *createCmd) doCreate(cmd *cobra.Command) error {
	if err := c.celer.Init(); err != nil {
		return configs.PrintError(err, "Failed to initialize celer.")
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
		err := fmt.Errorf("you must specify exactly one component to create (--platform, --project, or --port)")
		return configs.PrintError(err, "You must specify exactly one component to create (--platform, --project, or --port).")
	}

	// Validate inputs and create
	if c.platform != "" {
		if err := c.validatePlatformName(c.platform); err != nil {
			return configs.PrintError(err, "Invalid platform name.")
		}
		return c.createPlatform(c.platform)
	} else if c.project != "" {
		if err := c.validateProjectName(c.project); err != nil {
			return configs.PrintError(err, "Invalid project name.")
		}
		return c.createProject(c.project)
	} else if c.port != "" {
		if err := c.validatePortName(c.port); err != nil {
			return configs.PrintError(err, "Invalid port name.")
		}
		return c.createPort(c.port)
	}

	return nil
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

func (c *createCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--platform", "--project", "--port"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}
	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
