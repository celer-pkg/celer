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

	// Binary cache options.
	binaryCacheDir string
	cacheToken     string

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
    --platform        Set the target platform (e.g., windows-amd64, linux-x64)
    
  Project Configuration:
    --project         Set the current project configuration
    
  Build Configuration:
    --build-type      Set the build type (Release, Debug, RelWithDebInfo, MinSizeRel)
    --jobs            Set the number of parallel build jobs
    
  Runtime Options:
    --offline         Enable/disable offline mode (true/false)
    --verbose         Enable/disable verbose output (true/false)
    
  Binary Cache Configuration:
    --binary-cache-dir    Set the binary cache directory path
    --binary-cache-token  Set the binary cache authentication token
    
  Proxy Configuration:
    --proxy-host      Set the proxy server hostname
    --proxy-port      Set the proxy server port number
    
  CCache Configuration:
  	--ccache-enabled      	Set whether ccache is enabled (true/false)
    --ccache-dir      		Set the ccache directory path
    --ccache-maxsize  		Set the maximum cache size (e.g., "5G", "1024M")
    --ccache-compress 		Enable/disable ccache compression (true/false)
	--ccache-remote-address Set remote storage address for ccache

Examples:
  celer configure --platform windows-amd64        # Set target platform
  celer configure --project myproject             # Set current project
  celer configure --build-type Release            # Set build type to Release
  celer configure --jobs 8                        # Use 8 parallel build jobs
  celer configure --offline true                  # Enable offline mode
  celer configure --verbose false                 # Disable verbose output
  celer configure --binary-cache-dir /tmp/cache   # Set binary cache directory
  celer configure --proxy-host proxy.example.com  # Set proxy host
  celer configure --proxy-port 8080               # Set proxy port
  celer configure --ccache-maxsize 5G             # Set ccache max size to 5GB`,
		Run: func(cmd *cobra.Command, args []string) {
			if err := c.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			flags := cmd.Flags()

			switch {
			case flags.Changed("platform"):
				if err := c.celer.SetPlatform(c.platform); err != nil {
					configs.PrintError(err, "failed to set platform: %s.", c.platform)
					return
				}
				configs.PrintSuccess("current platform: %s.", c.platform)

			case flags.Changed("project"):
				if err := c.celer.SetProject(c.project); err != nil {
					configs.PrintError(err, "failed to set project: %s.", c.project)
					return
				}
				configs.PrintSuccess("current project: %s.", c.project)

				// Auto configure platform.
				defaultPlatform := c.celer.Project().GetDefaultPlatform()
				if defaultPlatform != "" && c.celer.Global.Platform == "" {
					if err := c.celer.SetPlatform(defaultPlatform); err != nil {
						configs.PrintError(err, "failed to set platform: %s.", c.celer.Global.Platform)
						return
					}
					configs.PrintSuccess("platform is auto configured to %s defined in current project.", c.celer.Global.Platform)
				}

			case flags.Changed("build-type"):
				if err := c.celer.SetBuildType(c.buildType); err != nil {
					configs.PrintError(err, "failed to set build type: %s.", c.buildType)
					return
				}
				configs.PrintSuccess("current build type: %s.", c.buildType)

			case flags.Changed("jobs"):
				if err := c.celer.SetJobs(c.jobs); err != nil {
					configs.PrintError(err, "failed to set job num: %d.", c.jobs)
					return
				}
				configs.PrintSuccess("current job num: %d.", c.jobs)

			case flags.Changed("offline"):
				if err := c.celer.SetOffline(c.offline); err != nil {
					configs.PrintError(err, "failed to set offline mode: %s.", expr.If(c.offline, "true", "false"))
					return
				}
				configs.PrintSuccess("current offline mode: %s.", expr.If(c.offline, "true", "false"))

			case flags.Changed("verbose"):
				if err := c.celer.SetVerbose(c.verbose); err != nil {
					configs.PrintError(err, "failed to set verbose mode: %s.", expr.If(c.verbose, "true", "false"))
					return
				}
				configs.PrintSuccess("current verbose mode: %s.", expr.If(c.verbose, "true", "false"))

			case flags.Changed("binary-cache-dir") || flags.Changed("binary-cache-token"):
				binaryCache := c.celer.BinaryCache()
				var binaryCacheDir string
				if binaryCache != nil {
					binaryCacheDir = binaryCache.GetDir()
				} else {
					binaryCacheDir = c.binaryCacheDir
				}
				if err := c.celer.SetBinaryCache(binaryCacheDir, c.cacheToken); err != nil {
					configs.PrintError(err, "failed to set cache dir: %s.", binaryCacheDir)
					return
				}
				configs.PrintSuccess("current cache dir: %s.", expr.If(binaryCacheDir != "", binaryCacheDir, "empty"))

			case flags.Changed("proxy-host"), flags.Changed("proxy-port"):
				if err := c.celer.SetProxy(c.proxy.Host, c.proxy.Port); err != nil {
					configs.PrintError(err, "failed to set proxy: %s:%d.", c.proxy.Host, c.proxy.Port)
					return
				}
				configs.PrintSuccess("current proxy: %s:%d.", c.proxy.Host, c.proxy.Port)

			case flags.Changed("ccache-enabled"):
				if err := c.celer.SetCCacheEnabled(c.ccache.Enabled); err != nil {
					configs.PrintError(err, "failed to update ccache enabled.")
					return
				}
				configs.PrintSuccess("current ccache enabled: %s.", expr.If(c.ccache.Enabled, "true", "false"))

			case flags.Changed("ccache-dir"):
				if err := c.celer.SetCCacheDir(c.ccache.Dir); err != nil {
					configs.PrintError(err, "failed to update ccache dir.")
					return
				}
				configs.PrintSuccess("current ccache dir: %s.", c.ccache.Dir)

			case flags.Changed("ccache-maxsize"):
				if err := c.celer.SetCCacheMaxSize(c.ccache.MaxSize); err != nil {
					configs.PrintError(err, "failed to update ccache.maxsize.")
					return
				}
				configs.PrintSuccess("current ccache maxsize: %s.", c.ccache.MaxSize)

			case flags.Changed("ccache-compress"):
				if err := c.celer.SetCCacheCompressed(c.ccache.Compress); err != nil {
					configs.PrintError(err, "failed to update ccache.compress.")
					return
				}
				configs.PrintSuccess("current ccache compress: %s.", expr.If(c.ccache.Compress, "true", "false"))

			case flags.Changed("ccache-remote-storage"):
				if err := c.celer.SetCCacheRemoteStorage(c.ccache.RemoteStorage); err != nil {
					configs.PrintError(err, "failed to update ccache.remote_storage.")
					return
				}
				configs.PrintSuccess("current ccache remote storage: %s.", c.ccache.RemoteStorage)
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
	flags.StringVar(&c.binaryCacheDir, "binary-cache-dir", "", "configure binary cache dir.")
	flags.StringVar(&c.cacheToken, "binary-cache-token", "", "configure binary cache token.")

	// Proxy flags.
	flags.StringVar(&c.proxy.Host, "proxy-host", "", "configure proxy host.")
	flags.IntVar(&c.proxy.Port, "proxy-port", 0, "configure proxy port.")

	// CCache flags.
	flags.BoolVar(&c.ccache.Enabled, "ccache-enabled", false, "configure ccache enabled.")
	flags.StringVar(&c.ccache.Dir, "ccache-dir", "", "configure ccache dir.")
	flags.StringVar(&c.ccache.MaxSize, "ccache-maxsize", "", "configure ccache maxsize.")
	flags.BoolVar(&c.ccache.Compress, "ccache-compress", false, "configure ccache compress.")
	flags.StringVar(&c.ccache.RemoteStorage, "ccache-remote-storage", "", "configure ccache remote storage.")

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
	command.RegisterFlagCompletionFunc("ccache-compress", func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
	})

	command.MarkFlagsMutuallyExclusive("platform", "project", "build-type", "jobs", "offline", "verbose")
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
		"--binary-cache-dir",
		"--binary-cache-token",
		"--proxy-host",
		"--proxy-port",
		"--ccache-enabled",
		"--ccache-compress",
		"--ccache-dir",
		"--ccache-maxsize",
		"--ccache-remote-storage",
	}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
