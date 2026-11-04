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
	ReadOnly          bool            `toml:"readonly,omitempty" json:"readonly,omitempty"`
	KeepArtifactCount *int            `toml:"keep_artifact_count,omitempty" json:"keep_artifact_count,omitempty"`
	Minio             *pkgcache.Minio `toml:"minio,omitempty" json:"minio,omitempty"`
	FS                *pkgcache.FS    `toml:"fs,omitempty" json:"fs,omitempty"`

	// Internal field.
	clientCache      pkgcache.ClientCache
	artifactCache    pkgcache.AritifactCache
	repoCache        pkgcache.RepoCache
	downloadCache    pkgcache.DownloadCache
	pythonWheelCache pkgcache.PythonWheelCache
}

// NewPkgCache returns an empty pkgcache config: readonly defaults to false and
// keep_artifact_count stays unset, so artifact pruning is disabled until a node
// opts in explicitly.
func NewPkgCache() *PkgCache { return &PkgCache{} }

func (p PkgCache) GetMinio() *pkgcache.Minio { return p.Minio }
func (p PkgCache) GetFS() *pkgcache.FS       { return p.FS }

func (p PkgCache) IsReadOnly() bool {
	return p.ReadOnly
}

// GetKeepArtifactCount returns how many of the newest artifacts to keep per port,
// or 0 when pruning is disabled. Pruning deletes from a cache other nodes share,
// so it only runs with a count this node explicitly configured.
func (p PkgCache) GetKeepArtifactCount() int {
	if p.KeepArtifactCount == nil || *p.KeepArtifactCount <= 0 {
		return 0
	}
	return *p.KeepArtifactCount
}

func (p PkgCache) GetClientCache() pkgcache.ClientCache {
	if p.clientCache == nil {
		return nil
	}
	return p.clientCache
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

func (p PkgCache) GetPythonWheelCache() pkgcache.PythonWheelCache {
	if p.pythonWheelCache == nil {
		return nil
	}
	return p.pythonWheelCache
}

// ================= DevCache ================= //

type DevCache struct {
	ctx              context.Context
	devArtifactCache *dev.DevArtifactCache
}

func NewDevCache(ctx context.Context) *DevCache {
	cacheConfig := DevCache{ctx: ctx}
	cacheDir := cacheConfig.GetDir()
	cacheConfig.devArtifactCache = dev.NewDevArtifactCache(cacheDir)
	return &cacheConfig
}

func (d DevCache) GetDir() string {
	homeDir, err := os.UserHomeDir()
	if err != nil {
		panic("cannot get user home dir: " + err.Error())
	}

	hostName := d.ctx.Platform().GetHostName()
	return filepath.Join(homeDir, "celer", hostName+"-dev")
}

func (d DevCache) GetDevArtifactCache() pkgcache.DevAritifactCache {
	return d.devArtifactCache
}
