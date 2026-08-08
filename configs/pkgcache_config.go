package configs

import (
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgcache/dev"
)

// ================= PkgCacheConfig ================= //

type PkgCacheConfig struct {
	Dir            string `toml:"dir"`
	Writable       bool   `toml:"writable"`
	CacheArtifacts bool   `toml:"cache_artifacts"`
	CacheDownloads bool   `toml:"cache_downloads"`

	// Internal field.
	artifactCache pkgcache.AritifactCache
	repoCache     pkgcache.RepoCache
}

func NewPkgCacheConfig(repoCache pkgcache.RepoCache, artifactCache pkgcache.AritifactCache) *PkgCacheConfig {
	return &PkgCacheConfig{
		repoCache:      repoCache,
		artifactCache:  artifactCache,
		CacheArtifacts: true,
		CacheDownloads: true,
		Writable:       true,
	}
}

func (p PkgCacheConfig) GetDir(dirType pkgcache.PkgCacheDirType) string {
	switch dirType {
	case pkgcache.PkgCacheDirArtifacts:
		return filepath.Join(p.Dir, "artifacts-"+Version)

	case pkgcache.PkgCacheDirRepos:
		return filepath.Join(p.Dir, "repos")

	case pkgcache.PkgCacheDirDownloads:
		return filepath.Join(p.Dir, "downloads")

	default:
		return p.Dir
	}
}

func (p PkgCacheConfig) IsWritable() bool {
	return p.Writable
}

func (p PkgCacheConfig) GetCacheArtifacts() bool {
	return p.CacheArtifacts
}

func (p PkgCacheConfig) GetCacheDownloads() bool {
	return p.CacheDownloads
}

func (p PkgCacheConfig) GetArtifactCache() pkgcache.AritifactCache {
	if p.artifactCache == nil {
		return nil
	}
	return p.artifactCache
}

func (p PkgCacheConfig) GetRepoCache() pkgcache.RepoCache {
	if p.repoCache == nil {
		return nil
	}
	return p.repoCache
}

// ================= DevCacheConfig ================= //

type DevCacheConfig struct {
	ctx              context.Context
	devArtifactCache *dev.DevArtifactCache
}

func NewDevCacheConfig(ctx context.Context) *DevCacheConfig {
	cacheConfig := DevCacheConfig{ctx: ctx}
	cacheDir := cacheConfig.GetDir()
	cacheConfig.devArtifactCache = dev.NewDevArtifactCache(ctx, cacheDir)
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
