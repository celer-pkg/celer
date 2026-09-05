package context

import (
	"context"

	"github.com/celer-pkg/celer/pkgcache"
)

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
	PkgCache() pkgcache.PkgCache
	DevCache() pkgcache.DevCache
	ProxyHostPort() (host string, port int)
	CCacheEnabled() bool
	GenerateToolchainFile() error
	ExprVars() *ExprVars
	PythonConfig() PythonConfig
	Features() Features
}

// PythonConfig exposes the Python interpreter setup for building python ports.
type PythonConfig interface {
	GetVersion() string
	GetIndexUrl() string
	GetExtraIndexUrls() []string
	GetTrustedHosts() []string
}

// Features features during development, can be configure temportary.
type Features interface {
	ShouldIgnoreCheckCMakeAbsPath() bool
}

// Background pointing to context.Background()
func Background() context.Context {
	return context.Background()
}
