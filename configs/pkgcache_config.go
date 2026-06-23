package configs

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// ================= PkgCacheConfig ================= //

type PkgCacheConfig struct {
	Dir            string `toml:"dir"`
	Writable       bool   `toml:"writable"`
	CacheArtifacts bool   `toml:"cache_artifacts"`
	CacheDownloads bool   `toml:"cache_downloads"`

	// Internal field.
	ctx            context.Context
	artifactConfig *pkgcache.ArtifactConfig
	repoConfig     *pkgcache.RepoConfig
}

func NewPkgCacheConfig(ctx context.Context, dir string, writable bool) *PkgCacheConfig {
	return &PkgCacheConfig{
		ctx:            ctx,
		Dir:            dir,
		Writable:       writable,
		CacheArtifacts: true,
		CacheDownloads: true,
	}
}

func (p *PkgCacheConfig) Refresh() error {
	if p.Dir == "" {
		return fmt.Errorf("pkgcache dir is empty")
	}
	if !fileio.PathExists(p.Dir) {
		return fmt.Errorf("pkgcache dir does not exist: %s", p.Dir)
	}

	// Create valid artifact config and repo config.
	p.artifactConfig = pkgcache.NewArtifactConfig(p.ctx, p.Writable)
	p.repoConfig = pkgcache.NewRepoConfig(p.ctx, p.Writable)

	return nil
}

func (p PkgCacheConfig) GetDir(dirType context.PkgCacheDirType) string {
	switch dirType {
	case context.PkgCacheDirArtifacts:
		return filepath.Join(p.Dir, "artifacts-"+Version)

	case context.PkgCacheDirRepos:
		return filepath.Join(p.Dir, "repos")

	case context.PkgCacheDirDownloads:
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

func (p PkgCacheConfig) GetArtifactCache() context.AritifactCache {
	if p.artifactConfig == nil {
		return nil
	}
	return p.artifactConfig
}

func (p PkgCacheConfig) GetRepoCache() context.RepoCache {
	if p.repoConfig == nil {
		return nil
	}
	return p.repoConfig
}

// ================= DevCacheConfig ================= //

type DevCacheConfig struct {
	ctx              context.Context
	devArtifactCache *pkgcache.DevArtifactCache
}

func NewDevCacheConfig(ctx context.Context) *DevCacheConfig {
	cacheConfig := DevCacheConfig{ctx: ctx}
	cacheDir := cacheConfig.GetDir()
	cacheConfig.devArtifactCache = pkgcache.NewDevArtifactCache(ctx, cacheDir)
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

func (d DevCacheConfig) GetDevArtifactCache() context.DevAritifactCache {
	return d.devArtifactCache
}
