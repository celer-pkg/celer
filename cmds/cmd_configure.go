package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type configureCmd struct {
	celer      *configs.Celer
	platform   string
	project    string
	cacheDir   string
	cacheToken string
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

			switch {
			case c.platform != "":
				c.setPlatform(c.platform)

			case c.project != "":
				c.setProject(c.project)

			case c.cacheDir != "":
				c.setCacheDir(c.cacheDir, c.cacheToken)
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().StringVar(&c.platform, "platform", "", "configure platform.")
	command.Flags().StringVar(&c.project, "project", "", "configure project.")
	command.Flags().StringVar(&c.cacheDir, "cache-dir", "", "configure cache dir.")
	command.Flags().StringVar(&c.cacheToken, "cache-token", "", "configure cache token.")

	// Support complete available platforms and projects.
	command.RegisterFlagCompletionFunc("platform", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.flagCompletion(dirs.ConfPlatformsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("project", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.flagCompletion(dirs.ConfProjectsDir, toComplete)
	})

	command.MarkFlagsMutuallyExclusive("platform", "project", "cachedir")
	return command
}

func (c configureCmd) setPlatform(platformName string) {
	if err := c.celer.SetPlatform(platformName); err != nil {
		configs.PrintError(err, "failed to set platform: %s.", platformName)
		os.Exit(1)
	}

	configs.PrintSuccess("current platform: %s.", platformName)
}

func (c configureCmd) setProject(projectName string) {
	if err := c.celer.SetProject(projectName); err != nil {
		configs.PrintError(err, "failed to set project: %s.", projectName)
		os.Exit(1)
	}
	configs.PrintSuccess("current project: %s.", projectName)
}

func (c configureCmd) setCacheDir(dir, token string) {
	cacheDirChanged := c.Command().Flags().Changed("cache-dir")
	cacheTokenChanged := c.Command().Flags().Changed("cache-token")

	finalDir := expr.If(cacheDirChanged, dir, c.celer.CacheDir().Dir)
	finalToken := expr.If(cacheTokenChanged, token, c.celer.CacheDir().Token)

	if err := c.celer.SetCacheDir(finalDir, finalToken); err != nil {
		configs.PrintError(err, "failed to set cache dir: %s, token: %s.", dir, token)
		os.Exit(1)
	}

	configs.PrintSuccess("current cache dir: %s, token: %s.", dir, token)
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
	commands := []string{"--platform", "--project", "--cache-dir", "--cache-token"}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
