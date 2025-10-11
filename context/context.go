package context

import (
	"celer/buildsystems"
	"celer/pkgs/proxy"
	"strings"
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

type Platform interface {
	Init(platformName string) error
	GetName() string
	GetHostName() string
	GetToolchain() Toolchain
	GetWindowsKit() WindowsKit
	Write(platformPath string) error
	Setup() error
}

type Project interface {
	Init(ctx Context, projectName string) error
	GetName() string
	GetPorts() []string
	GetDefaultPlatform() string
	Write(platformPath string) error
	Deploy() error
}

type Toolchain interface {
	Generate(toolchain *strings.Builder, hostName string) error
	GetName() string
	GetPath() string
	GetVersion() string
	GetHost() string
	GetSystemName() string
	GetSystemProcessor() string
	GetCrosstoolPrefix() string
	GetCStandard() string
	GetCXXStandard() string
	GetFullPath() string
	GetCC() string
	GetCXX() string
	GetLD() string
	GetAS() string
	GetFC() string
	GetAR() string
	GetRANLIB() string
	GetNM() string
	GetOBJCOPY() string
	GetOBJDUMP() string
	GetSTRIP() string
	GetREADELF() string
	GetMSVC() *MSVC
}

type RootFS interface {
	Validate() error
	CheckAndRepair() error
	Generate(toolchain *strings.Builder) error
	GetPkgConfigPath() []string
	GetIncludeDirs() []string
	GetLibDirs() []string
	GetFullPath() string
}

type WindowsKit interface {
	Detect(msvc *MSVC) error
}

type MSVC struct {
	VCVars string
	MT     string
	RC     string
}

type CacheDir interface {
	Validate() error
	GetDir() string
	Read(platformName, projectName, buildType, nameVersion, hash, destDir string) (bool, error)
	Write(packageDir, meta string) error
	Remove(platformName, projectName, buildType, nameVersion string) error
	Exist(platformName, projectName, buildType, nameVersion, hash string) bool
}
