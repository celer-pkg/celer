package configs

import (
	"bytes"
	"celer/context"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"path/filepath"
	"strings"
)

type RootFS struct {
	Url           string   `toml:"url"`               // Download url.
	Archive       string   `toml:"archive,omitempty"` // Archive can be changed to avoid conflict.
	Path          string   `toml:"path"`              // Runtime path of tool, it's relative path  and would be converted to absolute path later.
	PkgConfigPath []string `toml:"pkg_config_path"`
	IncludeDirs   []string `toml:"include_dirs"`
	LibDirs       []string `toml:"lib_dirs"`

	// Internal fields.
	ctx     context.Context
	abspath string
}

func (r *RootFS) Validate() error {
	// Validate rootfs download url.
	if r.Url == "" {
		return fmt.Errorf("rootfs.url is empty")
	}

	// Validate rootfs path and convert to absolute path.
	if r.Path == "" {
		return fmt.Errorf("rootfs.path is empty")
	}

	r.abspath = filepath.Join(dirs.DownloadedToolsDir, r.Path)

	return nil
}

func (r *RootFS) CheckAndRepair() error {
	// Default folder name is the first folder name of archive name.
	// but it can be specified by archive name.
	folderName := strings.Split(r.Path, string(filepath.Separator))[0]
	if r.Archive != "" {
		folderName = fileio.FileBaseName(r.Archive)
	}

	// Check and repair resource.
	archiveName := expr.If(r.Archive != "", r.Archive, filepath.Base(r.Url))
	repair := fileio.NewRepair(r.Url, archiveName, folderName, dirs.DownloadedToolsDir)
	if err := repair.CheckAndRepair(r.ctx); err != nil {
		return err
	}

	// Print download & extract info.
	location := filepath.Join(dirs.DownloadedToolsDir, folderName)
	color.Printf(color.List, "\n[✔] -- rootfs: %s\n", fileio.FileBaseName(r.Url))
	color.Printf(color.Hint, "Location: %s\n", location)

	return nil
}

func (r RootFS) GetAbsPath() string {
	return r.abspath
}

func (r RootFS) GetPkgConfigPath() []string {
	return r.PkgConfigPath
}

func (r RootFS) GetIncludeDirs() []string {
	return r.IncludeDirs
}

func (r RootFS) GetLibDirs() []string {
	return r.LibDirs
}

func (r RootFS) Generate(toolchain *strings.Builder) error {
	var buffer bytes.Buffer

	// SYSROOT section.
	fmt.Fprintf(&buffer, "\n# SYSROOT for cross-compile.\n")
	fmt.Fprintf(&buffer, "set(CMAKE_SYSROOT %q)\n", fileio.ToRelPath(r.abspath))

	// Append --sysroot to compiler flags.
	fmt.Fprintf(&buffer, `string(APPEND CMAKE_C_FLAGS_INIT " --sysroot=${CMAKE_SYSROOT}")`+"\n")
	fmt.Fprintf(&buffer, `string(APPEND CMAKE_CXX_FLAGS_INIT " --sysroot=${CMAKE_SYSROOT}")`+"\n")

	// Include directories section,
	// Note: for default include dirs like `/usr/include`, we must don't need to add them here.
	if len(r.IncludeDirs) > 0 {
		for _, incDir := range r.IncludeDirs {
			if strings.Contains(incDir, "usr/include") {
				return fmt.Errorf("usr/include should not be added to rootfs.include_dirs, " +
					"it'll cause system headers cannot be found error")
			}

			incPath := filepath.ToSlash(filepath.Join("${CMAKE_SYSROOT}", incDir))
			fmt.Fprintf(&buffer, `string(APPEND CMAKE_C_FLAGS_INIT " -isystem %s")`+"\n", incPath)
			fmt.Fprintf(&buffer, `string(APPEND CMAKE_CXX_FLAGS_INIT " -isystem %s")`+"\n", incPath)
		}
	}

	// Linker rpath-link section.
	if len(r.LibDirs) > 0 {
		fmt.Fprintf(&buffer, "\n# Linker needs rpath-link to resolve NEEDED dependencies from sysroot.\n")
		for _, libDir := range r.LibDirs {
			libPath := filepath.Join("${CMAKE_SYSROOT}", libDir)
			libPath = filepath.ToSlash(libPath)
			fmt.Fprintf(&buffer, `string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " -Wl,-rpath-link,%s")`+"\n", libPath)
			fmt.Fprintf(&buffer, `string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " -Wl,-rpath-link,%s")`+"\n", libPath)
			fmt.Fprintf(&buffer, `string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -Wl,-rpath-link,%s")`+"\n", libPath)
		}
	}

	// CMake search paths section.
	fmt.Fprintf(&buffer, "\n# Search programs in the host environment.\n")
	fmt.Fprintf(&buffer, "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)\n")
	fmt.Fprintf(&buffer, "\n# Search libraries and headers in the target environment.\n")
	fmt.Fprintf(&buffer, "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)\n")
	fmt.Fprintf(&buffer, "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)\n")
	fmt.Fprintf(&buffer, "set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)\n")

	// Write all at once.
	toolchain.WriteString(buffer.String())
	return nil
}
