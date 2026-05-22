package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type Toolchain struct {
	Url             string `toml:"url"`                       // Download url or local file url.
	SHA256          string `toml:"sha256"`                    // SHA256 of the toolchain archive, used for verification and caching.
	Name            string `toml:"name"`                      // It should be "gcc", "msvc", "clang-cl", "clang" and "msys2".
	Version         string `toml:"version"`                   // It should be version of gcc/msvc/clang.
	Archive         string `toml:"archive,omitempty"`         // Archive can be changed to avoid conflict.
	Path            string `toml:"path"`                      // Runtime path of tool, it's relative path and would be converted to absolute path later.
	SystemName      string `toml:"system_name"`               // It would be "Windows", "Linux", "Android" and so on.
	SystemProcessor string `toml:"system_processor"`          // It would be "x86_64", "aarch64" and so on.
	Host            string `toml:"host"`                      // It would be "x86_64-linux-gnu", "aarch64-linux-gnu" and so on.
	EmbeddedSystem  bool   `toml:"embedded_system,omitempty"` // Whether it's for embedded system, like mcu or bare-metal.
	CrosstoolPrefix string `toml:"crosstool_prefix"`          // It would be like "x86_64-linux-gnu-"

	// C/C++ standard.
	CStandard   string `toml:"c_standard,omitempty"`
	CXXStandard string `toml:"cxx_standard,omitempty"`

	// Mandatory fields.
	CC  string `toml:"cc"`  // C language compiler.
	CXX string `toml:"cxx"` // C++ language compiler.

	// Compiler target triplets for multi-target compiler drivers (e.g. qcc).
	// CMake maps these to CMAKE_C/CXX_COMPILER_TARGET, which translates to -V flags.
	// Required when the compiler driver defaults to a different architecture
	CCompilerTarget   string `toml:"c_compiler_target,omitempty"`
	CXXCompilerTarget string `toml:"cxx_compiler_target,omitempty"`

	// Core compiler tools (Essential).
	CPP string `toml:"cpp,omitempty"` // C preprocessor.
	AR  string `toml:"ar,omitempty"`  // Archive static library.
	LD  string `toml:"ld,omitempty"`  // Link executable.
	AS  string `toml:"as,omitempty"`  // Assemble assembly code.

	// Object file manipulation tools.
	OBJCOPY string `toml:"objcopy,omitempty"` // Copy object file.
	OBJDUMP string `toml:"objdump,omitempty"` // Dump object file.
	STRIP   string `toml:"strip,omitempty"`   // Strip executable and library.
	READELF string `toml:"readelf,omitempty"` // Read ELF file.
	SIZE    string `toml:"size,omitempty"`    // Display file size.
	STRINGS string `toml:"strings,omitempty"` // Display strings in file.

	// Symbol and archive tools.
	NM     string `toml:"nm,omitempty"`     // List symbols in object file.
	RANLIB string `toml:"ranlib,omitempty"` // Index static library.

	// Code coverage tools.
	GCOV string `toml:"gcov,omitempty"` // Gcov code coverage.

	// Debug and analysis tools.
	ADDR2LINE string `toml:"addr2line,omitempty"` // Convert address to line number.
	CXXFILT   string `toml:"cxxfilt,omitempty"`   // C++ symbol demangler.

	// Additional compiler tools.
	FC string `toml:"fc,omitempty"` // Compile Fortran code.

	// Platform-aware envs, flags.
	Envs           []string `toml:"envs"`
	CFlags         []string `toml:"cflags"`
	CXXFlags       []string `toml:"cxxflags"`
	LinkFlags      []string `toml:"linkflags"`
	CFlagsDebug    []string `toml:"cflags_debug"`
	CXXFlagsDebug  []string `toml:"cxxflags_debug"`
	LinkFlagsDebug []string `toml:"linkflags_debug"`

	// Internal fields.
	MSVC        context.MSVC `toml:"-"`
	ctx         context.Context
	displayName string
	rootDir     string
	abspath     string
}

