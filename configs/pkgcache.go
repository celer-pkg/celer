package configs

import (
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgcache/dev"
)

// ================= PkgCache ================= //

type PkgCache struct {
	Minio   *pkgcache.Minio   `toml:"minio"`
	FS      *pkgcache.FS      `toml:"fs"`
	Options pkgcache.Options  `toml:"options"`

	// Internal field.
	artifactCache pkgcache.AritifactCache
	repoCache     pkgcache.RepoCache
	downloadCache pkgcache.DownloadCache
}

func NewPkgCache() *PkgCache {
	return &PkgCache{}
}

func (p PkgCache) GetMinio() *pkgcache.Minio {
	return p.Minio
}

func (p PkgCache) GetFS() *pkgcache.FS {
	return p.FS
}

// GetOptions returns the options shared by all pkgcache backends.
func (p PkgCache) GetOptions() pkgcache.Options {
	return p.Options
}

func (p PkgCache) GetArtifactCache() pkgcache.AritifactCache {
	if p.artifactCache == nil {
		return nil
	}
	return p.artifactCache
}

func (p PkgCache) GetRepoCache() pkgcache.RepoCache {
	if p.repoCache == nil {
		return nil
	}
	return p.repoCache
}

func (p PkgCache) GetDownloadCache() pkgcache.DownloadCache {
	if p.downloadCache == nil {
		return nil
	}
	return p.downloadCache
}

// ================= DevCacheConfig ================= //

type DevCacheConfig struct {
	ctx              context.Context
	devArtifactCache *dev.DevArtifactCache
}

func NewDevCacheConfig(ctx context.Context) *DevCacheConfig {
	cacheConfig := DevCacheConfig{ctx: ctx}
	cacheDir := cacheConfig.GetDir()
	cacheConfig.devArtifactCache = dev.NewDevArtifactCache(cacheDir)
	return &cacheConfig
}

func (d DevCacheConfig) GetDir() string {
	homeDir, err := os.UserHomeDir()
	if err != nil {
		panic("cannot get user home dir: " + err.Error())
	}

	hostName := d.ctx.Platform().GetHostName()
	return filepath.Join(homeDir, "celer", hostName+"-dev")
}

func (d DevCacheConfig) GetDevArtifactCache() pkgcache.DevAritifactCache {
	return d.devArtifactCache
}
