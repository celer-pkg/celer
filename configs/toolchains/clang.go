package toolchains

import (
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
)

type Clang struct {
	Infos
	BuildTools
	BuildFlags
	context.Context
}

func (c *Clang) Dir(abspath string) string {
	if strings.Contains(abspath, "Microsoft Visual Studio") {
		return filepath.ToSlash(abspath)
	} else {
		return "${WORKSPACE_ROOT}/downloads/tools/" + filepath.ToSlash(c.Path)
	}
}

func (c *Clang) AssembleBuildTools(toolchain *strings.Builder) {
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
}

func (c *Clang) CFlags() []string {
	return []string{}
}

func (c *Clang) CXXFlags() []string {
	return []string{}
}

func (c *Clang) LDFlags() []string {
	return []string{}
}

func (c *Clang) RuntimeFlags() []string {
	var flags []string

	if !strings.EqualFold(c.SystemName, "android") {
		if strings.EqualFold(c.SystemName, "linux") {
			flags = append(flags, "--gcc-toolchain=/usr")
		}
		if strings.Contains(c.LD, "lld") {
			flags = append(flags, "-fuse-ld=lld")
		}
	}

	return flags
}
