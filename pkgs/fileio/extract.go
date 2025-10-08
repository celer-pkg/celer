package fileio

import (
	"celer/pkgs/cmd"
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

	fileName := filepath.Base(archiveFile)
	expr.PrintInline(fmt.Sprintf("\rExtracting: %s...", fileName))

	var extractFailed bool
	defer func() {
		if !extractFailed {
			expr.PrintInline(fmt.Sprintf("\rExtracted: %s...", fileName))
		}
	}()

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		extractFailed = true
		return fmt.Errorf("mkdir for extract: %w", err)
	}

	// var command string
	tarPath := expr.If(runtime.GOOS == "windows", "C:/Windows/System32/tar.exe", "/usr/bin/tar")

	switch {
	case strings.HasSuffix(archiveFile, ".tar.gz"),
		strings.HasSuffix(archiveFile, ".tgz"):
		args := []string{"-zxf", archiveFile, "-C", destDir}
		executor := cmd.NewExecutor("", tarPath, args...)
		if err := executor.Execute(); err != nil {
			extractFailed = true
			return fmt.Errorf("extract: %w", err)
		}

	case strings.HasSuffix(archiveFile, ".tar.xz"):
		args := []string{"-xf", archiveFile, "-C", destDir}
		executor := cmd.NewExecutor("", tarPath, args...)
		if err := executor.Execute(); err != nil {
			extractFailed = true
			return fmt.Errorf("extract: %w", err)
		}

	case strings.HasSuffix(archiveFile, ".tar.bz2"):
		args := []string{"-xjf", archiveFile, "-C", destDir}
		executor := cmd.NewExecutor("", tarPath, args...)
		if err := executor.Execute(); err != nil {
			extractFailed = true
			return fmt.Errorf("extract: %w", err)
		}

	case strings.HasSuffix(archiveFile, ".zip"):
		// In windows, tar support extract zip file.
		if runtime.GOOS == "windows" {
			args := []string{"-xf", archiveFile, "-C", destDir}
			executor := cmd.NewExecutor("", tarPath, args...)
			if err := executor.Execute(); err != nil {
				extractFailed = true
				return fmt.Errorf("extract: %w", err)
			}
		} else {
			args := []string{archiveFile, "-d", destDir}
			executor := cmd.NewExecutor("", "unzip", args...)
			if err := executor.Execute(); err != nil {
				extractFailed = true
				return fmt.Errorf("extract: %w", err)
			}
		}

	case strings.HasSuffix(archiveFile, ".7z"):
		args := []string{"x", archiveFile, "-o", destDir}
		executor := cmd.NewExecutor("", "7z", args...)
		if err := executor.Execute(); err != nil {
			extractFailed = true
			return fmt.Errorf("extract: %w", err)
		}

	case strings.HasSuffix(archiveFile, ".exe"):
		args := []string{archiveFile, "-o", destDir, "-y"}
		executor := cmd.NewExecutor("", archiveFile, args...)
		if err := executor.Execute(); err != nil {
			extractFailed = true
			return fmt.Errorf("extract: %w", err)
		}

	default:
		extractFailed = true
		return fmt.Errorf("unsupported archive file type: %s", archiveFile)
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
