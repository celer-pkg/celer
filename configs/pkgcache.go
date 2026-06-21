package configs

import (
	"fmt"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type PkgCache struct {
	Dir            string `toml:"dir"`
	Writable       bool   `toml:"writable"`
	CacheArtifacts bool   `toml:"cache_artifacts"`
	CacheDownloads bool   `toml:"cache_downloads"`

	// Internal field.
	ctx            context.Context
	artifactConfig *pkgcache.ArtifactConfig
	repoConfig     *pkgcache.RepoConfig
}

func NewPkgCache(ctx context.Context, dir string, writable bool) *PkgCache {
	return &PkgCache{
		ctx:            ctx,
		Dir:            dir,
		Writable:       writable,
		CacheArtifacts: true,
		CacheDownloads: true,
	}
}

func (p *PkgCache) Refresh() error {
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

func (p PkgCache) GetDir(dirType context.PkgCacheDirType) string {
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

func (p PkgCache) IsWritable() bool {
	return p.Writable
}

func (p PkgCache) GetArtifactCache() context.AritifactCache {
	if p.artifactConfig == nil {
		return nil
	}
	return p.artifactConfig
}

func (p PkgCache) GetRepoCache() context.RepoCache {
	if p.repoConfig == nil {
		return nil
	}
	return p.repoConfig
}
