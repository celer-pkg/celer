package configs

import (
	"fmt"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type pkgCache struct {
	Dir            string `toml:"dir"`
	Writable       bool   `toml:"writable"`
	CacheArtifacts bool   `toml:"cache_artifacts"`
	CacheDownloads bool   `toml:"cache_downloads"`

	// Internal field.
	ctx            context.Context
	artifactConfig *pkgcache.ArtifactConfig
	repoConfig     *pkgcache.RepoConfig
}

func NewPkgCache(ctx context.Context, dir string, writable bool) *pkgCache {
	return &pkgCache{
		ctx:            ctx,
		Dir:            dir,
		Writable:       writable,
		CacheArtifacts: true,
		CacheDownloads: true,
	}
}

func (p *pkgCache) Validate() error {
	if p.Dir == "" {
		return fmt.Errorf("pkgcache dir is empty")
	}
	if !fileio.PathExists(p.Dir) {
		return fmt.Errorf("pkgcache dir does not exist: %s", p.Dir)
	}

	// Create valid aritifact and repo cache.
	artifactCacheDir := p.GetDir(context.PkgCacheDirArtifacts)
	p.artifactConfig = pkgcache.NewArtifactConfig(p.ctx, artifactCacheDir, p.Writable)
	p.repoConfig = pkgcache.NewRepoConfig(p.ctx, p.Dir, p.Writable)
	return nil
}

func (p pkgCache) GetDir(dirType context.PkgCacheDirType) string {
	switch dirType {
	case context.PkgCacheDirArtifacts:
		return filepath.Join(p.Dir, "artifacts", "celer-"+Version)

	case context.PkgCacheDirRepos:
		return filepath.Join(p.Dir, "repos")

	case context.PkgCacheDirDownloads:
		return filepath.Join(p.Dir, "downloads")

	default:
		return p.Dir
	}
}

func (p pkgCache) IsWritable() bool {
	return p.Writable
}

func (p pkgCache) GetArtifactCache() context.AritifactCache {
	if p.artifactConfig == nil {
		return nil
	}
	return p.artifactConfig
}

func (p pkgCache) GetRepoCache() context.RepoCache {
	if p.repoConfig == nil {
		return nil
	}
	return p.repoConfig
}
