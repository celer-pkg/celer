package buildsystems

import (
	"celer/context"
	"celer/pkgs/expr"
	"runtime"
)

type nativeToolchain struct {
	msvc *context.MSVC
}

// Basic information.
func (n nativeToolchain) GetName() string {
	switch runtime.GOOS {
	case "windows":
		return "msvc"
	default:
		return "gcc"
	}
}

func (n nativeToolchain) GetSystemName() string { return expr.UpperFirst(runtime.GOOS) }
func (n nativeToolchain) GetSystemProcessor() string {
	switch runtime.GOARCH {
	case "amd64":
		return "x86_64"
	case "arm64":
		return "aarch64"
	default:
		panic("unsupported arch: " + runtime.GOARCH)
	}
}
func (n nativeToolchain) GetPath() string            { return "" }
func (n nativeToolchain) GetFullPath() string        { return "" }
func (n nativeToolchain) GetVersion() string         { return "" }
func (n nativeToolchain) GetHost() string            { return "" }
func (n nativeToolchain) GetCrosstoolPrefix() string { return "" }

// C/C++ standard.
func (n nativeToolchain) GetCStandard() string   { return "" }
func (n nativeToolchain) GetCXXStandard() string { return "" }

// Core compiler tools.
func (n nativeToolchain) GetCC() string {
	return expr.If(runtime.GOOS == "windows", "cl", "gcc")
}

func (n nativeToolchain) GetCXX() string {
	return expr.If(runtime.GOOS == "windows", "cl", "g++")
}

func (n nativeToolchain) GetCPP() string {
	return expr.If(runtime.GOOS == "windows", "cl", "g++")
}

func (n nativeToolchain) GetAR() string {
	return expr.If(runtime.GOOS == "windows", "lib", "ar")
}

func (n nativeToolchain) GetLD() string {
	return expr.If(runtime.GOOS == "windows", "link", "ld")
}

func (n nativeToolchain) GetAS() string {
	return expr.If(runtime.GOOS == "windows", "ml", "as")
}

// Object file manipulation tools.
func (n nativeToolchain) GetOBJCOPY() string { return "" }
func (n nativeToolchain) GetOBJDUMP() string { return "" }
func (n nativeToolchain) GetSTRIP() string   { return "" }
func (n nativeToolchain) GetREADELF() string { return "" }

// Symbol and archive tools.
func (n nativeToolchain) GetNM() string     { return "" }
func (n nativeToolchain) GetRANLIB() string { return "" }

// Code coverage tools.
func (n nativeToolchain) GetGCOV() string { return "" }

// Additional compiler tools.
func (n nativeToolchain) GetFC() string { return "" }

// MSVC support.
func (n nativeToolchain) GetMSVC() *context.MSVC { return n.msvc }

// Environment management.
func (t nativeToolchain) SetEnvs(rootfs context.RootFS, buildsystem string) {}
func (t nativeToolchain) ClearEnvs()                                        {}
