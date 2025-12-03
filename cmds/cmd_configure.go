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
	proxy      configs.Proxy
	ccache     configs.CCache
}

func (c configureCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "configure",
		Short: "Configure to change global settings.",
		Run: func(cmd *cobra.Command, args []string) {
			flags := cmd.Flags()

			switch {
			case flags.Changed("platform"):
				if err := c.celer.SetPlatform(c.platform); err != nil {
					configs.PrintError(err, "failed to set platform: %s.", c.platform)
					os.Exit(1)
				}
				configs.PrintSuccess("current platform: %s.", c.platform)

			case flags.Changed("project"):
				if err := c.celer.SetProject(c.project); err != nil {
					configs.PrintError(err, "failed to set project: %s.", c.project)
					os.Exit(1)
				}
				configs.PrintSuccess("current project: %s.", c.project)

				// Auto configure platform.
				defaultPlatform := c.celer.Project().GetDefaultPlatform()
				if defaultPlatform != "" && c.celer.Global.Platform == "" {
					if err := c.celer.SetPlatform(defaultPlatform); err != nil {
						configs.PrintError(err, "failed to set platform: %s.", c.celer.Global.Platform)
						os.Exit(1)
					}
					configs.PrintSuccess("platform is auto configured to %s defined in current project.", c.celer.Global.Platform)
				}

			case flags.Changed("build-type"):
				if err := c.celer.SetBuildType(c.buildType); err != nil {
					configs.PrintError(err, "failed to set build type: %s.", c.buildType)
					os.Exit(1)
				}
				configs.PrintSuccess("current build type: %s.", c.buildType)

			case flags.Changed("jobs"):
				if err := c.celer.SetJobs(c.jobs); err != nil {
					configs.PrintError(err, "failed to set job num: %d.", c.jobs)
					os.Exit(1)
				}
				configs.PrintSuccess("current job num: %d.", c.jobs)

			case flags.Changed("offline"):
				if err := c.celer.SetOffline(c.offline); err != nil {
					configs.PrintError(err, "failed to set offline mode: %s.", expr.If(c.offline, "true", "false"))
					os.Exit(1)
				}
				configs.PrintSuccess("current offline mode: %s.", expr.If(c.offline, "true", "false"))

			case flags.Changed("verbose"):
				if err := c.celer.SetVerbose(c.verbose); err != nil {
					configs.PrintError(err, "failed to set verbose mode: %s.", expr.If(c.verbose, "true", "false"))
					os.Exit(1)
				}
				configs.PrintSuccess("current verbose mode: %s.", expr.If(c.verbose, "true", "false"))

			case flags.Changed("binary-cache-dir") || flags.Changed("binary-cache-token"):
				cacheDir := expr.If(c.cacheDir != "", c.cacheDir, c.celer.BinaryCache().GetDir())
				if err := c.celer.SetBinaryCache(cacheDir, c.cacheToken); err != nil {
					configs.PrintError(err, "failed to set cache dir: %s.", cacheDir)
					os.Exit(1)
				}
				configs.PrintSuccess("current cache dir: %s.", expr.If(cacheDir != "", cacheDir, "empty"))

			case flags.Changed("proxy-host"), flags.Changed("proxy-port"):
				if err := c.celer.SetProxy(c.proxy.Host, c.proxy.Port); err != nil {
					configs.PrintError(err, "failed to set proxy: %s:%d.", c.proxy.Host, c.proxy.Port)
					os.Exit(1)
				}
				configs.PrintSuccess("current proxy: %s:%d.", c.proxy.Host, c.proxy.Port)

			case flags.Changed("ccache-dir"):
				if err := c.celer.SetCCacheDir(c.ccache.Dir); err != nil {
					configs.PrintError(err, "failed to update ccache dir.")
					os.Exit(1)
				}
				configs.PrintSuccess("current ccache dir: %s.", c.ccache.Dir)

			case flags.Changed("ccache-maxsize"):
				if err := c.celer.SetCCacheMaxSize(c.ccache.MaxSize); err != nil {
					configs.PrintError(err, "failed to update ccache.maxsize.")
					os.Exit(1)
				}
				configs.PrintSuccess("current ccache maxsize: %s.", c.ccache.MaxSize)

			case flags.Changed("ccache-compress"):
				if err := c.celer.CompressCCache(c.ccache.Compress); err != nil {
					configs.PrintError(err, "failed to update ccache.compress.")
					os.Exit(1)
				}
				configs.PrintSuccess("current ccache compress: %s.", expr.If(c.ccache.Compress, "true", "false"))
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	flags := command.Flags()
	flags.StringVar(&c.platform, "platform", "", "configure platform.")
	flags.StringVar(&c.project, "project", "", "configure project.")
	flags.StringVar(&c.buildType, "build-type", "", "configure build type.")
	flags.IntVar(&c.jobs, "jobs", 0, "configure jobs.")
	flags.BoolVar(&c.offline, "offline", false, "configure offline mode.")
	flags.BoolVar(&c.verbose, "verbose", false, "configure verbose mode.")

	// Binary cache flags.
	flags.StringVar(&c.cacheDir, "binary-cache-dir", "", "configure binary cache dir.")
	flags.StringVar(&c.cacheToken, "binary-cache-token", "", "configure binary cache token.")

	// Proxy flags.
	flags.StringVar(&c.proxy.Host, "proxy-host", "", "configure proxy host.")
	flags.IntVar(&c.proxy.Port, "proxy-port", 0, "configure proxy port.")

	// CCache flags.
	flags.StringVar(&c.ccache.Dir, "ccache-dir", "", "configure ccache dir.")
	flags.StringVar(&c.ccache.MaxSize, "ccache-maxsize", "", "configure ccache maxsize.")
	flags.BoolVar(&c.ccache.Compress, "ccache-compress", false, "configure ccache compress.")

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
	command.RegisterFlagCompletionFunc("verbose", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})
	command.RegisterFlagCompletionFunc("ccache-compress", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})

	command.MarkFlagsMutuallyExclusive("platform", "project", "build-type", "jobs", "offline", "verbose", "binary-cache-dir", "binary-cache-token", "proxy-host", "proxy-port", "ccache-dir", "ccache-maxsize", "ccache-compress")
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
		"--binary-cache-dir",
		"--binary-cache-token",
		"--proxy-host",
		"--proxy-port",
		"--ccache-compress",
		"--ccache-dir",
		"--ccache-maxsize",
	}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