func (t Toolchain) setupEnvs() {
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

func (t Toolchain) effectiveFlags(buildType string) (cflags, cxxflags, linkflags []string) {
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

		if len(t.LinkFlagsDebug) > 0 {
			linkflags = t.LinkFlagsDebug
		} else {
			linkflags = t.LinkFlags
		}
		return cflags, cxxflags, linkflags
	}

	return t.CFlags, t.CXXFlags, t.LinkFlags
}

func (t Toolchain) generate(toolchain *strings.Builder) error {
	writeIfNotEmpty := func(key, value string) {
		if value != "" {
			fmt.Fprintf(toolchain, "set(%s %q)\n", key, "${TOOLCHAIN_DIR}/"+value)
		}
	}
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
	buildType := ""
	if t.ctx != nil {
		buildType = t.ctx.BuildType()
	}
	cflags, cxxflags, linkflags := t.effectiveFlags(buildType)

	fmt.Fprintf(toolchain, "\n# ============== Cross-compile target system ============== #\n")
	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_SYSTEM_NAME", t.cmakeSystemName())
	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_SYSTEM_PROCESSOR", t.SystemProcessor)

	fmt.Fprintf(toolchain, "\n# ============== Cross-compile toolchain ============== #\n")

	switch runtime.GOOS {
	case "windows":
		if t.Name == "msvc" || t.Name == "clang-cl" || t.Name == "clang" {
			fmt.Fprintf(toolchain, "set(%s %q)\n", "TOOLCHAIN_DIR", filepath.ToSlash(t.abspath))
		}

	case "linux":
		if t.Path == "/usr/bin" {
			fmt.Fprintf(toolchain, "set(%s %q)\n", "TOOLCHAIN_DIR", "/usr/bin")
		} else {
			if strings.HasPrefix(t.Url, "file:///") {
				fmt.Fprintf(toolchain, "set(%s %q)\n", "TOOLCHAIN_DIR", t.abspath)
			} else {
				fmt.Fprintf(toolchain, "set(%s %q)\n", "TOOLCHAIN_DIR", fileio.ToRelPath(t.abspath))
			}
		}
	}

	writeIfNotEmpty("CMAKE_C_COMPILER", strings.Split(t.CC, " ")[0])
	writeIfNotEmpty("CMAKE_CXX_COMPILER", strings.Split(t.CXX, " ")[0])
	writeIfNotEmpty("CMAKE_AR", t.AR)
	writeIfNotEmpty("CMAKE_LINKER", t.LD)

	// Configure compiler targets are usually required by embed platform, like qnx.
	if t.CCompilerTarget != "" {
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_C_COMPILER_TARGET", t.CCompilerTarget)
	}
	if t.CXXCompilerTarget != "" {
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_CXX_COMPILER_TARGET", t.CXXCompilerTarget)
	}

	switch t.Name {
	case "gcc", "clang":
		writeIfNotEmpty("CMAKE_ASM_COMPILER", t.AS)
		writeIfNotEmpty("CMAKE_NM", t.NM)
		writeIfNotEmpty("CMAKE_Fortran_COMPILER", t.FC)
		writeIfNotEmpty("CMAKE_RANLIB", t.RANLIB)
		writeIfNotEmpty("CMAKE_OBJCOPY", t.OBJCOPY)
		writeIfNotEmpty("CMAKE_OBJDUMP", t.OBJDUMP)
		writeIfNotEmpty("CMAKE_STRIP", t.STRIP)
		writeIfNotEmpty("CMAKE_READELF", t.READELF)

		// For clang, if using lld, add LLVM runtime library flags to linker.
		if t.Name == "clang" && t.LD != "" && strings.Contains(t.LD, "lld") {
			fmt.Fprint(toolchain, "\n# Use LLVM lld linker, compiler-rt runtime and libc++ for clang.\n")
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_CXX_FLAGS_INIT " -stdlib=libc++")`+"\n")

			fmt.Fprintf(toolchain, `string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind")`+"\n")
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind")`+"\n")
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind")`+"\n")
		}

	case "qcc":
		writeIfNotEmpty("CMAKE_NM", t.NM)
		writeIfNotEmpty("CMAKE_RANLIB", t.RANLIB)
		writeIfNotEmpty("CMAKE_OBJDUMP", t.OBJDUMP)
		writeIfNotEmpty("CMAKE_STRIP", t.STRIP)

		fmt.Fprint(toolchain, "\n# QNX cross-compile settings.\n")
		qnxTarget := fileio.ToRelPath(filepath.Join(t.GetRootDir(), "target/qnx"))
		fmt.Fprintf(toolchain, "set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES %q)\n", filepath.Join(qnxTarget, "usr/include"))
		fmt.Fprintf(toolchain, "set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES %q)\n", filepath.Join(qnxTarget, "usr/include"))

	case "msvc", "clang-cl":
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_MT", filepath.ToSlash(t.MSVC.MT))
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_COMPILER_INIT", filepath.ToSlash(t.MSVC.RC))

		// For Ninja generator with MSVC, add include/lib paths as compiler/linker flags.
		// Note: Environment variables (INCLUDE/LIB) must still be set in preConfigure()
		// because they are not inherited from toolchain file to the build phase.
		if len(t.MSVC.Includes) > 0 {
			fmt.Fprint(toolchain, "\n# MSVC include paths for C/C++ and RC compilers.\n")
			// Use string(APPEND ...) for better readability
			fmt.Fprintf(toolchain, `set(CMAKE_C_FLAGS_INIT "")`+"\n")
			for _, inc := range t.MSVC.Includes {
				fmt.Fprintf(toolchain, `string(APPEND CMAKE_C_FLAGS_INIT " /I\"%s\"")`+"\n", filepath.ToSlash(inc))
			}
			fmt.Fprintf(toolchain, `set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")`+"\n")

			// Build RC compiler flags
			fmt.Fprint(toolchain, "\n# RC FLAGS for RC compilers.\n")
			fmt.Fprintf(toolchain, `set(CMAKE_RC_FLAGS_INIT "/nologo")`+"\n")
			for _, inc := range t.MSVC.Includes {
				fmt.Fprintf(toolchain, `string(APPEND CMAKE_RC_FLAGS_INIT " /I\"%s\"")`+"\n", filepath.ToSlash(inc))
			}
		} else {
			fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_FLAGS_INIT", "/nologo")
			fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_FLAGS", "/nologo")
		}

		if len(t.MSVC.Libs) > 0 {
			fmt.Fprint(toolchain, "\n# MSVC library paths for linker.\n")
			fmt.Fprintf(toolchain, `set(CMAKE_EXE_LINKER_FLAGS_INIT " /NODEFAULTLIB:LIBCMT")`+"\n")
			for _, lib := range t.MSVC.Libs {
				// Windows SDK libs need to include the x64 subdirectory
				libPath := filepath.ToSlash(lib)
				if !strings.HasSuffix(libPath, "/x64") && !strings.Contains(libPath, "/MSVC/") {
					libPath = filepath.ToSlash(filepath.Join(lib, "x64"))
				}
				fmt.Fprintf(toolchain, `string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " /LIBPATH:\"%s\"")`+"\n", libPath)
			}
			fmt.Fprintf(toolchain, `set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")`+"\n")
			fmt.Fprintf(toolchain, `set(CMAKE_MODULE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")`+"\n")
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

	if t.EmbeddedSystem {
		fmt.Fprint(toolchain, "\n# Embedded system settings.\n")
		fmt.Fprintf(toolchain, "set(CMAKE_SYSTEM_INCLUDE_PATH %s)\n", "\"/include\"")
		fmt.Fprintf(toolchain, "set(CMAKE_SYSTEM_LIBRARY_PATH %s)\n", "\"/lib\"")
		fmt.Fprintf(toolchain, "set(CMAKE_SYSTEM_PROGRAM_PATH %s)\n", "\"/bin\"")
		fmt.Fprintf(toolchain, "set(CMAKE_TRY_COMPILE_TARGET_TYPE %s)\n", "STATIC_LIBRARY")

		fmt.Fprintf(toolchain, "set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_ASM_USE_RESPONSE_FILE_FOR_OBJECTS %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_ASM_USE_RESPONSE_FILE_FOR_LIBRARIES %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES %s)\n", "0")
		fmt.Fprintf(toolchain, "set(CMAKE_ASM_USE_RESPONSE_FILE_FOR_INCLUDES %s)\n", "0")

		fmt.Fprintf(toolchain, "set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)\n")
	}

	if len(cflags) > 0 || len(cxxflags) > 0 || len(linkflags) > 0 {
		fmt.Fprint(toolchain, "\n# Append extra build flags.\n")

		// If both cflags and cxxflags exist, use foreach to avoid duplication
		if len(cflags) > 0 && len(cxxflags) > 0 {
			fmt.Fprint(toolchain, "foreach(flag_var CMAKE_C_FLAGS_INIT CMAKE_CXX_FLAGS_INIT)\n")
			appendFlags("${flag_var}", cflags, "  ")
			fmt.Fprint(toolchain, "endforeach()\n")
		} else {
			appendFlags("CMAKE_C_FLAGS_INIT", cflags, "")
			appendFlags("CMAKE_CXX_FLAGS_INIT", cxxflags, "")
		}

		if len(linkflags) > 0 {
			fmt.Fprint(toolchain, "foreach(flag_var CMAKE_EXE_LINKER_FLAGS_INIT CMAKE_SHARED_LINKER_FLAGS_INIT CMAKE_MODULE_LINKER_FLAGS_INIT)\n")
			appendFlags("${flag_var}", linkflags, "  ")
			fmt.Fprint(toolchain, "endforeach()\n")
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

func (t Toolchain) GetLinkFlags() []string {
	return t.LinkFlags
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
	switch strings.ToLower(t.SystemName) {
	case "windows":
		return "Windows"
	case "linux":
		return "Linux"
	case "android":
		return "Android"
	case "qnx":
		return "QNX"
	default:
		panic("unsupported operation system: " + t.SystemName)
	}
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

				// For Clang, add --gcc-toolchain to help find GCC runtime files (crtbeginS.o, etc.)
				if t.GetName() == "clang" {
					ccFlags = append(ccFlags, "--gcc-toolchain=/usr")
					cxxFlags = append(cxxFlags, "--gcc-toolchain=/usr")
				}

				// For clang with lld, add LLVM runtime library flags.
				if t.GetName() == "clang" && strings.Contains(t.GetLD(), "lld") {
					ccFlags = append(ccFlags, "-fuse-ld=lld", "--rtlib=compiler-rt", "--unwindlib=libunwind")
					cxxFlags = append(cxxFlags, "-stdlib=libc++", "-fuse-ld=lld", "--rtlib=compiler-rt", "--unwindlib=libunwind")
				}
			}
			os.Setenv("CC", strings.Join(ccFlags, " "))
			os.Setenv("CXX", strings.Join(cxxFlags, " "))
		}
	} else {
		ccFlags = append(ccFlags, cc)
		cxxFlags = append(cxxFlags, cxx)

		if rootfs != nil {
			ccFlags = append(ccFlags, "--sysroot="+rootfs.GetAbsDir())
			cxxFlags = append(ccFlags, "--sysroot="+rootfs.GetAbsDir())

			// For Clang, add --gcc-toolchain to help find GCC runtime files (crtbeginS.o, etc.)
			if t.GetName() == "clang" {
				ccFlags = append(ccFlags, "--gcc-toolchain=/usr")
				cxxFlags = append(cxxFlags, "--gcc-toolchain=/usr")
			}

			// For clang with lld, add LLVM runtime library flags.
			if t.GetName() == "clang" && strings.Contains(t.GetLD(), "lld") {
				ccFlags = append(ccFlags, "-fuse-ld=lld", "--rtlib=compiler-rt", "--unwindlib=libunwind")
				cxxFlags = append(cxxFlags, "-stdlib=libc++", "-fuse-ld=lld", "--rtlib=compiler-rt", "--unwindlib=libunwind")
			}
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

type WindowsKit struct {
	InstalledDir string `toml:"installed_dir"`
	Version      string `toml:"version"`
}
