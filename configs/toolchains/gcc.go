package toolchains

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
)

type GCC struct {
	Infos
	BuildTools
	BuildFlags
	context.Context
}

func (g *GCC) Dir(abspath string) string {
	if strings.HasPrefix(g.Url, "file:///") {
		return strings.TrimPrefix(filepath.ToSlash(g.Url), "file:///")
	}

	if abspath == "/usr/bin" || g.Path == "/usr/bin" {
		return "/usr/bin"
	}

	return "${WORKSPACE_ROOT}/downloads/tools/" + filepath.ToSlash(g.Path)
}

func (g *GCC) AssembleBuildTools(toolchain *strings.Builder) {
	// Mandatory build tools
	g.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER", strings.Split(g.CC, " ")[0])
	g.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER", strings.Split(g.CXX, " ")[0])
	g.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER_TARGET", g.CCompilerTarget)
	g.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER_TARGET", g.CXXCompilerTarget)
	g.writeIfNotEmpty(toolchain, "CMAKE_AR", g.AR)
	g.writeIfNotEmpty(toolchain, "CMAKE_LINKER", g.LD)

	// Optional build tools.
	g.writeIfNotEmpty(toolchain, "CMAKE_ASM_COMPILER", g.AS)
	g.writeIfNotEmpty(toolchain, "CMAKE_NM", g.NM)
	g.writeIfNotEmpty(toolchain, "CMAKE_Fortran_COMPILER", g.FC)
	g.writeIfNotEmpty(toolchain, "CMAKE_RANLIB", g.RANLIB)
	g.writeIfNotEmpty(toolchain, "CMAKE_OBJCOPY", g.OBJCOPY)
	g.writeIfNotEmpty(toolchain, "CMAKE_OBJDUMP", g.OBJDUMP)
	g.writeIfNotEmpty(toolchain, "CMAKE_STRIP", g.STRIP)
	g.writeIfNotEmpty(toolchain, "CMAKE_READELF", g.READELF)

	if g.EmbeddedSystem {
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
}

func (g *GCC) ReadBuiltinEnvs() (map[string]string, error) {
	return nil, nil
}

func (g *GCC) CFlags() []string {
	return []string{}
}

func (g *GCC) CXXFlags() []string {
	return []string{}
}

func (g *GCC) LDFlags() []string {
	return []string{}
}

func (g *GCC) RuntimeFlags() []string {
	return []string{}
}
