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
	buildType  string
	jobNum     int
	offline    bool
	cacheDir   string
	cacheToken string
}

func (c configureCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "configure",
		Short: "Configure to change gloabal settings.",
		Run: func(cmd *cobra.Command, args []string) {
			switch {
			case c.platform != "":
				if err := c.celer.SetPlatform(c.platform); err != nil {
					configs.PrintError(err, "failed to set platform: %s.", c.platform)
					os.Exit(1)
				}
				configs.PrintSuccess("current platform: %s.", c.platform)

			case c.project != "":
				if err := c.celer.SetProject(c.project); err != nil {
					configs.PrintError(err, "failed to set project: %s.", c.project)
					os.Exit(1)
				}
				configs.PrintSuccess("current project: %s.", c.project)

			case c.buildType != "":
				if err := c.celer.SetBuildType(c.buildType); err != nil {
					configs.PrintError(err, "failed to set build type: %s.", c.buildType)
					os.Exit(1)
				}
				configs.PrintSuccess("current build type: %s.", c.buildType)

			case c.jobNum != -1:
				if err := c.celer.SetJobNum(c.jobNum); err != nil {
					configs.PrintError(err, "failed to set job num: %d.", c.jobNum)
					os.Exit(1)
				}
				configs.PrintSuccess("current job num: %d.", c.jobNum)

			case c.offline != c.celer.Global.Offline:
				if err := c.celer.SetOffline(c.offline); err != nil {
					configs.PrintError(err, "failed to set offline mode: %s.", expr.If(c.offline, "true", "false"))
					os.Exit(1)
				}
				configs.PrintSuccess("current offline mode: %s.", expr.If(c.offline, "true", "false"))

			case c.cacheDir != "" || c.cacheToken != "":
				cacheDir := expr.If(c.cacheDir != "", c.cacheDir, c.celer.CacheDir().Dir)
				cacheToken := expr.If(c.cacheToken != "", c.cacheToken, c.celer.CacheDir().Token)

				if err := c.celer.SetCacheDir(cacheDir, cacheToken); err != nil {
					configs.PrintError(err, "failed to set cache dir: %s, token: %s.", cacheDir, cacheToken)
					os.Exit(1)
				}

				configs.PrintSuccess("current cache dir: %s, token: %s.",
					expr.If(cacheDir != "", cacheDir, "empty"),
					expr.If(cacheToken != "", cacheToken, "empty"),
				)
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().StringVar(&c.platform, "platform", "", "configure platform.")
	command.Flags().StringVar(&c.project, "project", "", "configure project.")
	command.Flags().StringVar(&c.buildType, "build-type", "", "configure build type.")
	command.Flags().IntVar(&c.jobNum, "job-num", -1, "configure job num.")
	command.Flags().BoolVar(&c.offline, "offline", false, "configure offline mode.")
	command.Flags().StringVar(&c.cacheDir, "cache-dir", "", "configure cache dir.")
	command.Flags().StringVar(&c.cacheToken, "cache-token", "", "configure cache token.")

	// Support complete available platforms and projects.
	command.RegisterFlagCompletionFunc("platform", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.tomlFileCompletion(dirs.ConfPlatformsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("project", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.tomlFileCompletion(dirs.ConfProjectsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("build-type", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{
			"Release",
			"Debug",
			"RelWithDebInfo",
			"MinSizeRel",
		}, cobra.ShellCompDirectiveNoFileComp
	})
	command.RegisterFlagCompletionFunc("offline", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})

	command.MarkFlagsMutuallyExclusive("platform", "project", "build-type", "job-num", "cache-dir", "offline")
	return command
}

func (c configureCmd) tomlFileCompletion(dir, toComplete string) ([]string, cobra.ShellCompDirective) {
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
	commands := []string{
		"--platform",
		"--project",
		"--build-type",
		"--job-num",
		"--offline",
		"--cache-dir",
		"--cache-token",
		"--opt-level-Debug",
		"--opt-level-Release",
		"--opt-level-RelWithDebInfo",
		"--opt-level-MinSizeRel",
	}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
