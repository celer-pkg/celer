package cmds

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/spf13/cobra"
	"github.com/spf13/pflag"
)

type configureCmd struct {
	celer     *configs.Celer
	platform  string
	project   string
	buildType string
	downloads string
	jobs      int
	offline   bool
	verbose   bool

	// Support port url and ref.
	port    string
	portUrl string
	portRef string

	// Pkg-cache flags, bound to the backend configs and shared options directly.
	pkgCacheFS      pkgcache.FS
	pkgCacheMinio   pkgcache.Minio
	pkgCacheOptions pkgcache.Options
	proxy           configs.Proxy
	ccache          configs.CCache
}

var flagGroup = map[string]string{
	"platform":                  "platform",
	"project":                   "project",
	"build-type":                "build-type",
	"downloads":                 "downloads",
	"jobs":                      "jobs",
	"offline":                   "offline",
	"verbose":                   "verbose",
	"pkgcache-fs-dir":           "pkgcache",
	"pkgcache-minio-host":       "pkgcache",
	"pkgcache-minio-access-key": "pkgcache",
	"pkgcache-minio-secret-key": "pkgcache",
	"pkgcache-writable":         "pkgcache",
	"pkgcache-cache-downloads":  "pkgcache",
	"pkgcache-cache-artifacts":  "pkgcache",
	"pkgcache-cache-repos":      "pkgcache",
	"proxy-host":                "proxy",
	"proxy-port":                "proxy",
	"proxy-remove":              "proxy",
	"ccache-enabled":            "ccache",
	"ccache-dir":                "ccache",
	"ccache-maxsize":            "ccache",
	"ccache-remote-storage":     "ccache",
	"ccache-remote-only":        "ccache",
	"port":                      "port",
	"port-url":                  "port",
	"port-ref":                  "port",
}

