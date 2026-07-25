package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/configs/toolchains"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type Toolchain struct {
	toolchains.Infos
	toolchains.BuildTools
	toolchains.BuildFlags

	// C/C++ standard.
	CStandard   string `toml:"c_standard,omitempty"`
	CXXStandard string `toml:"cxx_standard,omitempty"`

	// Default minimum is "3.5"
	CMakePolicyVersionMinimum string `toml:"cmake_policy_version_minimum,omitempty"`

	// Platform-aware envs and cmake variables.
	Envs      []string `toml:"envs,omitempty"`
	CMakeVars []string `toml:"cmake_vars,omitempty"`

	// Internal fields.
	toolchain toolchains.Toolchain

	ctx         context.Context
	displayName string
	rootDir     string
	abspath     string
}

func (t Toolchain) SetupEnvs() {
	exrVars := t.ctx.ExprVars()
	for _, item := range t.Envs {
		parts := strings.Split(item, "=")
		if len(parts) != 2 {
			continue
		}

		key := parts[0]
		value := parts[1]

		value = exrVars.Expand(value)
		os.Setenv(parts[0], env.JoinSpace(value, os.Getenv(key)))
	}
}

func (t Toolchain) effectiveFlags(buildType string) (cflags, cxxflags, ldflags []string) {
	if strings.EqualFold(buildType, "debug") {
		if len(t.CFlagsDebug) > 0 {
			cflags = t.CFlagsDebug
		} else {
			cflags = t.CFlags
		}
		if len(t.CXXFlagsDebug) > 0 {
			cxxflags = t.CXXFlagsDebug
		} else {
			cxxflags = t.CXXFlags
		}
		if len(t.LDFlagsDebug) > 0 {
			ldflags = t.LDFlagsDebug
		} else {
			ldflags = t.LDFlags
		}
	} else {
		cflags = t.CFlags
		cxxflags = t.CXXFlags
		ldflags = t.LDFlags
	}

	// Merge toolchain builtin flags.
	if t.toolchain != nil {
		cflags = append(cflags, t.toolchain.CFlags()...)
		cxxflags = append(cxxflags, t.toolchain.CXXFlags()...)
		ldflags = append(ldflags, t.toolchain.LDFlags()...)
	}

	return cflags, cxxflags, ldflags
}

