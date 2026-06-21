package context

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
	PkgCache() PkgCache
	ProxyHostPort() (host string, port int)
	CCacheEnabled() bool
	GenerateToolchainFile() error
	ExprVars() *ExprVars
	PythonConfig() PythonConfig
	Experiment() Experiment
}

type PkgCacheDirType uint8

const (
	PkgCacheDirRoot PkgCacheDirType = iota
	PkgCacheDirRepos
	PkgCacheDirArtifacts
	PkgCacheDirDownloads
)

type PkgCache interface {
	GetDir(dirType PkgCacheDirType) string
	IsWritable() bool
	GetCacheArtifacts() bool
	GetCacheDownloads() bool
	GetArtifactCache() AritifactCache
	GetRepoCache() RepoCache
}

type AritifactCache interface {
	Restore(nameVersion, buildhash, packageDir string) (string, error)
	Store(packageDir, metadata string) error
}

type RepoCache interface {
	Restore(nameVersion, repoUrl, repoDir, checksum string) (string, error)
	Store(nameVersion, repoUrl, repoDir, archiveFile string) (string, error)
}

type PythonConfig interface {
	GetVersion() string
	GetIndexUrl() string
	GetExtraIndexUrls() []string
	GetTrustedHosts() []string
}

type Experiment interface {
	GetCheckCMakeAbolutePath() bool
}
