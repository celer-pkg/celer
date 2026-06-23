package context

// ========================== context ========================== //
// Context exposes the workspace's global config (platform, project, build
// settings, caches, toolchain) to build components, decoupling them from Celer.
type Context interface {
	Version() string
	Platform() Platform
	RootFS() RootFS
	Project() Project
	BuildType() string
	LibraryFolder() string
	Downloads() string
	Jobs() int
	Offline() bool
	Verbose() bool
	InstalledDir() string
	InstalledDevDir() string
	PkgCacheConfig() PkgCacheConfig
	DevCacheConfig() DevCacheConfig
	ProxyHostPort() (host string, port int)
	CCacheEnabled() bool
	GenerateToolchainFile() error
	ExprVars() *ExprVars
	PythonConfig() PythonConfig
	Experiment() Experiment
}

// PythonConfig exposes the Python interpreter setup for building python ports.
type PythonConfig interface {
	GetVersion() string
	GetIndexUrl() string
	GetExtraIndexUrls() []string
	GetTrustedHosts() []string
}

// Experiment gates opt-in, not-yet-stable features behind a flag.
type Experiment interface {
	GetCheckCMakeAbolutePath() bool
}

// ========================== pkg-cache ========================== //
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

// ========================== dev-cache ========================== //

// DevCacheConfig is a per-developer local cache (under the user's home dir)
// for reusing built artifacts across workspaces.
type DevCacheConfig interface {
	GetDir() string
	GetDevArtifactCache() DevAritifactCache
}

// DevAritifactCache stores/restores a port's built package in the per-developer local cache.
type DevAritifactCache interface {
	Restore(nameVersion, buildhash, packageDir string) (string, error)
	Store(packageDir, metadata string) error
}
