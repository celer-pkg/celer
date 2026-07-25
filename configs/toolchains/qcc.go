package toolchains

import (
	"fmt"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type QCC struct {
	Infos
	BuildTools
	BuildFlags
	context.Context
}

func (q *QCC) Dir(abspath string) string {
	if strings.HasPrefix(q.Url, "file:///") {
		return strings.TrimPrefix(filepath.ToSlash(q.Url), "file:///")
	}

	return "${WORKSPACE_ROOT}/downloads/tools/" + filepath.ToSlash(q.Path)
}

func (q *QCC) AssembleBuildTools(toolchain *strings.Builder) {
	// Mandatory build tools
	q.writeIfNotEmpty(toolchain, "CMAKE_C_COMPILER", strings.Split(q.CC, " ")[0])
	q.writeIfNotEmpty(toolchain, "CMAKE_CXX_COMPILER", strings.Split(q.CXX, " ")[0])
	q.writeIfNotEmpty(toolchain, "CMAKE_AR", q.AR)
	q.writeIfNotEmpty(toolchain, "CMAKE_LINKER", q.LD)

	// Optional build tools.
	q.writeIfNotEmpty(toolchain, "CMAKE_ASM_COMPILER", q.AS)
	q.writeIfNotEmpty(toolchain, "CMAKE_NM", q.NM)
	q.writeIfNotEmpty(toolchain, "CMAKE_Fortran_COMPILER", q.FC)
	q.writeIfNotEmpty(toolchain, "CMAKE_RANLIB", q.RANLIB)
	q.writeIfNotEmpty(toolchain, "CMAKE_OBJCOPY", q.OBJCOPY)
	q.writeIfNotEmpty(toolchain, "CMAKE_OBJDUMP", q.OBJDUMP)
	q.writeIfNotEmpty(toolchain, "CMAKE_STRIP", q.STRIP)
	q.writeIfNotEmpty(toolchain, "CMAKE_READELF", q.READELF)

	fmt.Fprint(toolchain, "\n# QNX cross-compile settings.\n")
	firstDir, _, _ := strings.Cut(filepath.ToSlash(q.Path), "/")
	rootDir := filepath.Join(q.Downloads(), "tools", firstDir)
	qnxTarget := fileio.ToRelPath(filepath.Join(rootDir, "target/qnx"))
	fmt.Fprintf(toolchain, "set(CMAKE_SYSROOT %q)\n", qnxTarget)
	fmt.Fprintf(toolchain, "set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES %q)\n", filepath.Join(qnxTarget, "usr/include"))
	fmt.Fprintf(toolchain, "set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES %q)\n", filepath.Join(qnxTarget, "usr/include"))

	// QNX_HOST and QNX_TARGET must be in the environment for qcc to function.
	var qnxHostRel string
	switch runtime.GOOS {
	case "linux":
		qnxHostRel = "host/linux/x86_64"
	case "windows":
		qnxHostRel = "host/win64/x86_64"
	case "darwin":
		qnxHostRel = "host/darwin/x86_64"
	}
	qnxHost := fileio.ToRelPath(filepath.Join(rootDir, qnxHostRel))
	fmt.Fprintf(toolchain, "set(ENV{QNX_HOST} %q)\n", qnxHost)
	fmt.Fprintf(toolchain, "set(ENV{QNX_TARGET} %q)\n", qnxTarget)
}

func (q *QCC) CFlags() []string {
	return []string{
		"-D_QNX_SOURCE",
		"-V" + q.CCompilerTarget,
	}
}

func (q *QCC) CXXFlags() []string {
	return []string{
		"-D_QNX_SOURCE",
		"-V" + q.CXXCompilerTarget,
	}
}

func (q *QCC) LDFlags() []string {
	return []string{}
}

func (q *QCC) RuntimeFlags() []string {
	return []string{}
}
