package configs

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type pkgCache struct {
	Dir               string `toml:"dir"`
	Writable          bool   `toml:"writable"`
	CacheArtifacts    bool   `toml:"cache_artifacts"`
	CacheThirdParties bool   `toml:"cache_third_parties"`
	CacheDownloads    bool   `toml:"cache_downloads"`

	// Internal field.
	ctx            context.Context
	artifactConfig *pkgcache.ArtifactConfig
	repoConfig     *pkgcache.RepoConfig
}

func NewPkgCache(ctx context.Context, dir string, writable bool) *pkgCache {
	return &pkgCache{
		ctx:               ctx,
		Dir:               dir,
		Writable:          writable,
		CacheArtifacts:    true,
		CacheThirdParties: false,
		CacheDownloads:    true,
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
	p.artifactConfig = pkgcache.NewArtifactConfig(p.ctx, p.Dir, p.Writable)
	p.repoConfig = pkgcache.NewRepoConfig(p.ctx, p.Dir, p.Writable)
	return nil
}

func (p pkgCache) GetDir(dirType context.PkgCacheDirType) string {
	switch dirType {
	case context.PkgCacheDirArtifacts:
		return filepath.Join(p.Dir, "artifacts")

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

func (p *pkgCache) ShouldCacheRepo(nameVersion string) bool {
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		panic("invalid nameVersion: " + nameVersion)
	}

	// Only cache third-party repos that exists in ports dir.
	portName := parts[0]
	groupName := strings.ToLower(string([]rune(portName)[0]))
	portPath := filepath.Join(dirs.PortsDir, groupName, portName)
	return p.CacheThirdParties && fileio.PathExists(portPath)
}
