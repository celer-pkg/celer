package fileio

import (
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"fmt"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

func NewRepair(url, archive, folder, destDir string) *Repair {
	downloader := downloader{
		url:     url,
		archive: archive,
	}

	return &Repair{
		downloader: downloader,
		folder:     folder,
		destDir:    destDir,
	}
}

type Repair struct {
	ctx        context.Context
	httpClient *http.Client
	downloader downloader
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
		downloaded := filepath.Join(dirs.DownloadedDir, fileName)
		destDir := filepath.Join(r.destDir, r.folder)

		// Check if need to download.
		needToDownload, err := r.needToDownload(r.downloader.url, fileName)
		if err != nil {
			return err
		}

		// Get the actual downloaded file path (downloader.Start may rename the file)
		if needToDownload {
			actualDownloaded, err := r.downloader.Start(r.httpClient)
			if err != nil {
				return fmt.Errorf("failed to download %s: %w", r.downloader.url, err)
			}
			downloaded = actualDownloaded
		}

		// Check if it's a single executable file (like .exe or standalone binaries).
		isSingleFile := strings.HasSuffix(downloaded, ".exe") || !IsSupportedArchive(downloaded)

		// Determine if repair is needed.
		var needToRepair bool
		if isSingleFile {
			// For single files, check if the target file with the specified name exists,
			// Use archive name if specified, otherwise use downloaded file name.
			destFileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(downloaded))
			destFile := filepath.Join(destDir, destFileName)
			needToRepair = needToDownload || !PathExists(destFile)
		} else {
			// For archives, check if the destination directory exists.
			needToRepair = needToDownload || !PathExists(destDir)
		}

		// Repair resource.
		if needToRepair {
			// Remove for overwrite.
			if err := os.RemoveAll(destDir); err != nil {
				return err
			}

			if isSingleFile {
				// Create directory: downloads/tools/{name}/
				if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
					return fmt.Errorf("failed to mkdir %s\n %w", destDir, err)
				}

				// Determine the destination file name:
				// Use archive name if specified, otherwise use downloaded file name.
				destFileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(downloaded))
				destFile := filepath.Join(destDir, destFileName)

				// Copy file with the specified name.
				if err := CopyFile(downloaded, destFile); err != nil {
					return fmt.Errorf("failed to copy %s to %s\n %w", downloaded, destFile, err)
				}

				// Make sure the file is executable.
				if err := os.Chmod(destFile, os.ModePerm); err != nil {
					return fmt.Errorf("failed to chmod %s\n %w", destFile, err)
				}
			} else {
				// Extract archive file.
				if err := Extract(downloaded, destDir); err != nil {
					return fmt.Errorf("failed to extract %s.\n %w", downloaded, err)
				}

				// Check if has nested folder (handling case where there's a nested folder).
				if err := moveNestedFolderIfExist(destDir); err != nil {
					return fmt.Errorf("%s: move nested folder: %w", destDir, err)
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

		simpleName := FileBaseName(r.downloader.url)
		destDir := filepath.Join(r.destDir, simpleName)

		// Skip if destDir exist.
		if PathExists(destDir) {
			return nil
		}

		// Extract archive file.
		if err := Extract(localPath, destDir); err != nil {
			return fmt.Errorf("%s: extract: %w", localPath, err)
		}

		// Check if has nested folder (handling case where there's an extra nested folder).
		if err := moveNestedFolderIfExist(destDir); err != nil {
			return fmt.Errorf("%s: move nested folder: %w", r.folder, err)
		}

	default:
		return fmt.Errorf("%s is not accessible", r.downloader.url)
	}

	return nil
}

func (r *Repair) MoveAllToParent() error {
	entries, err := os.ReadDir(filepath.Join(dirs.DownloadedToolsDir, r.folder))
	if err != nil {
		return err
	}

	for _, entry := range entries {
		if err := RenameFile(entry.Name(), dirs.DownloadedToolsDir); err != nil {
			return err
		}
	}

	return nil
}

func (r Repair) needToDownload(url, archive string) (needToDownload bool, err error) {
	destFilePath := filepath.Join(dirs.DownloadedDir, archive)
	if PathExists(destFilePath) {
		// Skip checking filesize and re-download.
		if r.ctx.Offline() {
			return false, nil
		}

		// Need to download if remote file size and local file size not match.
		fileSize, err := FileSize(r.httpClient, url)
		if err != nil {
			return false, fmt.Errorf("get remote file size: %w", err)
		}
		info, err := os.Stat(destFilePath)
		if err != nil {
			return false, fmt.Errorf("%s: get local file size: %w", archive, err)
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
