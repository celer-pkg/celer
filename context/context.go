package context

type Context interface {
	Version() string
	Platform() Platform
	RootFS() RootFS
	Project() Project
	BuildType() string
	Downloads() string
	Jobs() int
	Offline() bool
	Verbose() bool
	InstalledDir() string
	InstalledDevDir() string
	PkgCache() PkgCache
	ProxyHostPort() (host string, port int)
	Optimize(buildsystem, toolchain string) *Optimize
	CCacheEnabled() bool
	GenerateToolchainFile() error
	ExprVars() *ExprVars
}

type PkgCache interface {
	GetDir() string
	IsWritable() bool
	GetArtifactCache() AritifactCache
	GetRepoCache() RepoCache
}

type AritifactCache interface {
	Fetch(nameVersion, hash, destDir string) (bool, error)
	Store(packageDir, meta string) error
}

type RepoCache interface {
	Fetch(repoUrl, repoDir, commit string) (string, error)
	Store(repoUrl, repoDir string) (string, error)
}
