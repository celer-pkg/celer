package configs

import (
	"bytes"
	"celer/context"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
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
	fullpath string
	ctx      context.Context
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

	r.fullpath = filepath.Join(dirs.DownloadedToolsDir, r.Path)

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

func (r RootFS) GetFullPath() string {
	return r.fullpath
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
	rootfsPath := "${CELER_ROOT}/" + strings.TrimPrefix(r.fullpath, dirs.WorkspaceDir+string(os.PathSeparator))

	var buffer bytes.Buffer

	// SYSROOT section.
	fmt.Fprintf(&buffer, "\n# SYSROOT for cross-compile.\n")
	fmt.Fprintf(&buffer, "set(%-22s%q)\n", "CMAKE_SYSROOT", rootfsPath)

	var cFlags, cxxFlags, sharedLdFlags, moduleLdFlags, exeLdFlags []string
	cFlags = append(cFlags, "--sysroot=${CMAKE_SYSROOT}")
	cxxFlags = append(cxxFlags, "--sysroot=${CMAKE_SYSROOT}")

	// Include directories section,
	// Note: for default include dirs like `/usr/include`, we must don't need to add them here.
	if len(r.IncludeDirs) > 0 {
		for _, incDir := range r.IncludeDirs {
			if strings.Contains(incDir, "usr/include") {
				return fmt.Errorf("usr/include should not be added to rootfs.include_dirs, " +
					"it'll cause system headers cannot be found error")
			}

			incPath := filepath.Join("${CMAKE_SYSROOT}", incDir)
			incPath = filepath.ToSlash(incPath)
			cFlags = append(cFlags, "-isystem "+incPath)
			cxxFlags = append(cxxFlags, "-isystem "+incPath)
		}
	}

	// Linker rpath-link section.
	if len(r.LibDirs) > 0 {
		for _, libDir := range r.LibDirs {
			libPath := filepath.Join("${CMAKE_SYSROOT}", libDir)
			libPath = filepath.ToSlash(libPath)
			sharedLdFlags = append(sharedLdFlags, "-Wl,-rpath-link="+libPath)
			moduleLdFlags = append(moduleLdFlags, "-Wl,-rpath-link="+libPath)
			exeLdFlags = append(exeLdFlags, "-Wl,-rpath-link="+libPath)
		}
	}

	// Write compiler flags.
	fmt.Fprintf(&buffer, "set(%-22s%q)\n", "CMAKE_C_FLAGS_INIT", strings.Join(cFlags, " "))
	fmt.Fprintf(&buffer, "set(%-22s%q)\n", "CMAKE_CXX_FLAGS_INIT", strings.Join(cxxFlags, " "))

	// Write linker flags if any.
	if len(sharedLdFlags) > 0 {
		fmt.Fprintf(&buffer, "\n# Linker needs rpath-link to resolve NEEDED dependencies from sysroot.\n")
		fmt.Fprintf(&buffer, "set(%-32s%q)\n", "CMAKE_SHARED_LINKER_FLAGS_INIT", strings.Join(sharedLdFlags, " "))
		fmt.Fprintf(&buffer, "set(%-32s%q)\n", "CMAKE_MODULE_LINKER_FLAGS_INIT", strings.Join(moduleLdFlags, " "))
		fmt.Fprintf(&buffer, "set(%-32s%q)\n", "CMAKE_EXE_LINKER_FLAGS_INIT", strings.Join(exeLdFlags, " "))
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
