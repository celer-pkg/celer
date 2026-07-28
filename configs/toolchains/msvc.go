package toolchains

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/cmd"
)

type MSVC struct {
	Infos
	BuildTools
	BuildFlags
	context.Context
}

func (m *MSVC) Dir(abspath string) string {
	return filepath.ToSlash(abspath)
}

func (m *MSVC) AssembleBuildTools(toolchain *strings.Builder) {
	// Mandatory build tools
	m.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER", strings.Split(m.CC, " ")[0])
	m.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER", strings.Split(m.CXX, " ")[0])
	m.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER_TARGET", m.CCompilerTarget)
	m.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER_TARGET", m.CXXCompilerTarget)
	m.writeIfNotEmpty(toolchain, "CMAKE_AR", m.AR)
	m.writeIfNotEmpty(toolchain, "CMAKE_LINKER", m.LD)

	// Optional build tools.
	m.writeIfNotEmpty(toolchain, "CMAKE_ASM_COMPILER", m.AS)
	m.writeIfNotEmpty(toolchain, "CMAKE_NM", m.NM)
	m.writeIfNotEmpty(toolchain, "CMAKE_Fortran_COMPILER", m.FC)
	m.writeIfNotEmpty(toolchain, "CMAKE_RANLIB", m.RANLIB)
	m.writeIfNotEmpty(toolchain, "CMAKE_OBJCOPY", m.OBJCOPY)
	m.writeIfNotEmpty(toolchain, "CMAKE_OBJDUMP", m.OBJDUMP)
	m.writeIfNotEmpty(toolchain, "CMAKE_STRIP", m.STRIP)
	m.writeIfNotEmpty(toolchain, "CMAKE_READELF", m.READELF)

	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_MT", filepath.ToSlash(m.MSVC.MT))
	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_COMPILER_INIT", filepath.ToSlash(m.MSVC.RC))

	// For Ninja generator with MSVC, add include/lib paths as compiler/linker flags.
	// Note: Environment variables (INCLUDE/LIB) must still be set in preConfigure()
	// because they are not inherited from toolchain file to the build phase.
	if len(m.MSVC.Includes) > 0 {
		fmt.Fprint(toolchain, "\n# MSVC include paths for C/C++ and RC compilers.\n")
		// Use string(APPEND ...) for better readability
		fmt.Fprintf(toolchain, `set(CMAKE_C_FLAGS_INIT "")`+"\n")
		for _, inc := range m.MSVC.Includes {
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_C_FLAGS_INIT " /I\"%s\"")`+"\n", filepath.ToSlash(inc))
		}
		fmt.Fprintf(toolchain, `set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")`+"\n")

		// Build RC compiler flags
		fmt.Fprint(toolchain, "\n# RC FLAGS for RC compilers.\n")
		fmt.Fprintf(toolchain, `set(CMAKE_RC_FLAGS_INIT "/nologo")`+"\n")
		for _, inc := range m.MSVC.Includes {
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_RC_FLAGS_INIT " /I\"%s\"")`+"\n", filepath.ToSlash(inc))
		}
	} else {
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_FLAGS_INIT", "/nologo")
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_FLAGS", "/nologo")
	}

	if len(m.MSVC.Libs) > 0 {
		fmt.Fprint(toolchain, "\n# MSVC library paths for linker.\n")
		fmt.Fprintf(toolchain, `set(CMAKE_EXE_LINKER_FLAGS_INIT " /NODEFAULTLIB:LIBCMT")`+"\n")
		for _, lib := range m.MSVC.Libs {
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

func (m *MSVC) CFlags() []string {
	return []string{}
}

func (m *MSVC) CXXFlags() []string {
	return []string{}
}

func (m *MSVC) LDFlags() []string {
	return []string{}
}

func (m *MSVC) RuntimeFlags() []string {
	return []string{}
}

// ReadMSVCEnvs call MSVC's batch file to get all build environment variables.
func ReadMSVCEnvs(toolchain context.Toolchain) (map[string]string, error) {
	// Read MSVC environment variables.
	// TODO: the `x64` may be different depending on the platform.
	command := fmt.Sprintf(`call "%s" x64 && set`, toolchain.GetMSVC().VCVars)
	executor := cmd.NewExecutor("", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return nil, err
	}

	// Parse environment variables from output.
	var msvcEnvs = make(map[string]string)
	lines := strings.SplitSeq(output, "\n")
	for line := range lines {
		line = strings.TrimSpace(line)
		if line != "" && strings.Contains(line, "=") {
			parts := strings.Split(line, "=")

			// Unify "Path" to "PATH".
			if parts[0] == "Path" {
				parts[0] = "PATH"
			}
			msvcEnvs[parts[0]] = parts[1]
		}
	}

	return msvcEnvs, nil
}
