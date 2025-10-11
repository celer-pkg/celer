//go:build windows

package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/env"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"

	"golang.org/x/sys/windows/registry"
)

func (t *Toolchain) Validate() error {
	// Validate toolchain download url.
	if t.Url == "" {
		return fmt.Errorf("toolchain.url would be http url or local file url, but it's empty")
	}

	// Guess toolchain name.
	if strings.Contains(t.Url, "Microsoft Visual Studio") {
		t.displayName = "Microsoft Visual Studio"
	} else {
		t.displayName = fileio.FileBaseName(t.Url)
	}

	// Validate toolchain.name.
	if strings.TrimSpace(t.Name) == "" {
		return fmt.Errorf("toolchain.name is empty")
	}
	t.Name = strings.ToLower(t.Name)
	if t.Name != "gcc" && t.Name != "msvc" && t.Name != "clang" {
		return fmt.Errorf("toolchain.name should be 'gcc', 'msvc' or 'clang'")
	}

	// Validate toolchain.system_name.
	if strings.TrimSpace(t.SystemName) == "" {
		return fmt.Errorf("toolchain.system_name is empty")
	}

	// Validate toolchain.system_processor.
	if strings.TrimSpace(t.SystemProcessor) == "" {
		return fmt.Errorf("toolchain.system_processor is empty")
	}

	// Validate toolchain prefix path and convert to absolute path.
	if t.Name != "msvc" && t.CrosstoolPrefix == "" {
		return fmt.Errorf("toolchain.crosstool_prefix should be like 'x86_64-linux-gnu-', but it's empty")
	}

	// Validate toolchain.host.
	if strings.TrimSpace(t.Host) == "" {
		t.Host = "x86_64-w64-mingw32"
	}

	// Validate toolchain.cc.
	if strings.TrimSpace(t.CC) == "" {
		if t.Name == "msvc" {
			t.CC = "cl.exe"
		} else {
			return fmt.Errorf("toolchain.cc is empty")
		}
	}

	// Validate toolchain.cxx.
	if strings.TrimSpace(t.CXX) == "" {
		if t.Name == "msvc" {
			t.CXX = "cl.exe"
		} else {
			return fmt.Errorf("toolchain.cxx is empty")
		}
	}

	if strings.TrimSpace(t.LD) == "" {
		if t.Name == "msvc" {
			t.LD = "link.exe"
		}
	}

	if strings.TrimSpace(t.AR) == "" {
		if t.Name == "msvc" {
			t.AR = "lib.exe"
		}
	}

	switch {
	// Web resource file would be extracted to specified path, so path can not be empty.
	case strings.HasPrefix(t.Url, "http"), strings.HasPrefix(t.Url, "ftp"):
		if t.Path == "" {
			return fmt.Errorf("toolchain.path is empty")
		}

		firstSection := strings.Split(filepath.ToSlash(t.Path), "/")[0]
		t.rootDir = filepath.Join(dirs.DownloadedToolsDir, firstSection)
		t.fullpath = filepath.Join(dirs.DownloadedToolsDir, t.Path)
		os.Setenv("PATH", env.JoinPaths("PATH", t.fullpath))

	case strings.HasPrefix(t.Url, "file:///"):
		localPath := strings.TrimPrefix(t.Url, "file:///")
		state, err := os.Stat(localPath)
		if err != nil {
			return fmt.Errorf("toolchain.url of %s is not exist", t.Url)
		}

		if state.IsDir() {
			if t.Name == "msvc" {
				localPath = strings.ReplaceAll(localPath, `/`, `\`)
				t.rootDir = localPath

				// Runtime paths.
				t.fullpath = fmt.Sprintf(`%s\VC\Tools\MSVC\%s\bin\Host%s\x64`, localPath, t.Version, t.arch())
			}
			os.Setenv("PATH", env.JoinPaths("PATH", t.fullpath))
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
			t.fullpath = filepath.Join(dirs.DownloadedToolsDir, t.Path)
			os.Setenv("PATH", env.JoinPaths("PATH", t.fullpath))
		}

	default:
		return fmt.Errorf("toolchain.url of %s is not exist", t.Url)
	}

	return nil
}

func (t *Toolchain) CheckAndRepair(silent bool) error {
	// Default folder name is the first folder name of archive name,
	// but it can be specified by archive name.
	folderName := strings.Split(t.Path, string(filepath.Separator))[0]
	if t.Archive != "" {
		folderName = fileio.FileBaseName(t.Archive)
	}

	// Use archive name as download file name if specified.
	archive := expr.If(t.Archive != "", t.Archive, filepath.Base(t.Url))

	// Check and repair resource.
	repair := fileio.NewRepair(t.Url, archive, folderName, dirs.DownloadedToolsDir)
	if err := repair.CheckAndRepair(t.ctx); err != nil {
		return err
	}

	// Print download & extract info.
	title := color.Sprintf(color.Green, "\n[✔] ---- Toolchain: %s\n", t.displayName)
	fmt.Printf("%sLocation: %s\n", title, t.rootDir)
	return nil
}

// Detect detect local installed MSVC.
func (t *Toolchain) Detect() error {
	if err := buildtools.CheckTools(t.ctx, "vswhere"); err != nil {
		return fmt.Errorf("vswhere is not available: %w", err)
	}

	// Query all available msvc installation paths.
	command := "vswhere -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath"
	exector := cmd.NewExecutor("", command)
	output, err := exector.ExecuteOutput()
	if err != nil {
		return err
	}

	// Trim the output.
	msvcDir := strings.TrimSpace(output)
	if msvcDir == "" {
		return fmt.Errorf("msvc not found, please install msvc first")
	}

	t.Url = "file:///" + msvcDir
	t.Name = "msvc"
	t.SystemName = "Windows"
	t.SystemProcessor = "x86_64"
	t.Host = "x86_64-w64-mingw32"

	version, err := t.findLatestMSVCVersion(filepath.Join(msvcDir, "VC", "Tools", "MSVC"))
	if err != nil {
		return err
	}
	t.Version = version

	if err := t.Validate(); err != nil {
		return err
	}

	if err := t.CheckAndRepair(true); err != nil {
		return err
	}

	return nil
}

func (Toolchain) arch() string {
	arch := runtime.GOARCH

	switch arch {
	case "amd64", "arm64":
		return "x64"
	case "386", "arm":
		return "x86"
	default:
		panic("unsupported arch: " + arch)
	}
}

func (t Toolchain) findLatestMSVCVersion(msvcDir string) (string, error) {
	entries, err := os.ReadDir(msvcDir)
	if err != nil {
		return "", err
	}

	var latest string
	for _, entry := range entries {
		if entry.IsDir() {
			name := entry.Name()
			if latest == "" || t.compareVersion(name, latest) > 0 {
				latest = name
			}
		}
	}
	if latest == "" {
		return "", fmt.Errorf("no MSVC versions found in %s", msvcDir)
	}
	return latest, nil
}

func (t Toolchain) compareVersion(first, second string) int {
	firstVersion := strings.Split(first, ".")
	secondVersion := strings.Split(second, ".")
	for i := 0; i < len(firstVersion) && i < len(secondVersion); i++ {
		firstInt, _ := strconv.Atoi(firstVersion[i])
		secondInt, _ := strconv.Atoi(secondVersion[i])
		if firstInt != secondInt {
			return firstInt - secondInt
		}
	}
	return len(firstVersion) - len(secondVersion)
}

func (w *WindowsKit) Detect(msvc *context.MSVC) error {
	// Check if installed.
	key, err := registry.OpenKey(registry.LOCAL_MACHINE,
		`SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0`,
		registry.QUERY_VALUE,
	)
	if err != nil {
		return fmt.Errorf("microsoft sdk v10.0 not found, make sure you have installed it")
	}
	defer key.Close()

	// Read installed dir.
	w.InstalledDir, _, err = key.GetStringValue("InstallationFolder")
	if err != nil {
		return fmt.Errorf("microsoft sdk v10.0 not found, make sure you have installed it")
	}

	// Read current version.
	version, _, err := key.GetStringValue("ProductVersion")
	if err != nil {
		return fmt.Errorf("cannot read current verson of microsoft sdk v10.0")
	}
	w.Version = w.normalizeVersion(version)

	binDir := filepath.Join(w.InstalledDir, "bin", w.Version, "x64")
	msvc.MT = filepath.Join(binDir, "mt.exe")
	msvc.RC = filepath.Join(binDir, "rc.exe")

	// Append path.
	os.Setenv("PATH", env.JoinPaths("PATH",
		filepath.Join(w.InstalledDir, "bin", w.Version, "x64"),
		filepath.Join(w.InstalledDir, "bin", "x64"),
		filepath.Join(w.InstalledDir, "Windows Performance Toolkit"),
	))

	return nil
}

// append ".0" as suffix.
func (w WindowsKit) normalizeVersion(version string) string {
	if strings.Count(version, ".") == 2 {
		return version + ".0"
	}
	return version
}
