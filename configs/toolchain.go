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
	Url             string `toml:"url"`               // Download url or local file url.
	Name            string `toml:"name"`              // It should be "gcc", "msvc", "clang-cl", "clang" and "msys2".
	Version         string `toml:"version"`           // It should be version of gcc/msvc/clang.
	Archive         string `toml:"archive,omitempty"` // Archive can be changed to avoid conflict.
	Path            string `toml:"path"`              // Runtime path of tool, it's relative path and would be converted to absolute path later.
	SystemName      string `toml:"system_name"`       // It would be "Windows", "Linux", "Android" and so on.
	SystemProcessor string `toml:"system_processor"`  // It would be "x86_64", "aarch64" and so on.
	Host            string `toml:"host"`              // It would be "x86_64-linux-gnu", "aarch64-linux-gnu" and so on.
	CrosstoolPrefix string `toml:"crosstool_prefix"`  // It would be like "x86_64-linux-gnu-"

	// C/C++ standard.
	CStandard   string `toml:"c_standard,omitempty"`
	CXXStandard string `toml:"cxx_standard,omitempty"`

	// Mandatory fields.
	CC  string `toml:"cc"`  // C language compiler.
	CXX string `toml:"cxx"` // C++ language compiler.

	// Suggested field.
	AR string `toml:"ar"` // Archive static library.
	LD string `toml:"ld"` // Link executable.

	// Optional fields for linux.
	AS      string `toml:"as,omitempty"`      // Assemble assembly code.
	FC      string `toml:"fc,omitempty"`      // Compile Fortran code.
	RANLIB  string `toml:"ranlib,omitempty"`  // Index static library.
	NM      string `toml:"nm,omitempty"`      // List symbols in static library.
	OBJCOPY string `toml:"objcopy,omitempty"` // Copy object file.
	OBJDUMP string `toml:"objdump,omitempty"` // Dump object file.
	STRIP   string `toml:"strip,omitempty"`   // Strip executable and library.
	READELF string `toml:"readelf,omitempty"` // Read ELF file.

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
func (t Toolchain) GetMSVC() *context.MSVC {
	return &t.MSVC
}

func (t Toolchain) GetFullPath() string {
	return t.fullpath
}

func (t Toolchain) GetCrosstoolPrefixPath() string {
	return filepath.Join(t.fullpath, t.CrosstoolPrefix)
}

func (t Toolchain) SetEnvs(rootfs context.RootFS, buildsystem string, ccacheEnabled bool) {
	os.Setenv("CROSSTOOL_PREFIX", t.GetCrosstoolPrefix())
	os.Setenv("HOST", t.GetHost())

	if ccacheEnabled {
		// For Windows + MSVC with Makefiles, don't set ccache in CC/CXX environment variables
		// because MSYS2 shell cannot handle "ccache cl.exe" as a command.
		if runtime.GOOS == "windows" && (t.GetName() == "msvc" || t.GetName() == "clang-cl") && buildsystem == "makefiles" {
			os.Setenv("CC", t.GetCC())
			os.Setenv("CXX", t.GetCXX())
		} else {
			if rootfs != nil {
				sysrootDir := rootfs.GetFullPath()
				os.Setenv("CC", "ccache "+t.GetCC()+" --sysroot="+sysrootDir)
				os.Setenv("CXX", "ccache "+t.GetCXX()+" --sysroot="+sysrootDir)
			} else {
				os.Setenv("CC", "ccache "+t.GetCC())
				os.Setenv("CXX", "ccache "+t.GetCXX())
			}
		}
	} else {
		if rootfs != nil {
			sysrootDir := rootfs.GetFullPath()
			os.Setenv("CC", t.GetCC()+" --sysroot="+sysrootDir)
			os.Setenv("CXX", t.GetCXX()+" --sysroot="+sysrootDir)
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
