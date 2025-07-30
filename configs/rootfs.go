package configs

import (
	"celer/pkgs/color"
	"celer/pkgs/dirs"
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

func (r RootFS) CheckAndRepair() error {
	// Default folder name is the first folder name of archive name.
	// but it can be specified by archive name.
	folderName := strings.Split(r.Path, string(filepath.Separator))[0]
	if r.Archive != "" {
		folderName = fileio.FileBaseName(r.Archive)
	}
	location := filepath.Join(dirs.DownloadedToolsDir, folderName)

	// Check if tool exists.
	if fileio.PathExists(r.fullpath) {
		return nil
	}

	// Use archive name as download file name if specified.
	archiveName := filepath.Base(r.Url)
	if r.Archive != "" {
		archiveName = r.Archive
	}

	// Check and repair resource.
	repair := fileio.NewRepair(r.Url, archiveName, folderName, dirs.DownloadedToolsDir)
	if err := repair.CheckAndRepair(); err != nil {
		return err
	}

	// Print download & extract info.
	if !DevMode {
		title := color.Sprintf(color.Green, "\n[✔] ---- Rootfs: %s\n", fileio.FileBaseName(r.Url))
		fmt.Printf("%sLocation: %s\n", title, location)
	}
	return nil
}

func (r RootFS) generate(toolchain *strings.Builder) error {
	rootfsPath := "${WORKSPACE_DIR}/" + strings.TrimPrefix(r.fullpath, dirs.WorkspaceDir+string(os.PathSeparator))
	fmt.Fprintf(toolchain, `
# Set sysroot for cross-compile.
set(CMAKE_SYSROOT "%s")
list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

# Search programs in the host environment.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search libraries and headers in the target environment.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
`, rootfsPath)
	return nil
}
