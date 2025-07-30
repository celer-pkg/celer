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

type configureCmd struct{}

func (c configureCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "configure",
		Short: "Configure to change platform or project.",
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			celer := configs.NewCeler()
			if err := celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			platform, _ := cmd.Flags().GetString("platform")
			project, _ := cmd.Flags().GetString("project")

			if platform != "" {
				c.selectPlatform(platform)
			} else if project != "" {
				c.selectProject(project)
			}
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			// Support flags completion.
			var suggestions []string
			for _, flag := range []string{"--platform", "--project"} {
				if strings.HasPrefix(flag, toComplete) {
					suggestions = append(suggestions, flag)
				}
			}

			return suggestions, cobra.ShellCompDirectiveNoFileComp
		},
	}

	// Register flags.
	command.Flags().String("platform", "", "configure platform.")
	command.Flags().String("project", "", "configure project.")

	// Support complete available platforms and projects.
	command.RegisterFlagCompletionFunc("platform", func(cmd *cobra.Command, args []string,
		toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.completion(dirs.ConfPlatformsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("project", func(cmd *cobra.Command, args []string,
		toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.completion(dirs.ConfProjectsDir, toComplete)
	})

	return command
}

func (c configureCmd) completion(dir, toComplete string) ([]string, cobra.ShellCompDirective) {
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

func (c configureCmd) selectPlatform(platformName string) {
	celer := configs.NewCeler()
	if err := celer.ChangePlatform(platformName); err != nil {
		configs.PrintError(err, "failed to select platform: %s.", platformName)
		os.Exit(1)
	}

	configs.PrintSuccess("current platform: %s.", platformName)
}

func (c configureCmd) selectProject(projectName string) {
	celer := configs.NewCeler()
	if err := celer.ChangeProject(projectName); err != nil {
		configs.PrintError(err, "failed to select project: %s.", projectName)
		os.Exit(1)
	}
	configs.PrintSuccess("celer is ready for project: %s.", projectName)
}
