package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type configureCmd struct {
	celer    *configs.Celer
	platform string
	project  string
}

func (c configureCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "configure",
		Short: "Configure to change platform or project.",
		Run: func(cmd *cobra.Command, args []string) {
			c.celer = configs.NewCeler()
			if err := c.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			if c.platform != "" {
				c.selectPlatform(c.platform)
			} else if c.project != "" {
				c.selectProject(c.project)
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().StringVar(&c.platform, "platform", "", "configure platform.")
	command.Flags().StringVar(&c.project, "project", "", "configure project.")

	// Support complete available platforms and projects.
	command.RegisterFlagCompletionFunc("platform", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.flagCompletion(dirs.ConfPlatformsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("project", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.flagCompletion(dirs.ConfProjectsDir, toComplete)
	})

	command.MarkFlagsMutuallyExclusive("platform", "project")
	return command
}

func (c configureCmd) selectPlatform(platformName string) {
	if err := c.celer.ChangePlatform(platformName); err != nil {
		configs.PrintError(err, "failed to select platform: %s.", platformName)
		os.Exit(1)
	}

	configs.PrintSuccess("current platform: %s.", platformName)
}

func (c configureCmd) selectProject(projectName string) {
	if err := c.celer.ChangeProject(projectName); err != nil {
		configs.PrintError(err, "failed to select project: %s.", projectName)
		os.Exit(1)
	}
	configs.PrintSuccess("celer is ready for project: %s.", projectName)
}

func (c configureCmd) flagCompletion(dir, toComplete string) ([]string, cobra.ShellCompDirective) {
	var fileNames []string
	if fileio.PathExists(dir) {
		entities, err := os.ReadDir(dir)
		if err != nil {
			color.Printf(color.Red, "failed to read %s: %s.\n", dir, err)
			os.Exit(1)
		}

		for _, entity := range entities {
			if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".toml") {
				fileName := strings.TrimSuffix(entity.Name(), ".toml")
				if strings.HasPrefix(fileName, toComplete) {
					fileNames = append(fileNames, fileName)
				}
			}
		}

		return fileNames, cobra.ShellCompDirectiveNoFileComp
	}

	return nil, cobra.ShellCompDirectiveNoFileComp
}

func (c configureCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--platform", "--project"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
