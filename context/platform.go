package context

import "strings"

type Platform interface {
	Init(platformName string) error
	GetName() string
	GetHostName() string
	GetToolchain() Toolchain
	Write(platformPath string) error
}

type Toolchain interface {
	GetName() string
	GetPath() string
	GetFullPath() string
	GetVersion() string
	GetHost() string
	GetSystemName() string
	GetSystemProcessor() string
	GetCrosstoolPrefix() string
	GetCStandard() string
	GetCXXStandard() string
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
	Generate(toolchain *strings.Builder, hostName string) error
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

type Optimize struct {
	Debug          string `toml:"debug"`
	Release        string `toml:"release"`
	RelWithDebInfo string `toml:"relwithdebinfo"`
	MinSizeRel     string `toml:"minsizerel"`
}

type WindowsKit interface {
	Detect(msvc *MSVC) error
}

type MSVC struct {
	VCVars      string
	KitIncludes []string
	KitLibs     []string
	MT          string
	RC          string
}
