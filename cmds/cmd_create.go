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
		Args: cobra.NoArgs,
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
	// Check that exactly one flag is provided.
	flags := cmd.Flags()
	platformChanged := flags.Changed("platform")
	projectChanged := flags.Changed("project")
	portChanged := flags.Changed("port")

	provided := 0
	if platformChanged {
		provided++
	}
	if projectChanged {
		provided++
	}
	if portChanged {
		provided++
	}

	if provided != 1 {
		err := fmt.Errorf("invalid input argument")
		return configs.PrintError(err, "You must specify exactly one component to create (--platform, --project, or --port).")
	}

	// Validate inputs and create.
	if platformChanged {
		if err := c.validatePlatformName(c.platform); err != nil {
			return configs.PrintError(err, "Invalid platform name.")
		}
		return c.createPlatform(c.platform)
	}

	if projectChanged {
		if err := c.validateProjectName(c.project); err != nil {
			return configs.PrintError(err, "Invalid project name.")
		}
		return c.createProject(c.project)
	}

	if portChanged {
		if err := c.validatePortName(c.port); err != nil {
			return configs.PrintError(err, "Invalid port name.")
		}
		return c.createPort(c.port)
	}

	return nil
}

func validateInput(value, field string, allowSpace bool) error {
	trimmed := strings.TrimSpace(value)
	if trimmed == "" {
		return fmt.Errorf("%s cannot be empty", field)
	}

	if strings.Contains(trimmed, "..") {
		return fmt.Errorf("%s cannot contain '..'", field)
	}

	// Keep generated files inside workspace and avoid invalid file-name characters.
	if strings.ContainsAny(trimmed, "<>:\"/\\|?*") {
		return fmt.Errorf("%s contains invalid file-name characters", field)
	}

	if !allowSpace && strings.Contains(trimmed, " ") {
		return fmt.Errorf("%s cannot contain spaces", field)
	}

	return nil
}

// validatePlatformName validates platform name format
func (c *createCmd) validatePlatformName(name string) error {
	return validateInput(name, "platform name", false)
}

// validateProjectName validates project name format
func (c *createCmd) validateProjectName(name string) error {
	return validateInput(name, "project name", true)
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
	if len(parts) != 2 {
		return fmt.Errorf("invalid port format, expected name@version")
	}

	name := strings.TrimSpace(parts[0])
	version := strings.TrimSpace(parts[1])
	if name == "" || version == "" {
		return fmt.Errorf("invalid port format, expected name@version")
	}

	if err := validateInput(name, "port name", false); err != nil {
		return err
	}
	if err := validateInput(version, "port version", false); err != nil {
		return err
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