func (t Toolchain) generate(toolchain *strings.Builder) error {
	appendFlags := func(key string, flags []string, indent string) {
		for _, item := range flags {
			item = strings.TrimSpace(item)
			if item == "" {
				continue
			}
			if t.ctx != nil {
				if exprVars := t.ctx.ExprVars(); exprVars != nil {
					item = exprVars.Expand(item)
				}
			}
			fmt.Fprintf(toolchain, "%sstring(APPEND %s %q)\n", indent, key, " "+item)
		}
	}

	fmt.Fprintf(toolchain, "\n# ============== Cross-compile target system ============== #\n")
	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_SYSTEM_NAME", t.cmakeSystemName())
	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_SYSTEM_PROCESSOR", t.SystemProcessor)
	if t.SystemVersion != "" {
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_SYSTEM_VERSION", t.SystemVersion)
	}

	// For Android, set CMAKE_ANDROID_NDK so CMake uses the NDK path.
	if strings.EqualFold(t.SystemName, "Android") {
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_ANDROID_NDK", fileio.ToRelPath(t.rootDir))
	}

	fmt.Fprintf(toolchain, "\n# ============== Cross-compile toolchain ============== #\n")
	fmt.Fprintf(toolchain, "set(%s %q)\n", "TOOLCHAIN", t.toolchain.Dir(t.abspath))
	t.toolchain.AssembleBuildTools(toolchain)

	// Some toolchain like clang may link runtime flags.
	rtFlags := t.toolchain.RuntimeFlags()
	if len(rtFlags) > 0 {
		fmt.Fprint(toolchain, "\n# clang cross-compile runtime flags.\n")
		for _, flag := range rtFlags {
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_C_FLAGS_INIT " %s")`+"\n", flag)
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_CXX_FLAGS_INIT " %s")`+"\n", flag)
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " %s")`+"\n", flag)
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " %s")`+"\n", flag)
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " %s")`+"\n", flag)
		}
	}

	// Configure compiler targets are usually required by embed platform.
	if t.CCompilerTarget != "" || t.CXXCompilerTarget != "" {
		fmt.Fprintf(toolchain, "\n# ============== Compiler targets are usually required by embed platform ============== #\n")
		if t.CCompilerTarget != "" {
			fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_C_COMPILER_TARGET", t.CCompilerTarget)
		}
		if t.CXXCompilerTarget != "" {
			fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_CXX_COMPILER_TARGET", t.CXXCompilerTarget)
		}
	}

	// CMake search paths section.
	fmt.Fprintf(toolchain, "\n# Search programs in the host environment.\n")
	fmt.Fprintf(toolchain, "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n")
	fmt.Fprintf(toolchain, "\n# Search libraries and headers in the target environment.\n")
	fmt.Fprintf(toolchain, "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)\n")
	fmt.Fprintf(toolchain, "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)\n")
	fmt.Fprintf(toolchain, "set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)\n")

	// Write C/C++ language standard.
	if t.CStandard != "" || t.CXXStandard != "" {
		fmt.Fprint(toolchain, "\n# C/CXX language standard.\n")

		if t.CStandard != "" {
			fmt.Fprintf(toolchain, "set(%s %s)\n", "CMAKE_C_STANDARD", strings.TrimPrefix(t.CStandard, "c"))
			fmt.Fprintf(toolchain, "set(%s %s)\n", "CMAKE_C_STANDARD_REQUIRED", "ON")
		}

		if t.CXXStandard != "" {
			fmt.Fprintf(toolchain, "set(%s %s)\n", "CMAKE_CXX_STANDARD", strings.TrimPrefix(t.CXXStandard, "c++"))
			fmt.Fprintf(toolchain, "set(%s %s)\n", "CMAKE_CXX_STANDARD_REQUIRED", "ON")
		}
	}

	buildType := t.ctx.BuildType()
	cflags, cxxflags, ldflags := t.effectiveFlags(buildType)
	if len(cflags) > 0 || len(cxxflags) > 0 || len(ldflags) > 0 {
		fmt.Fprint(toolchain, "\n# Setting extra build flags.\n")

		// If both cflags and cxxflags exist, use foreach to avoid duplication
		if len(cflags) > 0 && len(cxxflags) > 0 {
			fmt.Fprint(toolchain, "foreach(flag_var CMAKE_C_FLAGS_INIT CMAKE_CXX_FLAGS_INIT)\n")
			appendFlags("${flag_var}", cflags, "  ")
			fmt.Fprint(toolchain, "endforeach()\n")
		} else {
			appendFlags("CMAKE_C_FLAGS_INIT", cflags, "")
			appendFlags("CMAKE_CXX_FLAGS_INIT", cxxflags, "")
		}

		if len(ldflags) > 0 {
			fmt.Fprint(toolchain, "foreach(flag_var CMAKE_EXE_LINKER_FLAGS_INIT CMAKE_SHARED_LINKER_FLAGS_INIT CMAKE_MODULE_LINKER_FLAGS_INIT)\n")
			appendFlags("${flag_var}", ldflags, "  ")
			fmt.Fprint(toolchain, "endforeach()\n")
		}
	}

	// Set build environments for toolchain if required.
	if len(t.Envs) > 0 {
		fmt.Fprint(toolchain, "\n# Cross-compile environment.\n")
		for _, env := range t.Envs {
			parts := strings.Split(env, "=")
			if len(parts) != 2 {
				return fmt.Errorf("invalid toolchain.env: %s", env)
			}

			envKey := parts[0]
			envValue := t.ctx.ExprVars().Expand(parts[1])
			envValue = fileio.ToRelPath(envValue)
			fmt.Fprintf(toolchain, "set(ENV{%s} %q)\n", envKey, envValue)
		}
	}

	return nil
}

func (t Toolchain) GetName() string {
	return t.Name
}

func (t Toolchain) GetSHA256() string {
	return t.SHA256
}

func (t Toolchain) GetHost() string {
	return t.Host
}

func (t Toolchain) GetVersion() string {
	return t.Version
}

func (t Toolchain) GetSystemName() string {
	return t.SystemName
}

func (t Toolchain) GetSystemVersion() string {
	return t.SystemVersion
}

func (t Toolchain) GetSystemProcessor() string {
	return t.SystemProcessor
}

func (t Toolchain) GetCrosstoolPrefix() string {
	return t.CrosstoolPrefix
}

func (t Toolchain) GetCStandard() string {
	return t.CStandard
}

func (t Toolchain) GetCXXStandard() string {
	return t.CXXStandard
}

func (t Toolchain) GetCC() string {
	return t.CC
}

func (t Toolchain) GetCXX() string {
	return t.CXX
}

func (t Toolchain) GetCFlags() []string {
	return t.CFlags
}

func (t Toolchain) GetCXXFlags() []string {
	return t.CXXFlags
}

func (t Toolchain) GetLDFlags() []string {
	return t.LDFlags
}

func (t Toolchain) GetCMakePolicyVersionMinimum() string {
	return t.CMakePolicyVersionMinimum
}

func (t Toolchain) GetCMakeVars() []string {
	return t.CMakeVars
}

func (t Toolchain) GetCPP() string {
	return t.CPP
}

func (t Toolchain) GetAR() string {
	return t.AR
}

func (t Toolchain) GetLD() string {
	return t.LD
}

func (t Toolchain) GetAS() string {
	return t.AS
}

func (t Toolchain) GetFC() string {
	return t.FC
}

func (t Toolchain) GetRANLIB() string {
	return t.RANLIB
}

func (t Toolchain) GetNM() string {
	return t.NM
}

func (t Toolchain) GetOBJCOPY() string {
	return t.OBJCOPY
}

func (t Toolchain) GetOBJDUMP() string {
	return t.OBJDUMP
}

func (t Toolchain) GetSTRIP() string {
	return t.STRIP
}

func (t Toolchain) GetREADELF() string {
	return t.READELF
}

func (t Toolchain) GetSIZE() string {
	return t.SIZE
}

func (t Toolchain) GetSTRINGS() string {
	return t.STRINGS
}

func (t Toolchain) GetGCOV() string {
	return t.GCOV
}

func (t Toolchain) GetADDR2LINE() string {
	return t.ADDR2LINE
}

func (t Toolchain) GetCXXFILT() string {
	return t.CXXFILT
}
func (t Toolchain) GetMSVC() *context.MSVC {
	return &t.MSVC
}

func (t Toolchain) GetAbsDir() string {
	return t.abspath
}

func (t Toolchain) GetRootDir() string {
	return t.rootDir
}

func (t Toolchain) GetCrosstoolPrefixPath() string {
	return filepath.Join(t.abspath, t.CrosstoolPrefix)
}

func (t Toolchain) cmakeSystemName() string {
	systemName := strings.ToLower(t.SystemName)
	if systemName == "qnx" {
		return "QNX"
	}

	return expr.UpperFirst(t.SystemName)
}

func (t Toolchain) SetEnvs(rootfs context.RootFS, buildsystem string, portEnvs []string) {
	crosstoolPrefix := t.GetCrosstoolPrefix()
	cc := t.GetCC()
	cxx := t.GetCXX()

	// Capture CC/CXX from portEnvs if they are set.
	for _, env := range portEnvs {
		if before, after, ok := strings.Cut(env, "="); ok {
			switch strings.TrimSpace(before) {
			case "CC":
				cc = strings.TrimSpace(after)
			case "CXX":
				cxx = strings.TrimSpace(after)
			}
		}
	}

	// cross tool prefix maybe empty for msvc in windows.
	if crosstoolPrefix != "" {
		os.Setenv("CROSSTOOL_PREFIX", crosstoolPrefix)
	}
	os.Setenv("HOST", t.GetHost())

	// For CMake, compiler tools are defined in toolchain_file.cmake, skip environment variable setup.
	if buildsystem == "cmake" {
		return
	}

	var ccFlags, cxxFlags []string
	if t.ctx.CCacheEnabled() {
		// For Windows + MSVC with Makefiles, don't set ccache in CC/CXX environment variables,
		// because MSYS2 shell cannot handle "ccache cl.exe" as a command.
		if runtime.GOOS == "windows" && (t.GetName() == "msvc" || t.GetName() == "clang-cl") && buildsystem == "makefiles" {
			os.Setenv("CC", cc)
			os.Setenv("CXX", cxx)
		} else {
			ccFlags = append(ccFlags, "ccache", cc)
			cxxFlags = append(cxxFlags, "ccache", cxx)

			if rootfs != nil {
				ccFlags = append(ccFlags, "--sysroot="+rootfs.GetAbsDir())
				cxxFlags = append(cxxFlags, "--sysroot="+rootfs.GetAbsDir())

				// clang cross-compile runtime flags.
				rtFlags := t.RuntimeFlags()
				ccFlags = append(ccFlags, rtFlags...)
				cxxFlags = append(cxxFlags, rtFlags...)
			}
			os.Setenv("CC", strings.Join(ccFlags, " "))
			os.Setenv("CXX", strings.Join(cxxFlags, " "))
		}
	} else {
		ccFlags = append(ccFlags, cc)
		cxxFlags = append(cxxFlags, cxx)

		if rootfs != nil {
			ccFlags = append(ccFlags, "--sysroot="+rootfs.GetAbsDir())
			cxxFlags = append(cxxFlags, "--sysroot="+rootfs.GetAbsDir())

			// clang cross-compile runtime flags.
			rtFlags := t.RuntimeFlags()
			ccFlags = append(ccFlags, rtFlags...)
			cxxFlags = append(cxxFlags, rtFlags...)
		}
		os.Setenv("CC", strings.Join(ccFlags, " "))
		os.Setenv("CXX", strings.Join(cxxFlags, " "))
	}

	if t.GetAS() != "" {
		os.Setenv("AS", t.GetAS())
	}

	if t.GetFC() != "" {
		os.Setenv("FC", t.GetFC())
	}

	if t.GetRANLIB() != "" {
		os.Setenv("RANLIB", t.GetRANLIB())
	}

	if t.GetAR() != "" {
		os.Setenv("AR", t.GetAR())
	}

	if t.GetLD() != "" {
		os.Setenv("LD", t.GetLD())
	}

	if t.GetNM() != "" {
		os.Setenv("NM", t.GetNM())
	}

	if t.GetOBJCOPY() != "" {
		os.Setenv("OBJCOPY", t.GetOBJCOPY())
	}

	if t.GetOBJDUMP() != "" {
		os.Setenv("OBJDUMP", t.GetOBJDUMP())
	}

	if t.GetSTRIP() != "" {
		os.Setenv("STRIP", t.GetSTRIP())
	}

	if t.GetREADELF() != "" {
		os.Setenv("READELF", t.GetREADELF())
	}
}

func (t Toolchain) ClearEnvs() {
	os.Unsetenv("CROSSTOOL_PREFIX")
	os.Unsetenv("SYSROOT")
	os.Unsetenv("HOST")
	os.Unsetenv("CC")
	os.Unsetenv("CXX")
	os.Unsetenv("AS")
	os.Unsetenv("FC")
	os.Unsetenv("RANLIB")
	os.Unsetenv("AR")
	os.Unsetenv("LD")
	os.Unsetenv("NM")
	os.Unsetenv("OBJCOPY")
	os.Unsetenv("OBJDUMP")
	os.Unsetenv("STRIP")
	os.Unsetenv("READELF")

	// MSVC related envs.
	os.Unsetenv("INCLUDE")
	os.Unsetenv("LIB")
	os.Unsetenv("LIBPATH")
	os.Unsetenv("VSINSTALLDIR")
	os.Unsetenv("VCINSTALLDIR")

	// Clear toolchain-defined envs that must not leak into host-side dev builds.
	for _, item := range t.Envs {
		parts := strings.Split(item, "=")
		if len(parts) >= 2 {
			os.Unsetenv(parts[0])
		}
	}
}

// RuntimeFlags returns extra compiler flags needed when this toolchain
// cross-compiles against a sysroot. Self-contained toolchains (Android NDK,
// etc.) return nil.
func (t Toolchain) RuntimeFlags() []string {
	if t.GetName() != "clang" {
		return nil
	}

	var flags []string
	if strings.EqualFold(t.SystemName, "linux") {
		flags = append(flags, "--gcc-toolchain=/usr")
	}
	if strings.Contains(t.GetLD(), "lld") {
		flags = append(flags, "-fuse-ld=lld")
	}
	return flags
}

type WindowsKit struct {
	InstalledDir string `toml:"installed_dir"`
	Version      string `toml:"version"`
}
