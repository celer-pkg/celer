package context

import (
	"celer/buildsystems"
	"celer/pkgs/proxy"
)

type Context interface {
	Proxy() *proxy.Proxy
	Version() string
	Platform() Platform
	Project() Project
	BuildType() string
	Toolchain() Toolchain
	WindowsKit() WindowsKit
	RootFS() RootFS
	Jobs() int
	Offline() bool
	CacheDir() CacheDir
	Verbose() bool
	Optimize(buildsystem, toolchain string) *buildsystems.Optimize
	GenerateToolchainFile() error
}
