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
	jobs       int
	offline    bool
	verbose    bool
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

				// Auto configure platform.
				if c.celer.Project().Platform != "" && c.celer.Global.Platform == "" {
					if err := c.celer.SetPlatform(c.celer.Project().Platform); err != nil {
						configs.PrintError(err, "failed to set platform: %s.", c.celer.Global.Platform)
						os.Exit(1)
					}
					configs.PrintSuccess("platform is auto configured to %s defined in current project.", c.celer.Global.Platform)
				}

			case c.buildType != "":
				if err := c.celer.SetBuildType(c.buildType); err != nil {
					configs.PrintError(err, "failed to set build type: %s.", c.buildType)
					os.Exit(1)
				}
				configs.PrintSuccess("current build type: %s.", c.buildType)

			case c.jobs != -1:
				if err := c.celer.SetJobs(c.jobs); err != nil {
					configs.PrintError(err, "failed to set job num: %d.", c.jobs)
					os.Exit(1)
				}
				configs.PrintSuccess("current job num: %d.", c.jobs)

			case c.offline != c.celer.Global.Offline:
				if err := c.celer.SetOffline(c.offline); err != nil {
					configs.PrintError(err, "failed to set offline mode: %s.", expr.If(c.offline, "true", "false"))
					os.Exit(1)
				}
				configs.PrintSuccess("current offline mode: %s.", expr.If(c.offline, "true", "false"))

			case c.verbose != c.celer.Verbose():
				if err := c.celer.SetVerbose(c.verbose); err != nil {
					configs.PrintError(err, "failed to set verbose mode: %s.", expr.If(c.verbose, "true", "false"))
					os.Exit(1)
				}
				configs.PrintSuccess("current verbose mode: %s.", expr.If(c.verbose, "true", "false"))

			case c.cacheDir != "" || c.cacheToken != "":
				cacheDir := expr.If(c.cacheDir != "", c.cacheDir, c.celer.CacheDir().Dir)
				if err := c.celer.SetCacheDir(cacheDir, c.cacheToken); err != nil {
					configs.PrintError(err, "failed to set cache dir: %s.", cacheDir)
					os.Exit(1)
				}

				configs.PrintSuccess("current cache dir: %s.", expr.If(cacheDir != "", cacheDir, "empty"))
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	flags := command.Flags()
	flags.StringVar(&c.platform, "platform", "", "configure platform.")
	flags.StringVar(&c.project, "project", "", "configure project.")
	flags.StringVar(&c.buildType, "build-type", "", "configure build type.")
	flags.IntVar(&c.jobs, "jobs", -1, "configure jobs.")
	flags.BoolVar(&c.offline, "offline", false, "configure offline mode.")
	flags.BoolVar(&c.verbose, "verbose", false, "configure verbose mode.")
	flags.StringVar(&c.cacheDir, "cache-dir", "", "configure cache dir.")
	flags.StringVar(&c.cacheToken, "cache-token", "", "configure cache token.")

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

	command.MarkFlagsMutuallyExclusive("platform", "project", "build-type", "jobs", "offline", "verbose", "cache-dir")
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
		"--jobs",
		"--offline",
		"--verbose",
		"--cache-dir",
		"--cache-token",
	}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
