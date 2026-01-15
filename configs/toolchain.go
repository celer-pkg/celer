package configs

import (
	"celer/context"
	"celer/pkgs/dirs"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

type Toolchain struct {
	Url             string `toml:"url"`                       // Download url or local file url.
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

	// Internal fields.
	MSVC        context.MSVC `toml:"-"`
	ctx         context.Context
	displayName string
	rootDir     string
	fullpath    string
}

func (t Toolchain) generate(toolchain *strings.Builder) error {
	writeIfNotEmpty := func(key, value string) {
		if value != "" {
			fmt.Fprintf(toolchain, "set(%-30s%q)\n", key, "${TOOLCHAIN_DIR}/"+value)
		}
	}

	fmt.Fprintf(toolchain, "\n# Target information for cross-compile.\n")
	fmt.Fprintf(toolchain, "set(%-24s%q)\n", "CMAKE_SYSTEM_NAME", t.SystemName)
	fmt.Fprintf(toolchain, "set(%-24s%q)\n", "CMAKE_SYSTEM_PROCESSOR", t.SystemProcessor)

	fmt.Fprintf(toolchain, "\n# Toolchain for cross-compile.\n")
	cmakepath := strings.TrimPrefix(t.fullpath, dirs.WorkspaceDir+string(os.PathSeparator))

	switch runtime.GOOS {
	case "windows":
		if t.Name == "msvc" || t.Name == "clang-cl" || t.Name == "clang" {
			fmt.Fprintf(toolchain, "set(%-30s%q)\n", "TOOLCHAIN_DIR", filepath.ToSlash(cmakepath))
		}

	case "linux":
		if t.Path == "/usr/bin" {
			fmt.Fprintf(toolchain, "set(%-30s%q)\n", "TOOLCHAIN_DIR", "/usr/bin")
		} else {
			fmt.Fprintf(toolchain, "set(%-30s%q)\n", "TOOLCHAIN_DIR", "${CELER_ROOT}/"+cmakepath)
		}
	}

	writeIfNotEmpty("CMAKE_C_COMPILER", t.CC)
	writeIfNotEmpty("CMAKE_CXX_COMPILER", t.CXX)
	writeIfNotEmpty("CMAKE_AR", t.AR)
	writeIfNotEmpty("CMAKE_LINKER", t.LD)

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
			// Use LLVM's libc++ for C++ standard library.
			fmt.Fprint(toolchain, "\n# Use LLVM lld linker, compiler-rt runtime and libc++ for clang.\n")
			fmt.Fprintf(toolchain, "string(APPEND CMAKE_CXX_FLAGS_INIT \" -stdlib=libc++\")\n")

			// These flags are only needed during linking, if we set them in CMAKE_C_FLAGS_INIT,
			// they may cause warnings during compilation.
			fmt.Fprintf(toolchain, "string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT \" -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind\")\n")
			fmt.Fprintf(toolchain, "string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT \" -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind\")\n")
			fmt.Fprintf(toolchain, "string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT \" -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind\")\n")
		}

	case "msvc", "clang-cl":
		fmt.Fprintf(toolchain, "set(%-30s%q)\n", "CMAKE_MT", filepath.ToSlash(t.MSVC.MT))
		fmt.Fprintf(toolchain, "set(%-30s%q)\n", "CMAKE_RC_COMPILER_INIT", filepath.ToSlash(t.MSVC.RC))

		// For Ninja generator with MSVC, add include/lib paths as compiler/linker flags.
		// Note: Environment variables (INCLUDE/LIB) must still be set in preConfigure()
		// because they are not inherited from toolchain file to the build phase.
		if len(t.MSVC.Includes) > 0 {
			fmt.Fprint(toolchain, "\n# MSVC include paths for C/C++ and RC compilers.\n")
			// Use string(APPEND ...) for better readability
			fmt.Fprintf(toolchain, "set(CMAKE_C_FLAGS_INIT \"\")\n")
			for _, inc := range t.MSVC.Includes {
				fmt.Fprintf(toolchain, "string(APPEND CMAKE_C_FLAGS_INIT \" /I\\\"%s\\\"\")\n", filepath.ToSlash(inc))
			}
			fmt.Fprintf(toolchain, "set(CMAKE_CXX_FLAGS_INIT \"${CMAKE_C_FLAGS_INIT}\")\n")

			// Build RC compiler flags
			fmt.Fprintf(toolchain, "set(CMAKE_RC_FLAGS_INIT \"/nologo\")\n")
			for _, inc := range t.MSVC.Includes {
				fmt.Fprintf(toolchain, "string(APPEND CMAKE_RC_FLAGS_INIT \" /I\\\"%s\\\"\")\n", filepath.ToSlash(inc))
			}
			fmt.Fprintf(toolchain, "set(CMAKE_RC_FLAGS \"${CMAKE_RC_FLAGS_INIT}\")\n")
		} else {
			fmt.Fprintf(toolchain, "set(%-30s%q)\n", "CMAKE_RC_FLAGS_INIT", "/nologo")
			fmt.Fprintf(toolchain, "set(%-30s%q)\n", "CMAKE_RC_FLAGS", "/nologo")
		}

		if len(t.MSVC.Libs) > 0 {
			fmt.Fprint(toolchain, "\n# MSVC library paths for linker.\n")
			// Use string(APPEND ...) for better readability
			fmt.Fprintf(toolchain, "set(CMAKE_EXE_LINKER_FLAGS_INIT \"\")\n")
			for _, lib := range t.MSVC.Libs {
				// Windows SDK libs need to include the x64 subdirectory
				libPath := filepath.ToSlash(lib)
				if !strings.HasSuffix(libPath, "/x64") && !strings.Contains(libPath, "/MSVC/") {
					libPath = filepath.ToSlash(filepath.Join(lib, "x64"))
				}
				fmt.Fprintf(toolchain, "string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT \" /LIBPATH:\\\"%s\\\"\")\n", libPath)
			}
			fmt.Fprintf(toolchain, "set(CMAKE_SHARED_LINKER_FLAGS_INIT \"${CMAKE_EXE_LINKER_FLAGS_INIT}\")\n")
			fmt.Fprintf(toolchain, "set(CMAKE_MODULE_LINKER_FLAGS_INIT \"${CMAKE_EXE_LINKER_FLAGS_INIT}\")\n")
		}
	}

	// Write C/C++ language standard.
	if t.CStandard != "" || t.CXXStandard != "" {
		fmt.Fprint(toolchain, "\n# C/CXX language standard.\n")

		if t.CStandard != "" {
			fmt.Fprintf(toolchain, "set(%-30s%s)\n", "CMAKE_C_STANDARD", strings.TrimPrefix(t.CStandard, "c"))
			fmt.Fprintf(toolchain, "set(%-30s%s)\n", "CMAKE_C_STANDARD_REQUIRED", "ON")
		}

		if t.CXXStandard != "" {
			fmt.Fprintf(toolchain, "set(%-30s%s)\n", "CMAKE_CXX_STANDARD", strings.TrimPrefix(t.CXXStandard, "c++"))
			fmt.Fprintf(toolchain, "set(%-30s%s)\n", "CMAKE_CXX_STANDARD_REQUIRED", "ON")
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

	return nil
}

func (t Toolchain) GetName() string {
	return t.Name
}

func (t Toolchain) GetHost() string {
	return t.Host
}

func (t Toolchain) GetVersion() string {
	return t.Version
}

func (t Toolchain) GetPath() string {
	return t.Path
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

func (t Toolchain) GetFullPath() string {
	return t.fullpath
}

func (t Toolchain) GetCrosstoolPrefixPath() string {
	return filepath.Join(t.fullpath, t.CrosstoolPrefix)
}

func (t Toolchain) SetEnvs(rootfs context.RootFS, buildsystem string) {
	os.Setenv("CROSSTOOL_PREFIX", t.GetCrosstoolPrefix())
	os.Setenv("HOST", t.GetHost())

	if t.ctx.CCacheEnabled() {
		// For Windows + MSVC with Makefiles, don't set ccache in CC/CXX environment variables,
		// because MSYS2 shell cannot handle "ccache cl.exe" as a command.
		if runtime.GOOS == "windows" && (t.GetName() == "msvc" || t.GetName() == "clang-cl") && buildsystem == "makefiles" {
			os.Setenv("CC", t.GetCC())
			os.Setenv("CXX", t.GetCXX())
		} else {
			if rootfs != nil {
				sysrootDir := rootfs.GetFullPath()
				ccFlags := " --sysroot=" + sysrootDir
				cxxFlags := ccFlags

				// For Clang, add --gcc-toolchain to help find GCC runtime files (crtbeginS.o, etc.)
				if t.GetName() == "clang" {
					ccFlags += " --gcc-toolchain=/usr"
					cxxFlags += " --gcc-toolchain=/usr"
				}

				// For clang with lld, add LLVM runtime library flags.
				if t.GetName() == "clang" && strings.Contains(t.GetLD(), "lld") {
					ccFlags += " -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind"
					cxxFlags += " -stdlib=libc++ -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind"
				}
				os.Setenv("CC", "ccache "+t.GetCC()+ccFlags)
				os.Setenv("CXX", "ccache "+t.GetCXX()+cxxFlags)
			} else {
				os.Setenv("CC", "ccache "+t.GetCC())
				os.Setenv("CXX", "ccache "+t.GetCXX())
			}
		}
	} else {
		if rootfs != nil {
			sysrootDir := rootfs.GetFullPath()
			ccFlags := " --sysroot=" + sysrootDir
			cxxFlags := ccFlags

			// For Clang, add --gcc-toolchain to help find GCC runtime files (crtbeginS.o, etc.)
			if t.GetName() == "clang" {
				ccFlags += " --gcc-toolchain=/usr"
				cxxFlags += " --gcc-toolchain=/usr"
			}

			// For clang with lld, add LLVM runtime library flags.
			if t.GetName() == "clang" && strings.Contains(t.GetLD(), "lld") {
				ccFlags += " -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind"
				cxxFlags += " -stdlib=libc++ -fuse-ld=lld --rtlib=compiler-rt --unwindlib=libunwind"
			}
			os.Setenv("CC", t.GetCC()+ccFlags)
			os.Setenv("CXX", t.GetCXX()+cxxFlags)
		}
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
}

type WindowsKit struct {
	InstalledDir string `toml:"installed_dir"`
	Version      string `toml:"version"`
}