func (c *configureCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "configure",
		Short: "Configure settings for your workspace.",
		Long: `Configure settings for your workspace.

This command allows you to modify various configuration settings that affect
how celer works. You can configure one setting or one related group of settings
in a single command (do not mix flags from different groups).

Available Configuration Options:

  Platform Configuration:
    --platform                  Set the target platform (e.g., x86_64-linux-ubuntu-22.04-gcc-11.5.0)

  Project Configuration:
    --project                   Set the current project configuration

  Build Configuration:
    --build-type                Set the build type (Release, Debug, RelWithDebInfo, MinSizeRel)
    --downloads                 Set the download directory
    --jobs                      Set the number of parallel build jobs

  Runtime Options:
    --offline                   Enable/disable offline mode (true/false)
    --verbose                   Enable/disable verbose output (true/false)

  PkgCache Configuration (fs and minio are mutually exclusive backends):
    --pkgcache-fs-dir             Set the fs backend directory path
    --pkgcache-minio-host         Set the minio backend host (e.g. http://minio.example.com:9000)
    --pkgcache-minio-access-key   Set the minio access key
    --pkgcache-minio-secret-key   Set the minio secret key

  PkgCache Options (shared by all backends, all enabled on first backend configuration):
    --pkgcache-writable           Set whether the package cache is writable (true/false)
    --pkgcache-cache-downloads    Cache downloaded sources into the package cache (true/false)
    --pkgcache-cache-artifacts    Cache built artifacts into the package cache (true/false)
    --pkgcache-cache-repos        Cache source repos into the package cache (true/false)

  Proxy Configuration:
    --proxy-host                Set the proxy server hostname
    --proxy-port                Set the proxy server port number
    --proxy-remove				Remove http/https proxy

  CCache Configuration:
    --ccache-enabled            Enable/disable ccache (true/false)
    --ccache-dir                Set the ccache directory path
    --ccache-maxsize            Set the maximum cache size (e.g., "5G", "1024M")
    --ccache-remote-storage     Set remote storage address for ccache (e.g., http://host:port/path)
    --ccache-remote-only        Use remote ccache only, skip local cache (true/false)

  Port Configuration:
    --port                      Target port to update, in name@version form (e.g., eigen@3.4.0)
    --port-url                  New source URL for the port (requires --port)
    --port-ref                  New ref for the port — branch, tag, or commit (requires --port)

Examples:
  celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0  # Set target platform
  celer configure --project=myproject                              # Set current project
  celer configure --build-type=Release                             # Set build type to Release
  celer configure --downloads=/home/xxx/Downloads                  # Set download directory
  celer configure --jobs=8                                         # Use 8 parallel build jobs
  celer configure --offline=true                                   # Enable offline mode
  celer configure --verbose=false                                  # Disable verbose output
  celer configure --pkgcache-fs-dir=/tmp/cache                  # Set pkgcache.fs directory
  celer configure --pkgcache-minio-host=http://srv:9000 \
                  --pkgcache-minio-access-key=xxx \
                  --pkgcache-minio-secret-key=yyy               # Configure pkgcache.minio
  celer configure --pkgcache-writable=false                     # Disable pkgcache write
  celer configure --pkgcache-cache-artifacts=true               # Cache built artifacts
  celer configure --proxy-host=proxy.example.com                   # Set proxy host
  celer configure --proxy-port=8080                                # Set proxy port
  celer configure --proxy-remove                                   # Remove http/https proxy
  celer configure --ccache-enabled=true                            # Enable ccache
  celer configure --ccache-maxsize=5G                              # Set ccache max size to 5GB
  celer configure --ccache-remote-storage=http://srv:8080/ccache   # Set ccache remote storage
  celer configure --ccache-remote-only=true                        # Use remote ccache only
  celer configure --port=eigen@3.4.0 --port-ref=3.4.1              # Pin a port to a new ref
  celer configure --port=eigen@3.4.0 --port-url=https://example.com/eigen.git --port-ref=main
                                                                   # Override both url and ref`,
		Args: cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			flags := cmd.Flags()

			// Init must be done before configure any.
			if err := c.checkIfInitialized(); err != nil {
				return color.PrintError(err, "please run `celer init` first.")
			}

			// Init celer with options, allow skip platform or project
			// not found when configure platform or project.
			var initOpt configs.InitOption
			if flags.Changed("platform") || flags.Changed("project") {
				initOpt.SkipPlatform = true
				initOpt.SkipProject = true
			}

			if err := c.celer.InitWithOptions(initOpt); err != nil {
				return fmt.Errorf("failed to init celer -> %w", err)
			}

			changedCount := 0
			activeGroups := map[string]bool{}
			for name, group := range flagGroup {
				if flags.Changed(name) {
					changedCount++
					activeGroups[group] = true
				}
			}

			if changedCount == 0 {
				return color.PrintError(errors.ErrNoConfigFlagProvided,
					"please specify exactly one configuration flag.",
				)
			}
			if len(activeGroups) > 1 {
				return color.PrintError(
					fmt.Errorf("flags from different groups were provided"),
					"please configure only one setting or one related group at a time.",
				)
			}

			if err := c.configureMain(flags); err != nil {
				return err
			}
			if err := c.configureCCache(flags); err != nil {
				return err
			}
			if err := c.configureProxy(flags); err != nil {
				return err
			}
			if err := c.configurePkgCache(flags); err != nil {
				return err
			}
			if err := c.configurePort(flags); err != nil {
				return err
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
	flags.StringVar(&c.downloads, "downloads", "", "configure downloads.")
	flags.IntVar(&c.jobs, "jobs", 0, "configure jobs.")
	flags.BoolVar(&c.offline, "offline", false, "configure offline mode.")
	flags.BoolVar(&c.verbose, "verbose", false, "configure verbose mode.")

	// Pkg-cache flags, grouped by backend (fs or minio).
	flags.StringVar(&c.pkgCacheFS.Dir, "pkgcache-fs-dir", "", "configure pkgcache.fs dir.")

	flags.StringVar(&c.pkgCacheMinio.Host, "pkgcache-minio-host", "", "configure pkgcache.minio host (e.g. http://minio.example.com:9000).")
	flags.StringVar(&c.pkgCacheMinio.AccessKey, "pkgcache-minio-access-key", "", "configure pkgcache.minio access key.")
	flags.StringVar(&c.pkgCacheMinio.SecretKey, "pkgcache-minio-secret-key", "", "configure pkgcache.minio secret key.")

	// Options shared by all pkgcache backends, all enabled by default on
	// the first backend configuration.
	flags.BoolVar(&c.pkgCacheOptions.Writable, "pkgcache-writable", true, "configure pkgcache writable.")
	flags.BoolVar(&c.pkgCacheOptions.Downloads, "pkgcache-cache-downloads", true, "configure pkgcache to cache downloads.")
	flags.BoolVar(&c.pkgCacheOptions.Artifacts, "pkgcache-cache-artifacts", true, "configure pkgcache to cache artifacts.")
	flags.BoolVar(&c.pkgCacheOptions.Repos, "pkgcache-cache-repos", true, "configure pkgcache to cache repos.")

	// Proxy flags.
	flags.StringVar(&c.proxy.Host, "proxy-host", "", "configure proxy host.")
	flags.IntVar(&c.proxy.Port, "proxy-port", 0, "configure proxy port.")
	flags.BoolVar(&c.proxy.Remove, "proxy-remove", false, "remove http/https proxy.")

	// CCache flags.
	flags.BoolVar(&c.ccache.Enabled, "ccache-enabled", false, "configure ccache enabled.")
	flags.StringVar(&c.ccache.Dir, "ccache-dir", "", "configure ccache dir.")
	flags.StringVar(&c.ccache.MaxSize, "ccache-maxsize", "", "configure ccache maxsize.")
	flags.StringVar(&c.ccache.RemoteStorage, "ccache-remote-storage", "", "configure ccache remote storage.")
	flags.BoolVar(&c.ccache.RemoteOnly, "ccache-remote-only", false, "configure ccache remote only.")

	// Port flags.
	flags.StringVar(&c.port, "port", "", "port name@version to configure (e.g. rmw_zenoh_cpp@humble)")
	flags.StringVar(&c.portUrl, "port-url", "", "configure port url")
	flags.StringVar(&c.portRef, "port-ref", "", "configure port ref (branch/tag/commit)")

	// Support complete available platforms and projects.
	command.RegisterFlagCompletionFunc("platform", platformCompletgion)
	command.RegisterFlagCompletionFunc("project", projectCompletgion)
	command.RegisterFlagCompletionFunc("build-type", buildTypeCompletion)
	command.RegisterFlagCompletionFunc("offline", boolCompletion)
	command.RegisterFlagCompletionFunc("verbose", boolCompletion)

	// PkgCache flag completions.
	command.RegisterFlagCompletionFunc("pkgcache-writable", boolCompletion)
	command.RegisterFlagCompletionFunc("pkgcache-cache-downloads", boolCompletion)
	command.RegisterFlagCompletionFunc("pkgcache-cache-artifacts", boolCompletion)
	command.RegisterFlagCompletionFunc("pkgcache-cache-repos", boolCompletion)

	// CCache flag completions.
	command.RegisterFlagCompletionFunc("ccache-enabled", boolCompletion)
	command.RegisterFlagCompletionFunc("ccache-remote-only", boolCompletion)

	// Proxy remove completions.
	command.RegisterFlagCompletionFunc("proxy-remove", boolCompletion)

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true

	return command
}

func (c configureCmd) checkIfInitialized() error {
	if !fileio.PathExists(filepath.Join(dirs.WorkspaceDir, "celer.toml")) {
		return fmt.Errorf("celer.toml not found")
	}
	if !fileio.PathExists(dirs.PortsDir) {
		return fmt.Errorf("ports directory not found")
	}
	return nil
}

func (c *configureCmd) configureMain(flags *pflag.FlagSet) error {
	if flags.Changed("platform") {
		if err := c.celer.SetPlatform(c.platform); err != nil {
			return color.PrintError(err, "failed to set platform.")
		}
		color.PrintSuccess("current platform: %s", c.platform)
	}

	if flags.Changed("project") {
		if err := c.celer.SetProject(c.project); err != nil {
			return color.PrintError(err, "failed to set project: %s", c.project)
		}
		color.PrintSuccess("current project: %s", c.project)

		// Auto configure platform.
		targetPlatform := c.celer.Project().GetTargetPlatform()
		if targetPlatform != "" && c.celer.Platform().GetName() == "" {
			if err := c.celer.SetPlatform(targetPlatform); err != nil {
				return color.PrintError(err, "failed to set platform: %s", targetPlatform)
			}
			color.PrintSuccess("current platform: %s => Default target platform defined in project", c.celer.Platform().GetName())
		}
	}

	if flags.Changed("build-type") {
		if err := c.celer.SetBuildType(c.buildType); err != nil {
			return color.PrintError(err, "failed to set build type: %s", c.buildType)
		}
		color.PrintSuccess("current build type: %s", c.buildType)
	}

	if flags.Changed("downloads") {
		if err := c.celer.SetDownloads(c.downloads); err != nil {
			return color.PrintError(err, "failed to set downloads: %s", c.downloads)
		}
		color.PrintSuccess("current downloads: %s", c.downloads)
	}

	if flags.Changed("jobs") {
		if err := c.celer.SetJobs(c.jobs); err != nil {
			return color.PrintError(err, "failed to set job num: %d.", c.jobs)
		}
		color.PrintSuccess("current job num: %d.", c.jobs)
	}

	if flags.Changed("offline") {
		if err := c.celer.SetOffline(c.offline); err != nil {
			return color.PrintError(err, "failed to set offline mode: %s", expr.If(c.offline, "true", "false"))
		}
		color.PrintSuccess("current offline mode: %s", expr.If(c.offline, "true", "false"))
	}

	if flags.Changed("verbose") {
		if err := c.celer.SetVerbose(c.verbose); err != nil {
			return color.PrintError(err, "failed to set verbose mode: %s", expr.If(c.verbose, "true", "false"))
		}
		color.PrintSuccess("current verbose mode: %s", expr.If(c.verbose, "true", "false"))
	}

	return nil
}

func (c *configureCmd) configureCCache(flags *pflag.FlagSet) error {
	if flags.Changed("ccache-enabled") {
		if err := c.celer.SetCCacheEnabled(c.ccache.Enabled); err != nil {
			return color.PrintError(err, "failed to update ccache enabled.")
		}
		color.PrintSuccess("current ccache enabled: %s", expr.If(c.ccache.Enabled, "true", "false"))
	}

	if flags.Changed("ccache-dir") {
		if err := c.celer.SetCCacheDir(c.ccache.Dir); err != nil {
			return color.PrintError(err, "failed to update ccache dir.")
		}
		color.PrintSuccess("current ccache dir: %s", c.ccache.Dir)
	}

	if flags.Changed("ccache-maxsize") {
		if err := c.celer.SetCCacheMaxSize(c.ccache.MaxSize); err != nil {
			return color.PrintError(err, "failed to update ccache.maxsize.")
		}
		color.PrintSuccess("current ccache maxsize: %s", c.ccache.MaxSize)
	}

	if flags.Changed("ccache-remote-storage") {
		if err := c.celer.SetCCacheRemoteStorage(c.ccache.RemoteStorage); err != nil {
			return color.PrintError(err, "failed to update ccache.remote_storage.")
		}
		color.PrintSuccess("current ccache remote storage: %s", c.ccache.RemoteStorage)
	}

	if flags.Changed("ccache-remote-only") {
		if err := c.celer.SetCCacheRemoteOnly(c.ccache.RemoteOnly); err != nil {
			return color.PrintError(err, "failed to update ccache.remote_only.")
		}
		color.PrintSuccess("current ccache remote only: %s", expr.If(c.ccache.RemoteOnly, "true", "false"))
	}

	return nil
}

func (c *configureCmd) configureProxy(flags *pflag.FlagSet) error {
	if flags.Changed("proxy-remove") {
		if err := c.celer.RemoveProxy(); err != nil {
			return err
		}
		color.PrintSuccess("http/https proxy is removed")
	} else {
		if flags.Changed("proxy-host") {
			if err := c.celer.SetProxyHost(c.proxy.Host); err != nil {
				return color.PrintError(err, "failed to configure proxy host: %s", c.proxy.Host)
			}
			color.PrintSuccess("current proxy host: %s", c.proxy.Host)
		}

		if flags.Changed("proxy-port") {
			if err := c.celer.SetProxyPort(c.proxy.Port); err != nil {
				return color.PrintError(err, "failed to set proxy port: %d.", c.proxy.Port)
			}
			color.PrintSuccess("current proxy port: %d.", c.proxy.Port)
		}
	}

	return nil
}

func (c *configureCmd) configurePkgCache(flags *pflag.FlagSet) error {
	// fs backend.
	if flags.Changed("pkgcache-fs-dir") {
		if err := c.celer.SetPkgCacheFSDir(c.pkgCacheFS.Dir); err != nil {
			return color.PrintError(err, "failed to set pkgcache.fs dir: %s", c.pkgCacheFS.Dir)
		}
		color.PrintSuccess("current pkgcache.fs dir: %s.", expr.If(c.pkgCacheFS.Dir != "", c.pkgCacheFS.Dir, "empty"))
	}

	// minio backend. Empty values keep the current host/keys, so a single
	// flag can rotate them alone.
	if flags.Changed("pkgcache-minio-host") || flags.Changed("pkgcache-minio-access-key") || flags.Changed("pkgcache-minio-secret-key") {
		if err := c.celer.SetPkgCacheMinio(c.pkgCacheMinio.Host, c.pkgCacheMinio.AccessKey, c.pkgCacheMinio.SecretKey); err != nil {
			return color.PrintError(err, "failed to set pkgcache.minio: %s", c.pkgCacheMinio.Host)
		}
		color.PrintSuccess("current pkgcache.minio host: %s.", expr.If(c.pkgCacheMinio.Host != "", c.pkgCacheMinio.Host, "unchanged"))
	}

	// Options shared by all backends.
	if flags.Changed("pkgcache-writable") {
		if err := c.celer.SetPkgCacheWritable(c.pkgCacheOptions.Writable); err != nil {
			return color.PrintError(err, "failed to set pkgcache writable: %s", expr.If(c.pkgCacheOptions.Writable, "true", "false"))
		}
		color.PrintSuccess("current pkgcache writable: %s.", expr.If(c.pkgCacheOptions.Writable, "true", "false"))
	}
	if flags.Changed("pkgcache-cache-downloads") {
		if err := c.celer.SetPkgCacheCacheDownloads(c.pkgCacheOptions.Downloads); err != nil {
			return color.PrintError(err, "failed to set pkgcache cache-downloads: %s", expr.If(c.pkgCacheOptions.Downloads, "true", "false"))
		}
		color.PrintSuccess("current pkgcache cache-downloads: %s.", expr.If(c.pkgCacheOptions.Downloads, "true", "false"))
	}
	if flags.Changed("pkgcache-cache-artifacts") {
		if err := c.celer.SetPkgCacheCacheArtifacts(c.pkgCacheOptions.Artifacts); err != nil {
			return color.PrintError(err, "failed to set pkgcache cache-artifacts: %s", expr.If(c.pkgCacheOptions.Artifacts, "true", "false"))
		}
		color.PrintSuccess("current pkgcache cache-artifacts: %s.", expr.If(c.pkgCacheOptions.Artifacts, "true", "false"))
	}
	if flags.Changed("pkgcache-cache-repos") {
		if err := c.celer.SetPkgCacheCacheRepos(c.pkgCacheOptions.Repos); err != nil {
			return color.PrintError(err, "failed to set pkgcache cache-repos: %s", expr.If(c.pkgCacheOptions.Repos, "true", "false"))
		}
		color.PrintSuccess("current pkgcache cache-repos: %s.", expr.If(c.pkgCacheOptions.Repos, "true", "false"))
	}

	return nil
}

func (c *configureCmd) configurePort(flags *pflag.FlagSet) error {
	if !flags.Changed("port") {
		return nil
	}

	if c.port == "" {
		return fmt.Errorf("--port is required when using --port-url or --port-ref")
	}
	if !flags.Changed("port-url") && !flags.Changed("port-ref") {
		return fmt.Errorf("at least one of --port-url or --port-ref is required")
	}

	// Init port and update it with url and ref.
	var port configs.Port
	if err := port.Init(c.celer, c.port); err != nil {
		return err
	}
	portPath, err := port.Update(c.portUrl, c.portRef)
	if err != nil {
		return err
	}

	// Print configure result.
	var builder strings.Builder
	fmt.Fprintf(&builder, "%s is updated with", portPath)
	if flags.Changed("port-url") {
		fmt.Fprintf(&builder, ` "url = %s"`, c.portUrl)
	}
	if flags.Changed("port-ref") {
		if flags.Changed("port-url") {
			fmt.Fprintf(&builder, ` and "ref = %s"`, c.portRef)
		} else {
			fmt.Fprintf(&builder, ` "ref = %s"`, c.portRef)
		}
	}
	color.PrintSuccess("%s", builder.String())

	return nil
}

func (c *configureCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	commands := []string{
		"--platform",
		"--project",
		"--build-type",
		"--downloads",
		"--jobs",
		"--offline",
		"--verbose",
		"--pkgcache-fs-dir",
		"--pkgcache-minio-host",
		"--pkgcache-minio-access-key",
		"--pkgcache-minio-secret-key",
		"--pkgcache-writable",
		"--pkgcache-cache-downloads",
		"--pkgcache-cache-artifacts",
		"--pkgcache-cache-repos",
		"--proxy-host",
		"--proxy-port",
		"--proxy-remove",
		"--ccache-enabled",
		"--ccache-dir",
		"--ccache-maxsize",
		"--ccache-remote-storage",
		"--ccache-remote-only",
		"--port",
		"--port-url",
		"--port-ref",
	}

	var suggestions []string
	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
