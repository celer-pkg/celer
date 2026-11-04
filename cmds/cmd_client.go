package cmds

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgcache/filesystem"
	miniocache "github.com/celer-pkg/celer/pkgcache/minio"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"

	"github.com/spf13/cobra"
)

type clientCmd struct {
	celer *configs.Celer

	fsDir   string
	minio   pkgcache.Minio
	version string
	dest    string
}

func (c *clientCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := cobra.Command{
		Use:   "client",
		Short: "List or download celer clients from pkgcache.",
		Long: `List the celer clients stored in pkgcache.

The cache is read from CELER_PKGCACHE_FS_DIR or the CELER_PKGCACHE_MINIO_* variables,
falling back to the workspace's celer.toml when they are not set. That makes the
command usable before 'celer init', when celer.toml has no pkgcache yet.`,
	}

	listCommand := cobra.Command{
		Use:   "list",
		Short: "List the celer versions available in pkgcache, newest first.",
		Args:  cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			return c.list(cmd)
		},
		ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
			return nil, cobra.ShellCompDirectiveNoFileComp
		},
	}
	listCommand.SilenceErrors = true
	listCommand.SilenceUsage = true

	downloadCommand := cobra.Command{
		Use:   "download",
		Short: "Download a celer version from pkgcache.",
		Args:  cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			return c.fetch()
		},
		ValidArgsFunction: c.completion,
	}
	downloadCommand.SilenceErrors = true
	downloadCommand.SilenceUsage = true
	downloadCommand.Flags().StringVar(&c.version, "version", "", "The celer version to download [mandatory].")
	downloadCommand.Flags().StringVar(&c.dest, "dest", "", "The file path to write the binary to [mandatory].")
	downloadCommand.MarkFlagRequired("version")
	downloadCommand.MarkFlagRequired("dest")

	command.AddCommand(&listCommand, &downloadCommand)

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true

	return &command
}

func (c *clientCmd) list(cmd *cobra.Command) error {
	cache, err := c.clientCache()
	if err != nil {
		return logger.PrintError(err, "failed to open the pkgcache.")
	}

	versions, err := cache.List()
	if err != nil {
		return logger.PrintError(err, "failed to list celer binaries.")
	}

	// Only versions on stdout: a job captures this output to fill a choice list.
	for _, version := range versions {
		fmt.Fprintln(cmd.OutOrStdout(), version)
	}
	return nil
}

func (c *clientCmd) fetch() error {
	if err := checkClientVersion(c.version); err != nil {
		return logger.PrintError(err, "invalid --version.")
	}
	if strings.TrimSpace(c.dest) == "" {
		return logger.PrintError(fmt.Errorf("empty --dest"), "invalid destination.")
	}

	clientCache, err := c.clientCache()
	if err != nil {
		return logger.PrintError(err, "failed to open the pkgcache.")
	}

	if err := clientCache.Fetch(c.version, c.dest); err != nil {
		return logger.PrintError(err, "failed to download celer %s.", c.version)
	}

	logger.PrintSuccess("downloaded celer %s to '%s'.", c.version, c.dest)
	return nil
}

func (c *clientCmd) clientCache() (pkgcache.ClientCache, error) {
	// Environment first: it is the only source before `celer configure --pkgcache-*`.
	cache, err := c.clientCacheFromEnv()
	if err != nil || cache != nil {
		return cache, err
	}

	configPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	if !fileio.PathExists(configPath) {
		return nil, fmt.Errorf("no pkgcache in the environment and no '%s': set CELER_PKGCACHE_FS_DIR or CELER_PKGCACHE_MINIO_*, or run 'celer init' and 'celer configure --pkgcache-*' first", configPath)
	}

	if err := c.celer.Init(); err != nil {
		return nil, err
	}

	pkgCache := c.celer.PkgCache()
	if pkgCache == nil || pkgCache.GetClientCache() == nil {
		return nil, fmt.Errorf("no pkgcache in the environment or in '%s'", configPath)
	}

	return pkgCache.GetClientCache(), nil
}

// clientCacheFromEnv builds a client cache from the pkgcache environment.
func (c *clientCmd) clientCacheFromEnv() (pkgcache.ClientCache, error) {
	c.fsDir = os.Getenv("CELER_PKGCACHE_FS_DIR")
	c.minio = pkgcache.Minio{
		Host:      os.Getenv("CELER_PKGCACHE_MINIO_HOST"),
		AccessKey: os.Getenv("CELER_PKGCACHE_MINIO_ACCESS_KEY"),
		SecretKey: os.Getenv("CELER_PKGCACHE_MINIO_SECRET_KEY"),
	}

	if c.minio.Host != "" || c.minio.AccessKey != "" || c.minio.SecretKey != "" {
		if err := c.minio.Validate(); err != nil {
			return nil, err
		}

		client, err := miniocache.NewClient(&c.minio, false)
		if err != nil {
			return nil, err
		}

		return miniocache.NewClientCacheFrom(c.celer, client, &c.minio, false), nil
	}

	if c.fsDir != "" {
		fs := &pkgcache.FS{Dir: c.fsDir}
		if err := fs.Validate(); err != nil {
			return nil, err
		}

		return filesystem.NewClientCacheFrom(c.celer, fs, false), nil
	}

	return nil, nil
}

func checkClientVersion(version string) error {
	if strings.TrimSpace(version) == "" {
		return fmt.Errorf("no celer version given")
	}
	if strings.ContainsAny(version, `/\`) || strings.Contains(version, "..") {
		return fmt.Errorf("invalid celer version '%s'", version)
	}

	return nil
}

// completion suggests the flags of 'celer client download', which takes no positional
// args: nothing else could be completed here.
func (c *clientCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--version", "--dest"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
