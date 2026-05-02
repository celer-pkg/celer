package fileio

import (
	"fmt"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/expr"
)

func NewRepair(url, downloads, archive, folder, destDir string) *Repair {
	downloader := NewDownloader(url, downloads)
	downloader.WithArchive(archive)

	return &Repair{
		downloader: downloader,
		folder:     folder,
		destDir:    destDir,
	}
}

type Repair struct {
	ctx        context.Context
	httpClient *http.Client
	downloader *downloader
	folder     string
	destDir    string
}

func (r *Repair) CheckAndRepair(ctx context.Context) error {
	r.ctx = ctx
	r.httpClient = httpClient(r.ctx.ProxyHostPort())

	switch {
	case strings.HasPrefix(r.downloader.url, "http"), strings.HasPrefix(r.downloader.url, "ftp"):
		// Use archive name if specified, otherwise use filename from URL
		fileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(r.downloader.url))
		downloaded := filepath.Join(ctx.Downloads(), fileName)

		// For single-file tools, destDir is tools directory,
		// For archives, destDir is tools/{folder}
		var destDir string
		if r.folder == "" {
			destDir = r.destDir
		} else {
			destDir = filepath.Join(r.destDir, r.folder)
		}

		// Check if need to download.
		needToDownload, err := r.needToDownload(r.downloader.url, fileName)
		if err != nil {
			return err
		}

		// Get the actual downloaded file path (downloader.Start may rename the file)
		if needToDownload {
			actualDownloaded, err := r.downloader.Start(r.httpClient)
			if err != nil {
				return fmt.Errorf("failed to download %s -> %w", r.downloader.url, err)
			}
			downloaded = actualDownloaded
		}

		// Check if it's a single executable file (like .exe, .sh or standalone binaries).
		isSingleFile := strings.HasSuffix(downloaded, ".sh") ||
			strings.HasSuffix(downloaded, ".exe") ||
			!IsSupportedArchive(downloaded)

		// Repair resource.
		if needToDownload {
			if isSingleFile {
				// For single-file tools, just ensure it's executable.
				// (file is already downloaded to /downloads/)
				if err := os.Chmod(downloaded, os.ModePerm); err != nil {
					return fmt.Errorf("failed to chmod %s\n %w", downloaded, err)
				}
			} else {
				// For archives: remove and extract.
				if err := os.RemoveAll(destDir); err != nil {
					return err
				}

				// Extract archive file.
				if err := Extract(downloaded, destDir); err != nil {
					return fmt.Errorf("failed to extract %s -> %w", downloaded, err)
				}

				// Check if has nested folder (handling case where there's a nested folder).
				if err := moveNestedFolderIfExist(destDir); err != nil {
					return fmt.Errorf("%s: move nested folder -> %w", destDir, err)
				}
			}
		}

	case strings.HasPrefix(r.downloader.url, "file:///"):
		localPath := strings.TrimPrefix(r.downloader.url, "file:///")
		state, err := os.Stat(localPath)
		if err != nil {
			return fmt.Errorf("%s is not accessable", r.downloader.url)
		}

		// If localPath is a directory, we assume it is valid.
		if state.IsDir() {
			return nil
		}

		simpleName := Base(r.downloader.url)
		destDir := filepath.Join(r.destDir, simpleName)

		// Skip if destDir exist.
		if PathExists(destDir) {
			return nil
		}

		// Extract archive file.
		if err := Extract(localPath, destDir); err != nil {
			return fmt.Errorf("%s: extract -> %w", localPath, err)
		}

		// Check if has nested folder (handling case where there's an extra nested folder).
		if err := moveNestedFolderIfExist(destDir); err != nil {
			return fmt.Errorf("%s: move nested folder -> %w", r.folder, err)
		}

	default:
		return fmt.Errorf("%s is not accessible", r.downloader.url)
	}

	return nil
}

func (r *Repair) MoveAllToParent() error {
	toolsDir := filepath.Join(r.ctx.Downloads(), "tools")
	entries, err := os.ReadDir(filepath.Join(toolsDir, r.folder))
	if err != nil {
		return err
	}

	for _, entry := range entries {
		if err := RenameFile(entry.Name(), toolsDir); err != nil {
			return err
		}
	}

	return nil
}

func (r Repair) needToDownload(url, archive string) (needToDownload bool, err error) {
	destFilePath := filepath.Join(r.ctx.Downloads(), archive)
	if PathExists(destFilePath) {
		// Skip checking filesize and re-download.
		if r.ctx.Offline() {
			return false, nil
		}

		// Need to download if remote file size and local file size not match.
		fileSize, err := FileSize(r.httpClient, url)
		if err != nil {
			return false, fmt.Errorf("failed to get remote file size -> %w", err)
		}
		info, err := os.Stat(destFilePath)
		if err != nil {
			return false, fmt.Errorf("failed to get local file size for %s -> %w", archive, err)
		}

		// Not all remote files have size, so we need to check if file size is greater than 0.
		if fileSize > 0 && info.Size() != fileSize {
			return true, nil
		}

		return false, nil
	} else {
		// Skip downloading in offline mode.
		if r.ctx.Offline() {
			return false, fmt.Errorf("downloading has been ignored since you are currently in offline mode.")
		}

		return true, nil
	}
}
