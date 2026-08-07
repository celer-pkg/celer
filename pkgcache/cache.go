package pkgcache

// ========================== CacheContext ========================== //

type CacheContext interface {
	Offline() bool
	PlatformName() string
	ProjectName() string
	BuildType() string
	PkgCacheConfig() PkgCacheConfig
}

// ========================== pkg-cache types ========================== //

// PkgCacheDirType identifies a cache subdirectory.
type PkgCacheDirType uint8

const (
	PkgCacheDirRoot PkgCacheDirType = iota
	PkgCacheDirRepos
	PkgCacheDirArtifacts
	PkgCacheDirDownloads
)

// PkgCacheConfig is the shared (typically NFS) package cache: stores/restores
// source repos and built artifacts so repeat builds skip clone and compile.
type PkgCacheConfig interface {
	GetDir(dirType PkgCacheDirType) string
	IsWritable() bool
	GetCacheArtifacts() bool
	GetCacheDownloads() bool
	GetArtifactCache() AritifactCache
	GetRepoCache() RepoCache
}

// AritifactCache stores/restores a port's built package, keyed by name@version + build hash.
type AritifactCache interface {
	Restore(nameVersion, buildhash, packageDir string) (string, error)
	Store(packageDir, metadata string) error
}

// RepoCache stores/restores a port's source tree, keyed by name@version + checksum.
type RepoCache interface {
	Restore(nameVersion, repoUrl, repoDir, checksum string) (string, error)
	Store(nameVersion, repoUrl, repoDir, archiveFile string) (string, error)
}

// ========================== local cache ========================== //

// LocalCacheConfig is the config for local cache (under the user's home dir)
// for reusing built artifacts across workspaces.
type LocalCacheConfig interface {
	GetDir() string
	GetDevArtifactCache() LocalAritifactCache
}

// LocalAritifactCache stores/restores a port's built package under user's home dir.
type LocalAritifactCache interface {
	Restore(nameVersion, buildhash, packageDir string) (string, error)
	Store(packageDir, metadata string) error
}
