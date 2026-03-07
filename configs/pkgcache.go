package configs

import (
	"celer/context"
	"celer/pkgcache"
	"celer/pkgs/fileio"
	"fmt"
)

type pkgCache struct {
	Dir            string `toml:"dir"`
	Writable       bool   `toml:"writable"`
	CacheArtifiact bool   `toml:"cache_artifiact"`
	CacheRepo      bool   `toml:"cache_repo"`

	// Internal field.
	ctx           context.Context
	artifactCache *pkgcache.Aritifact
	repoCache     *pkgcache.Repo
}

func NewPkgCache(ctx context.Context, dir string, writable bool) *pkgCache {
	return &pkgCache{
		ctx:      ctx,
		Dir:      dir,
		Writable: writable,
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
	p.artifactCache = pkgcache.NewArtifactCacheDir(p.ctx, p.Dir, p.Writable)
	p.repoCache = pkgcache.NewRepo(p.Dir, p.Writable)
	return nil
}

func (p pkgCache) GetDir() string {
	return p.Dir
}

func (p pkgCache) IsWritable() bool {
	return p.Writable
}

func (p pkgCache) GetArtifactCache() context.AritifactCache {
	if p.artifactCache == nil {
		return nil
	}
	return p.artifactCache
}

func (p pkgCache) GetRepoCache() context.RepoCache {
	if p.repoCache == nil {
		return nil
	}
	return p.repoCache
}
