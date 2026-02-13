//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package configs

import (
	"celer/buildsystems"
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/env"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

func (t *Toolchain) Validate() error {
	// Validate toolchain download url.
	if t.Url == "" {
		return fmt.Errorf("toolchain.url would be http url or local file url, but it's empty")
	}

	if t.Url == "file:////usr/bin" {
		t.displayName = t.Name
		t.rootDir = "/usr/bin"
	} else {
		t.displayName = fileio.FileBaseName(t.Url)
	}

	// Validate toolchain.name.
	if strings.TrimSpace(t.Name) == "" {
		return fmt.Errorf("toolchain.name is empty")
	}
	t.Name = strings.ToLower(t.Name)
	if t.Name != "gcc" && t.Name != "clang" && t.Name != "clang-cl" {
		return fmt.Errorf("toolchain.name should be 'gcc', 'clang' or 'clang-cl'")
	}

	// Validate toolchain.system_name.
	if strings.TrimSpace(t.SystemName) == "" {
		return fmt.Errorf("toolchain.system_name is empty")
	}
	t.SystemName = expr.UpperFirst(t.SystemName)

	// Validate toolchain.system_processor.
	if strings.TrimSpace(t.SystemProcessor) == "" {
		return fmt.Errorf("toolchain.system_processor is empty")
	}

	// Validate toolchain.crosstool_prefix path and convert to absolute path.
	if strings.TrimSpace(t.CrosstoolPrefix) == "" {
		return fmt.Errorf("toolchain.crosstool_prefix should be like 'x86_64-linux-gnu-', but it's empty")
	}

	// Validate toolchain.c_standard.
	if strings.TrimSpace(t.CStandard) != "" && !slices.Contains(buildsystems.CStandards, t.CStandard) {
		return fmt.Errorf("toolchain.c_standard should be one of %s", strings.Join(buildsystems.CStandards, ", "))
	}

	// Validate toolchain.cxx_standard.
	if strings.TrimSpace(t.CXXStandard) != "" && !slices.Contains(buildsystems.CXXStandards, t.CXXStandard) {
		return fmt.Errorf("toolchain.cxx_standard should be one of %s", strings.Join(buildsystems.CXXStandards, ", "))
	}

	// Validate toolchain.host.
	if strings.TrimSpace(t.Host) == "" {
		return fmt.Errorf("toolchain.host should be like 'x86_64-linux-gnu', but it's empty")
	}

	// Validate toolchain.cc.
	if strings.TrimSpace(t.CC) == "" {
		return fmt.Errorf("toolchain.cc is empty")
	}

	// Validate toolchain.cxx.
	if strings.TrimSpace(t.CXX) == "" {
		return fmt.Errorf("toolchain.cxx is empty")
	}

	switch {
	// Web resource file would be extracted to specified path, so path can not be empty.
	case strings.HasPrefix(t.Url, "http"), strings.HasPrefix(t.Url, "ftp"):
		if t.Path == "" {
			return fmt.Errorf("toolchain.path is empty")
		}

		firstSection := strings.Split(filepath.ToSlash(t.Path), "/")[0]
		t.rootDir = filepath.Join(dirs.DownloadedToolsDir, firstSection)
		t.abspath = filepath.Join(dirs.DownloadedToolsDir, t.Path)
		os.Setenv("PATH", env.JoinPaths("PATH", t.abspath))

	case strings.HasPrefix(t.Url, "file:///"):
		localPath := strings.TrimPrefix(t.Url, "file:///")
		state, err := os.Stat(localPath)
		if err != nil {
			return fmt.Errorf("toolchain.url of %s is not exist", t.Url)
		}

		if state.IsDir() {
			t.abspath = localPath
			os.Setenv("PATH", env.JoinPaths("PATH", t.abspath))
		} else {
			// Even local must be a archive file and path should not be empty.
			if t.Path == "" {
				return fmt.Errorf("toolchain.path is empty")
			}

			// Check if celer supported archive file.
			if !fileio.IsSupportedArchive(localPath) {
				return fmt.Errorf("toolchain.path of %s is not a archive file", t.Url)
			}

			firstSection := strings.Split(filepath.ToSlash(t.Path), "/")[0]
			t.rootDir = filepath.Join(dirs.DownloadedToolsDir, firstSection)
			t.abspath = filepath.Join(dirs.DownloadedToolsDir, t.Path)
			os.Setenv("PATH", env.JoinPaths("PATH", t.abspath))
		}

	default:
		return fmt.Errorf("toolchain.url of %s is not exist", t.Url)
	}

	return nil
}

func (t *Toolchain) CheckAndRepair(silent bool) error {
	// Default folder name is the first folder name of archive name.
	// but it can be specified by archive name.
	folderName := strings.Split(t.Path, string(filepath.Separator))[0]
	if t.Archive != "" {
		folderName = fileio.FileBaseName(t.Archive)
	}

	// Check and repair resource.
	archiveName := expr.If(t.Archive != "", t.Archive, filepath.Base(t.Url))
	repair := fileio.NewRepair(t.Url, archiveName, folderName, dirs.DownloadedToolsDir)
	if err := repair.CheckAndRepair(t.ctx); err != nil {
		return err
	}

	if !silent {
		// Print download & extract info.
		if t.rootDir == "" {
			color.Printf(color.List, "\n[✔] -- toolchain: %s\n", "local")
			color.Printf(color.Hint, "Location: %s\n", strings.ReplaceAll(t.Url, "file:///", ""))
		} else {
			color.Printf(color.List, "\n[✔] -- toolchain: %s\n", t.displayName)
			color.Printf(color.Hint, "Location: %s\n", t.rootDir)
		}
	}

	return nil
}

// Detect detect local installed gcc.
func (t *Toolchain) Detect(platformName string) error {
	if platformName == "" {
		if err := buildtools.CheckTools(t.ctx, platformName); err != nil {
			return err
		}
	}

	t.Url = "file:////usr/bin"
	t.Path = "/usr/bin"
	t.Name = "gcc"
	t.SystemName = "Linux"
	t.SystemProcessor = "x86_64"
	t.Host = "x86_64-linux-gnu"
	t.CrosstoolPrefix = "x86_64-linux-gnu-"
	t.CC = "x86_64-linux-gnu-gcc"
	t.CXX = "x86_64-linux-gnu-g++"
	t.RANLIB = "x86_64-linux-gnu-gcc-ranlib"
	t.AR = "x86_64-linux-gnu-gcc-ar"
	t.LD = "x86_64-linux-gnu-ld"
	t.NM = "x86_64-linux-gnu-nm"
	t.OBJDUMP = "x86_64-linux-gnu-objdump"
	t.STRIP = "x86_64-linux-gnu-strip"

	if err := t.Validate(); err != nil {
		return err
	}

	if err := t.CheckAndRepair(true); err != nil {
		return err
	}

	return nil
}

// Detect no msvc in linux.
func (w *WindowsKit) Detect(msvc *context.MSVC) error {
	return nil
}
