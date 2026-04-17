package context

import "strings"

type Platform interface {
	Init(platformName string) error
	GetName() string
	GetHostName() string
	GetToolchain() Toolchain
	GetRootFS() RootFS
	GetArchiveChecksums() (toolchainChecksum, rootfsChecksum string, err error)
	Setup() error
}

type Toolchain interface {
	GetName() string
	GetAbsDir() string
	GetRootDir() string
	GetVersion() string
	GetHost() string
	GetSystemName() string
	GetSystemProcessor() string
	GetCrosstoolPrefix() string
	GetCStandard() string
	GetCXXStandard() string
	GetCC() string
	GetCXX() string
	GetCFlags() []string
	GetCXXFlags() []string
	GetLinkFlags() []string
	GetCPP() string
	GetLD() string
	GetGCOV() string
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
	SetEnvs(rootfs RootFS, buildsystem string)
	ClearEnvs()
}

type RootFS interface {
	Validate() error
	CheckAndRepair() error
	Generate(toolchain *strings.Builder) error
	GetPkgConfigPath() []string
	GetIncludeDirs() []string
	GetLibDirs() []string
	GetAbsDir() string
}

type WindowsKit interface {
	Detect(msvc *MSVC) error
}

type MSVC struct {
	VCVars   string
	Includes []string
	Libs     []string
	MT       string
	RC       string
}
