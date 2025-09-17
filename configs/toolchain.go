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

	msvc msvc
}

type msvc struct {
	VCVars string
	MtPath string
	RcPath string
}

func (t Toolchain) generate(toolchain *strings.Builder, hostName string) error {
	cmakepaths := []string{
		fmt.Sprintf("${WORKSPACE_DIR}/installed/%s-dev/bin", hostName),
	}

	cmakepath := strings.TrimPrefix(t.fullpath, dirs.WorkspaceDir+string(os.PathSeparator))
	if cmakepath != t.fullpath {
		cmakepaths = append(cmakepaths, fmt.Sprintf("${WORKSPACE_DIR}/%s", filepath.ToSlash(cmakepath)))
	} else {
		cmakepaths = append(cmakepaths, filepath.ToSlash(cmakepath))
	}

	toolchain.WriteString("\n# Set runtime path.\n")
	toolchain.WriteString("set(PATH_LIST" + "\n")
	for _, path := range cmakepaths {
		toolchain.WriteString(fmt.Sprintf(`	"%s"`, path) + "\n")
	}
	toolchain.WriteString(")\n")
	toolchain.WriteString(fmt.Sprintf(`list(JOIN PATH_LIST "%s" PATH_STR)`, string(os.PathListSeparator)) + "\n")
	toolchain.WriteString(fmt.Sprintf(`set(ENV{PATH} "${PATH_STR}%s$ENV{PATH}")`, string(os.PathListSeparator)) + "\n")

	writeIfNotEmpty := func(content, value string) {
		if value != "" {
			toolchain.WriteString(fmt.Sprintf("set(%s\"%s\")\n", content, value))
		}
	}

	toolchain.WriteString("\n# Set toolchain for cross-compile.\n")
	writeIfNotEmpty("CMAKE_C_COMPILER 		", t.CC)
	writeIfNotEmpty("CMAKE_CXX_COMPILER		", t.CXX)
	writeIfNotEmpty("CMAKE_Fortran_COMPILER	", t.FC)
	writeIfNotEmpty("CMAKE_RANLIB 			", t.RANLIB)
	writeIfNotEmpty("CMAKE_AR 				", t.AR)
	writeIfNotEmpty("CMAKE_LINKER 			", t.LD)
	writeIfNotEmpty("CMAKE_NM 				", t.NM)
	writeIfNotEmpty("CMAKE_OBJDUMP 			", t.OBJDUMP)
	writeIfNotEmpty("CMAKE_STRIP 			", t.STRIP)

	// Only linux may have sysroot.
	if t.Name == "gcc" {
		toolchain.WriteString("\n")
		toolchain.WriteString("set(CMAKE_C_FLAGS \"--sysroot=${CMAKE_SYSROOT}\")\n")
		toolchain.WriteString("set(CMAKE_CXX_FLAGS \"--sysroot=${CMAKE_SYSROOT}\")\n")
	}

	return nil
}

type WindowsKit struct {
	InstalledDir string `toml:"installed_dir"`
	Version      string `toml:"version"`
}
