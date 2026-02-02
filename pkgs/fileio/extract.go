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
		strings.HasSuffix(filePath, ".tgz") ||
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

	fileName := filepath.Base(archiveFile)
	expr.PrintInline(fmt.Sprintf("\rExtracting: %s...", fileName))

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return fmt.Errorf("mkdir for extract: %w", err)
	}

	// var command string
	tarPath := expr.If(runtime.GOOS == "windows", "C:/Windows/System32/tar.exe", "/usr/bin/tar")
	var cmd *exec.Cmd

	switch {
	case strings.HasSuffix(archiveFile, ".tar.gz"),
		strings.HasSuffix(archiveFile, ".tgz"):
		cmd = exec.Command(tarPath, "-zxf", archiveFile, "-C", destDir)

	case strings.HasSuffix(archiveFile, ".tar.xz"):
		if runtime.GOOS == "windows" && os.Getenv("GITHUB_ACTIONS") == "true" {
			sevenZipPath, err := exec.LookPath("7z")
			if err != nil {
				return fmt.Errorf("7z utility not found, please install 7z for Windows")
			}
			cmd = exec.Command(sevenZipPath, "x", archiveFile, "-o"+destDir)
		} else {
			cmd = exec.Command(tarPath, "-Jxf", archiveFile, "-C", destDir)
		}

	case strings.HasSuffix(archiveFile, ".tar.bz2"):
		cmd = exec.Command(tarPath, "-xjf", archiveFile, "-C", destDir)

	case strings.HasSuffix(archiveFile, ".zip"):
		if runtime.GOOS == "windows" {
			cmd = exec.Command(tarPath, "-xf", archiveFile, "-C", destDir)
		} else {
			cmd = exec.Command("unzip", archiveFile, "-d", destDir)
		}

	case strings.HasSuffix(archiveFile, ".7z"):
		cmd = exec.Command("7z", "x", archiveFile, "-o"+destDir)

	case strings.HasSuffix(archiveFile, ".exe"):
		cmd = exec.Command(archiveFile, "-o", destDir, "-y")

	default:
		return fmt.Errorf("unsupported archive file type: %s", archiveFile)
	}

	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stdout
	cmd.Env = os.Environ()

	if err := cmd.Run(); err != nil {
		return fmt.Errorf("extract archive: %w", err)
	}

	expr.PrintInline(fmt.Sprintf("\rExtracting: %s...\n", fileName))
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
