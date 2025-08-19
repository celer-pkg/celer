package fileio

import (
	"celer/pkgs/dirs"
	"fmt"
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
	Repaired bool

	downloader downloader
	folder     string
	destDir    string
}

func (r *Repair) CheckAndRepair() error {
	switch {
	case strings.HasPrefix(r.downloader.url, "http"), strings.HasPrefix(r.downloader.url, "ftp"):
		downloaded := filepath.Join(dirs.DownloadedDir, r.downloader.archive)
		destDir := filepath.Join(r.destDir, r.folder)

		// Download archive file if not exist.
		if !PathExists(downloaded) {
			if err := r.download(r.downloader.url, r.downloader.archive); err != nil {
				return err
			}
			if err := os.RemoveAll(destDir); err != nil {
				return err
			}
		}

		// Skip if destDir exist.
		if PathExists(destDir) {
			return nil
		}

		if strings.HasSuffix(downloaded, ".exe") {
			destFile := filepath.Join(destDir, filepath.Base(downloaded))
			if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
				return fmt.Errorf("%s: mkdir error: %w", destDir, err)
			}
			if err := CopyFile(downloaded, destFile); err != nil {
				return fmt.Errorf("%s: rename error: %w", downloaded, err)
			}
		} else {
			// Extract archive file.
			if err := Extract(downloaded, destDir); err != nil {
				return fmt.Errorf("%s: extract error: %w", downloaded, err)
			}

			// Check if has nested folder (handling case where there's an nested folder).
			if err := moveNestedFolderIfExist(destDir); err != nil {
				return fmt.Errorf("%s: move nested folder: %w", destDir, err)
			}
		}
		r.Repaired = true

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
		destDir := filepath.Join(dirs.DownloadedToolsDir, simpleName)

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

		r.Repaired = true

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

func (r Repair) download(url, archive string) (err error) {
	downloaded := filepath.Join(dirs.DownloadedDir, archive)
	if PathExists(downloaded) {
		// Redownload if remote file size and local file size not match.
		fileSize, err := FileSize(url)
		if err != nil {
			return fmt.Errorf("get remote filesize: %w", err)
		}
		info, err := os.Stat(downloaded)
		if err != nil {
			return fmt.Errorf("%s: get local filesize: %w", archive, err)
		}

		// Not every remote file has size, so we need to check if fileSize is greater than 0.
		if fileSize > 0 && info.Size() != fileSize {
			if _, err := r.downloader.Start(); err != nil {
				return fmt.Errorf("%s: download: %w", archive, err)
			}
		}
	} else {
		if _, err := r.downloader.Start(); err != nil {
			return fmt.Errorf("%s: download: %w", archive, err)
		}
	}

	return nil
}
