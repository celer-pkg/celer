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
	Restore(nameVersion, hash, destDir string) (string, error)
	Store(packageDir, meta string) error
}

type RepoCache interface {
	Restore(nameVersion, repoUrl, repoDir, commit string) (string, error)
	Store(nameVersion, repoUrl, repoDir string) (string, error)
}
