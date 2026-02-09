package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"os"
	"strings"

	"github.com/spf13/cobra"
)

type configureCmd struct {
	celer     *configs.Celer
	platform  string
	project   string
	buildType string
	jobs      int
	offline   bool
	verbose   bool

	// Package cache options.
	packageCacheDir   string
	packageCacheToken string

	proxy  configs.Proxy
	ccache configs.CCache
}

func (c *configureCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "configure",
		Short: "Configure global settings for your workspace.",
		Long: `Configure global settings for your workspace.

This command allows you to modify various configuration settings that affect
how celer works. You can only configure one setting at a time due to the
mutually exclusive nature of the flags.

Available Configuration Options:

  Platform Configuration:
    --platform        Set the target platform (e.g., windows-x86_64, linux-x64)
    
  Project Configuration:
    --project         Set the current project configuration
    
  Build Configuration:
    --build-type      Set the build type (Release, Debug, RelWithDebInfo, MinSizeRel)
    --jobs            Set the number of parallel build jobs
    
  Runtime Options:
    --offline         Enable/disable offline mode (true/false)
    --verbose         Enable/disable verbose output (true/false)
    
  Package Cache Configuration:
    --package-cache-dir    Set the package cache directory path
    --package-cache-token  Set the package cache authentication token
    
  Proxy Configuration:
    --proxy-host      Set the proxy server hostname
    --proxy-port      Set the proxy server port number
    
  CCache Configuration:
  	--ccache-enabled      	Set whether ccache is enabled (true/false)
    --ccache-dir      		Set the ccache directory path
    --ccache-maxsize  		Set the maximum cache size (e.g., "5G", "1024M")
	--ccache-remote-storage Set remote storage address for ccache

Examples:
  celer configure --platform windows-x86_64        # Set target platform
  celer configure --project myproject             # Set current project
  celer configure --build-type Release            # Set build type to Release
  celer configure --jobs 8                        # Use 8 parallel build jobs
  celer configure --offline true                  # Enable offline mode
  celer configure --verbose false                 # Disable verbose output
  celer configure --package-cache-dir /tmp/cache  # Set package cache directory
  celer configure --proxy-host proxy.example.com  # Set proxy host
  celer configure --proxy-port 8080               # Set proxy port
  celer configure --ccache-maxsize 5G             # Set ccache max size to 5GB`,
		RunE: func(cmd *cobra.Command, args []string) error {
			if err := c.celer.Init(); err != nil {
				return configs.PrintError(err, "failed to init celer.")
			}

			flags := cmd.Flags()

			switch {
			case flags.Changed("platform"):
				if err := c.celer.SetPlatform(c.platform); err != nil {
					return configs.PrintError(err, "failed to set platform.")
				}
				configs.PrintSuccess("current platform: %s.", c.platform)

			case flags.Changed("project"):
				if err := c.celer.SetProject(c.project); err != nil {
					return configs.PrintError(err, "failed to set project: %s.", c.project)
				}
				configs.PrintSuccess("current project: %s.", c.project)

				// Auto configure platform.
				targetPlatform := c.celer.Project().GetTargetPlatform()
				if targetPlatform != "" && c.celer.Global.Platform == "" {
					if err := c.celer.SetPlatform(targetPlatform); err != nil {
						return configs.PrintError(err, "failed to set platform: %s.", c.celer.Global.Platform)
					}
					configs.PrintSuccess("current platform: %s => Default target platform defined in project", c.celer.Global.Platform)
				}

			case flags.Changed("build-type"):
				if err := c.celer.SetBuildType(c.buildType); err != nil {
					return configs.PrintError(err, "failed to set build type: %s.", c.buildType)
				}
				configs.PrintSuccess("current build type: %s.", c.buildType)

			case flags.Changed("jobs"):
				if err := c.celer.SetJobs(c.jobs); err != nil {
					return configs.PrintError(err, "failed to set job num: %d.", c.jobs)
				}
				configs.PrintSuccess("current job num: %d.", c.jobs)

			case flags.Changed("offline"):
				if err := c.celer.SetOffline(c.offline); err != nil {
					return configs.PrintError(err, "failed to set offline mode: %s.", expr.If(c.offline, "true", "false"))
				}
				configs.PrintSuccess("current offline mode: %s.", expr.If(c.offline, "true", "false"))

			case flags.Changed("verbose"):
				if err := c.celer.SetVerbose(c.verbose); err != nil {
					return configs.PrintError(err, "failed to set verbose mode: %s.", expr.If(c.verbose, "true", "false"))
				}
				configs.PrintSuccess("current verbose mode: %s.", expr.If(c.verbose, "true", "false"))

			case flags.Changed("package-cache-dir"):
				if err := c.celer.SetPackageCacheDir(c.packageCacheDir); err != nil {
					return configs.PrintError(err, "failed to set package cache dir: %s.", c.packageCacheDir)
				}
				configs.PrintSuccess("current cache dir: %s.", expr.If(c.packageCacheDir != "", c.packageCacheDir, "empty"))
			case flags.Changed("package-cache-token"):
				if err := c.celer.SetPackageCacheToken(c.packageCacheToken); err != nil {
					return configs.PrintError(err, "failed to set package cache token: %s.", c.packageCacheToken)
				}
				configs.PrintSuccess("current cache token: %s.", expr.If(c.packageCacheToken != "", c.packageCacheToken, "empty"))

			case flags.Changed("proxy-host"):
				if err := c.celer.SetProxyHost(c.proxy.Host); err != nil {
					return configs.PrintError(err, "failed to set proxy host: %s.", c.proxy.Host)
				}
				configs.PrintSuccess("current proxy host: %s.", c.proxy.Host)

			case flags.Changed("proxy-port"):
				if err := c.celer.SetProxyPort(c.proxy.Port); err != nil {
					return configs.PrintError(err, "failed to set proxy port: %d.", c.proxy.Port)
				}
				configs.PrintSuccess("current proxy port: %d.", c.proxy.Port)

			case flags.Changed("ccache-enabled"):
				if err := c.celer.SetCCacheEnabled(c.ccache.Enabled); err != nil {
					return configs.PrintError(err, "failed to update ccache enabled.")
				}
				configs.PrintSuccess("current ccache enabled: %s.", expr.If(c.ccache.Enabled, "true", "false"))

			case flags.Changed("ccache-dir"):
				if err := c.celer.SetCCacheDir(c.ccache.Dir); err != nil {
					return configs.PrintError(err, "failed to update ccache dir.")
				}
				configs.PrintSuccess("current ccache dir: %s.", c.ccache.Dir)

			case flags.Changed("ccache-maxsize"):
				if err := c.celer.SetCCacheMaxSize(c.ccache.MaxSize); err != nil {
					return configs.PrintError(err, "failed to update ccache.maxsize.")
				}
				configs.PrintSuccess("current ccache maxsize: %s.", c.ccache.MaxSize)

			case flags.Changed("ccache-remote-storage"):
				if err := c.celer.SetCCacheRemoteStorage(c.ccache.RemoteStorage); err != nil {
					return configs.PrintError(err, "failed to update ccache.remote_storage.")
				}
				configs.PrintSuccess("current ccache remote storage: %s.", c.ccache.RemoteStorage)

			case flags.Changed("ccache-remote-only"):
				if err := c.celer.SetCCacheRemoteOnly(c.ccache.RemoteOnly); err != nil {
					return configs.PrintError(err, "failed to update ccache.remote_only.")
				}
				configs.PrintSuccess("current ccache remote only: %s.", expr.If(c.ccache.RemoteOnly, "true", "false"))
			}

			return nil
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

	// Package cache flags.
	flags.StringVar(&c.packageCacheDir, "package-cache-dir", "", "configure package cache dir.")
	flags.StringVar(&c.packageCacheToken, "package-cache-token", "", "configure package cache token.")

	// Proxy flags.
	flags.StringVar(&c.proxy.Host, "proxy-host", "", "configure proxy host.")
	flags.IntVar(&c.proxy.Port, "proxy-port", 0, "configure proxy port.")

	// CCache flags.
	flags.BoolVar(&c.ccache.Enabled, "ccache-enabled", false, "configure ccache enabled.")
	flags.StringVar(&c.ccache.Dir, "ccache-dir", "", "configure ccache dir.")
	flags.StringVar(&c.ccache.MaxSize, "ccache-maxsize", "", "configure ccache maxsize.")
	flags.StringVar(&c.ccache.RemoteStorage, "ccache-remote-storage", "", "configure ccache remote storage.")
	flags.BoolVar(&c.ccache.RemoteOnly, "ccache-remote-only", false, "configure ccache remote only.")

	// Support complete available platforms and projects.
	command.RegisterFlagCompletionFunc("platform", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.tomlFileCompletion(dirs.ConfPlatformsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("project", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return c.tomlFileCompletion(dirs.ConfProjectsDir, toComplete)
	})
	command.RegisterFlagCompletionFunc("build-type", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"Release", "Debug", "RelWithDebInfo", "MinSizeRel"}, cobra.ShellCompDirectiveNoFileComp
	})
	command.RegisterFlagCompletionFunc("offline", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})
	command.RegisterFlagCompletionFunc("verbose", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})

	// CCache flag completions.
	command.RegisterFlagCompletionFunc("ccache-enabled", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})
	command.RegisterFlagCompletionFunc("ccache-remote-only", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})

	command.MarkFlagsMutuallyExclusive("platform", "project", "build-type", "jobs", "offline", "verbose")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true

	return command
}

func (c *configureCmd) tomlFileCompletion(dir, toComplete string) ([]string, cobra.ShellCompDirective) {
	var fileNames []string
	if fileio.PathExists(dir) {
		entities, err := os.ReadDir(dir)
		if err != nil {
			return nil, cobra.ShellCompDirectiveNoFileComp
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

func (c *configureCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	commands := []string{
		"--platform",
		"--project",
		"--build-type",
		"--jobs",
		"--offline",
		"--verbose",
		"--package-cache-dir",
		"--package-cache-token",
		"--proxy-host",
		"--proxy-port",
		"--ccache-enabled",
		"--ccache-dir",
		"--ccache-maxsize",
		"--ccache-remote-storage",
		"--ccache-remote-only",
	}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
