package toolchains

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
)

type ClangCL struct {
	Infos
	BuildTools
	BuildFlags
	context.Context
}

func (c *ClangCL) Dir(abspath string) string {
	return filepath.ToSlash(abspath)
}

func (c *ClangCL) AssembleBuildTools(toolchain *strings.Builder) {
	// Mandatory build tools
	c.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER", strings.Split(c.CC, " ")[0])
	c.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER", strings.Split(c.CXX, " ")[0])
	c.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER_TARGET", c.CCompilerTarget)
	c.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER_TARGET", c.CXXCompilerTarget)
	c.writeIfNotEmpty(toolchain, "CMAKE_AR", c.AR)
	c.writeIfNotEmpty(toolchain, "CMAKE_LINKER", c.LD)

	// Optional build tools.
	c.writeIfNotEmpty(toolchain, "CMAKE_ASM_COMPILER", c.AS)
	c.writeIfNotEmpty(toolchain, "CMAKE_NM", c.NM)
	c.writeIfNotEmpty(toolchain, "CMAKE_Fortran_COMPILER", c.FC)
	c.writeIfNotEmpty(toolchain, "CMAKE_RANLIB", c.RANLIB)
	c.writeIfNotEmpty(toolchain, "CMAKE_OBJCOPY", c.OBJCOPY)
	c.writeIfNotEmpty(toolchain, "CMAKE_OBJDUMP", c.OBJDUMP)
	c.writeIfNotEmpty(toolchain, "CMAKE_STRIP", c.STRIP)
	c.writeIfNotEmpty(toolchain, "CMAKE_READELF", c.READELF)

	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_MT", filepath.ToSlash(c.MSVC.MT))
	fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_COMPILER_INIT", filepath.ToSlash(c.MSVC.RC))

	// For Ninja generator with MSVC, add include/lib paths as compiler/linker flags.
	// Note: Environment variables (INCLUDE/LIB) must still be set in preConfigure()
	// because they are not inherited from toolchain file to the build phase.
	if len(c.MSVC.Includes) > 0 {
		fmt.Fprint(toolchain, "\n# MSVC include paths for C/C++ and RC compilers.\n")
		// Use string(APPEND ...) for better readability
		fmt.Fprintf(toolchain, `set(CMAKE_C_FLAGS_INIT "")`+"\n")
		for _, inc := range c.MSVC.Includes {
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_C_FLAGS_INIT " /I\"%s\"")`+"\n", filepath.ToSlash(inc))
		}
		fmt.Fprintf(toolchain, `set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")`+"\n")

		// Build RC compiler flags
		fmt.Fprint(toolchain, "\n# RC FLAGS for RC compilers.\n")
		fmt.Fprintf(toolchain, `set(CMAKE_RC_FLAGS_INIT "/nologo")`+"\n")
		for _, inc := range c.MSVC.Includes {
			fmt.Fprintf(toolchain, `string(APPEND CMAKE_RC_FLAGS_INIT " /I\"%s\"")`+"\n", filepath.ToSlash(inc))
		}
	} else {
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_FLAGS_INIT", "/nologo")
		fmt.Fprintf(toolchain, "set(%s %q)\n", "CMAKE_RC_FLAGS", "/nologo")
	}

	if len(c.MSVC.Libs) > 0 {
		fmt.Fprint(toolchain, "\n# MSVC library paths for linker.\n")
		fmt.Fprintf(toolchain, `set(CMAKE_EXE_LINKER_FLAGS_INIT " /NODEFAULTLIB:LIBCMT")`+"\n")
		for _, lib := range c.MSVC.Libs {
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

func (c *ClangCL) ReadBuiltinEnvs() (map[string]string, error) {
	return nil, nil
}

func (c *ClangCL) CFlags() []string {
	return []string{}
}

func (c *ClangCL) CXXFlags() []string {
	return []string{}
}

func (c *ClangCL) LDFlags() []string {
	return []string{}
}

func (c *ClangCL) RuntimeFlags() []string {
	return []string{}
}
