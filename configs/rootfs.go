package configs

import (
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
	title := color.Sprintf(color.Green, "\n[✔] ---- Rootfs: %s\n", fileio.FileBaseName(r.Url))
	fmt.Printf("%sLocation: %s\n", title, location)

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
	fmt.Fprintf(toolchain, `
# SYSROOT for cross-compile.
set(CMAKE_SYSROOT 	"%s")
set(CMAKE_C_FLAGS   "--sysroot=${CMAKE_SYSROOT} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "--sysroot=${CMAKE_SYSROOT} ${CMAKE_CXX_FLAGS}")

# Search programs in the host environment.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search libraries and headers in the target environment.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
`, rootfsPath)
	return nil
}
