package context

import "strings"

type Platform interface {
	Init(platformName string) error
	GetName() string
	GetHostName() string
	GetToolchain() Toolchain
	GetWindowsKit() WindowsKit
	Write(platformPath string) error
	Setup() error
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
