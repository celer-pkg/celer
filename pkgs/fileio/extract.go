package fileio

import (
	"celer/pkgs/expr"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

func IsSupportedArchive(filePath string) bool {
	return strings.HasSuffix(filePath, ".tar.gz") ||
		strings.HasSuffix(filePath, ".tar.xz") ||
		strings.HasSuffix(filePath, ".tar.bz2") ||
		strings.HasSuffix(filePath, ".zip") ||
		strings.HasSuffix(filePath, ".7z")
}

func Extract(archiveFile, destDir string) error {
	// Create build dir if not exists.
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return err
	}

	// Exactly specify tar bin in different os.
	var tarPath string
	if runtime.GOOS == "windows" {
		tarPath = "C:/Windows/System32/tar.exe"
	} else {
		tarPath = "/usr/bin/tar"
	}

	var extractFailed bool

	fileName := filepath.Base(archiveFile)
	expr.PrintInline(fmt.Sprintf("\rExtracting: %s...", fileName))
	defer func() {
		if !extractFailed {
			expr.PrintInline(fmt.Sprintf("\rExtracted: %s...", fileName))
		}
	}()

	var command string

	switch {
	case strings.HasSuffix(archiveFile, ".tar.gz"),
		strings.HasSuffix(archiveFile, ".tgz"):
		command = fmt.Sprintf("%s -zxf %s -C %s", tarPath, archiveFile, destDir)

	case strings.HasSuffix(archiveFile, ".tar.xz"):
		command = fmt.Sprintf("%s -xf %s -C %s", tarPath, archiveFile, destDir)

	case strings.HasSuffix(archiveFile, ".tar.bz2"):
		command = fmt.Sprintf("%s -xjf %s -C %s", tarPath, archiveFile, destDir)

	case strings.HasSuffix(archiveFile, ".zip"):
		// In windows, tar support extract zip file.
		if runtime.GOOS == "windows" {
			command = fmt.Sprintf("C:/Windows/System32/tar.exe -xf %s -C %s", archiveFile, destDir)
		} else {
			command = fmt.Sprintf("unzip %s -d %s", archiveFile, destDir)
		}

	case strings.HasSuffix(archiveFile, ".7z"):
		command = fmt.Sprintf("7z x %s -o %s", archiveFile, destDir)

	case strings.HasSuffix(archiveFile, ".exe"):
		command = fmt.Sprintf("%s -o %s -y", archiveFile, destDir)

	default:
		extractFailed = true
		return fmt.Errorf("unsupported archive file type: %s", archiveFile)
	}

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		extractFailed = true
		return fmt.Errorf("mkdir for extract: %w", err)
	}

	// Run command.
	var cmd *exec.Cmd
	if runtime.GOOS == "windows" {
		cmd = exec.Command("cmd", "/c", command)
	} else {
		cmd = exec.Command("bash", "-c", command)
	}

	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stdout
	cmd.Env = os.Environ()

	if err := cmd.Run(); err != nil {
		extractFailed = true
		return fmt.Errorf("extract: %w", err)
	}

	return nil
}

// Targz creates a tarball from srcDir and saves it to archivePath.
func Targz(archivePath, srcDir string, includeFolder bool) error {
	// Exactly specify tar bin in different os.
	var tarPath string
	if runtime.GOOS == "windows" {
		tarPath = "C:/Windows/System32/tar.exe"
	} else {
		tarPath = "/usr/bin/tar"
	}

	var cmd *exec.Cmd
	var command string

	if includeFolder {
		command = fmt.Sprintf("%s -cvzf %s %s", tarPath, archivePath, srcDir)
	} else {
		command = fmt.Sprintf("%s -cvzf %s -C %s .", tarPath, archivePath, srcDir)
	}

	if runtime.GOOS == "windows" {
		cmd = exec.Command("cmd", "/c", command)
	} else {
		cmd = exec.Command("bash", "-c", command)
	}

	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stdout
	cmd.Env = os.Environ()

	if err := cmd.Run(); err != nil {
		return fmt.Errorf("create tarball: %w", err)
	}

	return nil
}
