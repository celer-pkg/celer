package configs

import (
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// GetArchiveChecksums returns SHA-256 of toolchain and rootfs archives for cache key.
// Empty string means no archive (e.g. native toolchain, local dir, or nil rootfs).
func (p *Platform) GetArchiveChecksums() (toolchainChecksum, rootfsChecksum string, err error) {
	if p.Toolchain != nil {
		toolchainChecksum, err = p.toolchainChecksum()
		if err != nil {
			return "", "", fmt.Errorf("failed to get toolchain checksum: %w", err)
		}
	}

	if p.RootFS != nil {
		rootfsChecksum, err = p.rootfsChecksum()
		if err != nil {
			return "", "", fmt.Errorf("failed to get rootfs checksum: %w", err)
		}
	}

	return toolchainChecksum, rootfsChecksum, nil
}

func (p *Platform) toolchainChecksum() (string, error) {
	toolchainPath := p.getToolchainPath()
	if toolchainPath == "" {
		return "", nil
	}

	if !fileio.PathExists(toolchainPath) {
		return "", fmt.Errorf("toolchain archive not found: %s", toolchainPath)
	}

	return fileio.CalculateChecksum(toolchainPath)
}

func (p *Platform) getToolchainPath() string {
	t := p.Toolchain
	if t == nil {
		return ""
	}

	// Native toolchain (e.g. /usr/bin).
	if t.Path == "/usr/bin" || t.Url == "file:////usr/bin" {
		return ""
	}

	// Remote archive: downloaded to downloads/
	if strings.HasPrefix(t.Url, "http") || strings.HasPrefix(t.Url, "ftp") {
		archiveName := expr.If(t.Archive != "", t.Archive, filepath.Base(t.Url))
		return filepath.Join(p.ctx.Downloads(), archiveName)
	}

	// Local file:/// with archive file.
	if after, ok := strings.CutPrefix(t.Url, "file:///"); ok {
		localPath := after
		if fileio.PathExists(localPath) {
			resolved, err := filepath.EvalSymlinks(localPath)
			if err == nil {
				localPath = resolved
			}
			info, err := os.Stat(localPath)
			if err == nil && info.IsDir() {
				return "" // local dir, no archive
			}
			if fileio.IsSupportedArchive(localPath) {
				return localPath
			}
		}
	}
	return ""
}

func (p *Platform) rootfsChecksum() (string, error) {
	rootfsPath := p.getRootfsPath()
	if rootfsPath == "" {
		return "", nil
	}

	if !fileio.PathExists(rootfsPath) {
		return "", fmt.Errorf("rootfs archive not found: %s", rootfsPath)
	}

	return fileio.CalculateChecksum(rootfsPath)
}

func (p *Platform) getRootfsPath() string {
	rootfs := p.RootFS
	if rootfs == nil {
		return ""
	}

	// Remote archive: downloaded to downloads/
	if strings.HasPrefix(rootfs.Url, "http") || strings.HasPrefix(rootfs.Url, "ftp") {
		archiveName := expr.If(rootfs.Archive != "", rootfs.Archive, filepath.Base(rootfs.Url))
		return filepath.Join(p.ctx.Downloads(), archiveName)
	}

	// Local file:/// with archive file.
	if strings.HasPrefix(rootfs.Url, "file:///") {
		localPath := strings.TrimPrefix(rootfs.Url, "file:///")
		if fileio.PathExists(localPath) {
			info, err := os.Stat(localPath)
			if err == nil && info.IsDir() {
				return "" // local dir, no archive
			}

			if fileio.IsSupportedArchive(localPath) {
				return localPath
			}
		}
	}
	return ""
}
