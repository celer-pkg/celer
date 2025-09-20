package configs

import (
	"celer/pkgs/dirs"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type Toolchain struct {
	Url             string `toml:"url"`               // Download url or local file url.
	Name            string `toml:"name"`              // It should be "gcc", "msvc" and "clang".
	Version         string `toml:"version"`           // It should be version of gcc/msvc/clang.
	Archive         string `toml:"archive,omitempty"` // Archive can be changed to avoid conflict.
	Path            string `toml:"path"`              // Runtime path of tool, it's relative path and would be converted to absolute path later.
	SystemName      string `toml:"system_name"`       // It would be "Windows", "Linux", "Android" and so on.
	SystemProcessor string `toml:"system_processor"`  // It would be "x86_64", "aarch64" and so on.
	Host            string `toml:"host"`              // It would be "x86_64-linux-gnu", "aarch64-linux-gnu" and so on.
	CrosstoolPrefix string `toml:"crosstool_prefix"`  // It would be like "x86_64-linux-gnu-"

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
	ctx         Context
	displayName string
	rootDir     string
	fullpath    string
	cmakepath   string

	MSVC MSVC
}

type MSVC struct {
	VCVars string
	MT     string
	RC     string
}

func (t Toolchain) generate(toolchain *strings.Builder, hostName string) error {
	t.cmakepath = fmt.Sprintf("${WORKSPACE_DIR}/installed/%s-dev/bin", hostName)

	toolchain.WriteString("\n# Runtime paths.\n")
	toolchain.WriteString("set(PATH_LIST" + "\n")
	toolchain.WriteString(fmt.Sprintf(`	"%s"`, t.cmakepath) + "\n")
	toolchain.WriteString(")\n")
	toolchain.WriteString(fmt.Sprintf(`list(JOIN PATH_LIST "%s" PATH_STR)`, string(os.PathListSeparator)) + "\n")
	toolchain.WriteString(fmt.Sprintf(`set(ENV{PATH} "${PATH_STR}%s$ENV{PATH}")`, string(os.PathListSeparator)) + "\n")

	writeIfNotEmpty := func(key, value string) {
		if value != "" {
			fmt.Fprintf(toolchain, "set(%-25s\"%s\")\n", key, value)
		}
	}

	toolchain.WriteString("\n# TOOLCHAIN for cross-compile.\n")
	cmakepath := strings.TrimPrefix(t.fullpath, dirs.WorkspaceDir+string(os.PathSeparator))
	if t.Name == "msvc" {
		writeIfNotEmpty("TOOLCHAIN_DIR", filepath.ToSlash(cmakepath))
	} else {
		writeIfNotEmpty("TOOLCHAIN_DIR", "${WORKSPACE_DIR}/"+cmakepath)
	}
	writeIfNotEmpty("CMAKE_SYSTEM_NAME", t.SystemName)
	writeIfNotEmpty("CMAKE_SYSTEM_PROCESSOR", t.SystemProcessor)
	writeIfNotEmpty("CMAKE_C_COMPILER", t.CC)
	writeIfNotEmpty("CMAKE_CXX_COMPILER", t.CXX)
	writeIfNotEmpty("CMAKE_ASM_COMPILER", t.AS)
	writeIfNotEmpty("CMAKE_Fortran_COMPILER", t.FC)
	writeIfNotEmpty("CMAKE_RANLIB", t.RANLIB)
	writeIfNotEmpty("CMAKE_AR", t.AR)
	writeIfNotEmpty("CMAKE_LINKER", t.LD)
	writeIfNotEmpty("CMAKE_NM", t.NM)
	writeIfNotEmpty("CMAKE_OBJCOPY", t.OBJCOPY)
	writeIfNotEmpty("CMAKE_OBJDUMP", t.OBJDUMP)
	writeIfNotEmpty("CMAKE_STRIP", t.STRIP)
	writeIfNotEmpty("CMAKE_READELF", t.READELF)

	// Only linux may have sysroot.
	switch t.Name {
	case "gcc":
		toolchain.WriteString("\n")
		writeIfNotEmpty("CMAKE_C_FLAGS", "--sysroot=${CMAKE_SYSROOT} ${CMAKE_C_FLAGS}")
		writeIfNotEmpty("CMAKE_CXX_FLAGS", "--sysroot=${CMAKE_SYSROOT} ${CMAKE_CXX_FLAGS}")
	case "msvc":
		writeIfNotEmpty("CMAKE_MT", t.MSVC.MT)
		writeIfNotEmpty("CMAKE_RC_COMPILER_INIT", t.MSVC.RC)
		writeIfNotEmpty("CMAKE_RC_FLAGS_INIT", "/nologo")
	}

	return nil
}

type WindowsKit struct {
	InstalledDir string `toml:"installed_dir"`
	Version      string `toml:"version"`
}
