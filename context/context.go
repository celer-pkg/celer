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
	BinaryCache() BinaryCache
	ProxyHostPort() (host string, port int)
	Optimize(buildsystem, toolchain string) *Optimize
	CCacheEnabled() bool
	GenerateToolchainFile() error
}

type BinaryCache interface {
	GetDir() string
	Read(nameVersion, hash, destDir string) (bool, error)
	Write(packageDir, meta string) error
}
