package buildsystems

import (
	"celer/context"
	"celer/pkgs/expr"
	"runtime"
)

type nativeToolchain struct {
	msvc *context.MSVC
}

func (n nativeToolchain) GetName() string {
	switch runtime.GOOS {
	case "windows":
		return "msvc"
	default:
		return "gcc"
	}
}
func (n nativeToolchain) GetCC() string {
	switch runtime.GOOS {
	case "windows":
		return "cl"
	default:
		return "gcc"
	}
}
func (n nativeToolchain) GetCXX() string {
	switch runtime.GOOS {
	case "windows":
		return "cl"
	default:
		return "g++"
	}
}
func (n nativeToolchain) GetLD() string {
	switch runtime.GOOS {
	case "windows":
		return "link"
	default:
		return "ld"
	}
}
func (n nativeToolchain) GetAR() string {
	switch runtime.GOOS {
	case "windows":
		return "lib"
	default:
		return "ar"
	}
}
func (n nativeToolchain) GetSystemName() string                             { return expr.UpperFirst(runtime.GOOS) }
func (n nativeToolchain) GetSystemProcessor() string                        { return "x86_64" }
func (n nativeToolchain) GetPath() string                                   { return "" }
func (n nativeToolchain) GetFullPath() string                               { return "" }
func (n nativeToolchain) GetVersion() string                                { return "" }
func (n nativeToolchain) GetHost() string                                   { return "" }
func (n nativeToolchain) GetCrosstoolPrefix() string                        { return "" }
func (n nativeToolchain) GetCStandard() string                              { return "" }
func (n nativeToolchain) GetCXXStandard() string                            { return "" }
func (n nativeToolchain) GetAS() string                                     { return "" }
func (n nativeToolchain) GetFC() string                                     { return "" }
func (n nativeToolchain) GetRANLIB() string                                 { return "" }
func (n nativeToolchain) GetNM() string                                     { return "" }
func (n nativeToolchain) GetOBJCOPY() string                                { return "" }
func (n nativeToolchain) GetOBJDUMP() string                                { return "" }
func (n nativeToolchain) GetSTRIP() string                                  { return "" }
func (n nativeToolchain) GetREADELF() string                                { return "" }
func (n nativeToolchain) GetMSVC() *context.MSVC                            { return n.msvc }
func (t nativeToolchain) SetEnvs(rootfs context.RootFS, buildsystem string) {}
func (t nativeToolchain) ClearEnvs()                                        {}
