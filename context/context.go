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
}

type CacheDir interface {
	Validate() error
	GetDir() string
	Read(platformName, projectName, buildType, nameVersion, hash, destDir string) (bool, error)
	Write(packageDir, meta string) error
	Remove(platformName, projectName, buildType, nameVersion string) error
	Exist(platformName, projectName, buildType, nameVersion, hash string) bool
}
