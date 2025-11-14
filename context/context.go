package context

type Context interface {
	Version() string
	Platform() Platform
	Project() Project
	BuildType() string
	RootFS() RootFS
	Jobs() int
	Offline() bool
	Verbose() bool
	CacheDir() CacheDir
	Proxy() (host string, port int)
	Optimize(buildsystem, toolchain string) *Optimize
	GenerateToolchainFile() error
	NewPort(devDep bool) Port
}

type CacheDir interface {
	GetDir() string
	Read(nameVersion, hash, destDir string) (bool, error)
	Write(packageDir, meta string) error
}
