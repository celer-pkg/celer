package fileio

import (
	"fmt"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/expr"
)

// checkedFiles tracks files that have already passed CheckAndRepair,
// keyed by downloaded file path, to avoid repeated SHA256 verification.
var checkedFiles sync.Map

type Repair struct {
	ctx        context.Context
	httpClient *http.Client
	downloader *downloader
	folder     string
	destDir    string
	sha256     string
}

func NewRepair(url, downloads, archive, folder, destDir, sha256 string) *Repair {
	downloader := NewDownloader(url, downloads)
	downloader.WithArchive(archive)

	return &Repair{
		downloader: downloader,
		folder:     folder,
		destDir:    destDir,
		sha256:     sha256,
	}
}

func (r *Repair) CheckAndRepair(ctx context.Context) error {
	// Skip if this file has already been checked and repaired.
	checkedKey := r.fileCheckedKey()
	if _, loaded := checkedFiles.LoadOrStore(checkedKey, true); loaded {
		// Even if cached, verify destination exists (it may have been removed externally).
		if r.folder == "" {
			return nil
		}
		destDir := filepath.Join(r.destDir, r.folder)
		if PathExists(destDir) {
			return nil
		}
		checkedFiles.Delete(checkedKey)
	}

	r.ctx = ctx
	r.httpClient = httpClient(r.ctx.ProxyHostPort())

	switch {
	case strings.HasPrefix(r.downloader.url, "http"), strings.HasPrefix(r.downloader.url, "ftp"):
		return r.handleRemoteURL(ctx)

	case strings.HasPrefix(r.downloader.url, "file:///"):
		return r.handleLocalFile()

	default:
		return fmt.Errorf("%s is not accessible", r.downloader.url)
	}
}

// handleRemoteURL processes HTTP/FTP downloads with cache support.
func (r *Repair) handleRemoteURL(ctx context.Context) error {
	fileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(r.downloader.url))
	downloaded := filepath.Join(ctx.Downloads(), fileName)

	// For single-file tools (folder is empty), destDir is not used.
	// For archive tools (folder is not empty), destDir is the target extraction directory.
	var destDir string
	if r.folder == "" {
		destDir = ctx.Downloads() // Not used for single-file tools.
	} else {
		destDir = filepath.Join(r.destDir, r.folder)
	}

	// Check if local file is valid. If not, try to restore from cache or download again.
	pkgCacheConfig := ctx.PkgCacheConfig()
	cachedDownloadsDir := ""
	canUseCache := r.sha256 != "" && !r.ctx.Offline() && pkgCacheConfig != nil && pkgCacheConfig.IsWritable()
	if canUseCache {
		cachedDownloadsDir = pkgCacheConfig.GetDir(context.PkgCacheDirDownloads)
	}

	// Determine if download is needed.
	needToDownload, err := r.needToDownload(fileName, r.sha256)
	if err != nil {
		return err
	}

	// Try restore from cache if local file is invalid.
	if needToDownload && canUseCache {
		cachedFile, err := r.tryRestoreFromCache(cachedDownloadsDir, fileName)
		if err != nil {
			color.Printf(color.Warning, "✘ failed to search pkgcache: %v\n", err)
		} else if cachedFile != "" {
			if err := os.MkdirAll(ctx.Downloads(), os.ModePerm); err != nil {
				return fmt.Errorf("failed to mkdir downloads dir -> %w", err)
			}
			if err := CopyFile(cachedFile, downloaded); err != nil {
				return fmt.Errorf("failed to restore cached file %s -> %w", fileName, err)
			}
			color.Printf(color.Hint, "✔ restore from pkgcache: %s\n", fileName)
			needToDownload = false
		}
	}

	// Download if still needed.
	if needToDownload {
		actualDownloaded, err := r.downloader.Start(r.httpClient)
		if err != nil {
			return fmt.Errorf("failed to download %s -> %w", r.downloader.url, err)
		}
		downloaded = actualDownloaded

		// Verify and cache after download.
		if canUseCache {
			if !verifySHA256(downloaded, r.sha256) {
				return fmt.Errorf("sha-256 mismatch for %s: expected %s", fileName, r.sha256)
			}

			color.Printf(color.Hint, "- caching to pkgcache: %s", fileName)
			chattrFS := NewChattrFS(pkgCacheConfig.GetDir(context.PkgCacheDirRoot))
			cachedPath, err := SaveCachedFile(downloaded, cachedDownloadsDir, fileName, r.sha256, chattrFS)
			if err != nil {
				return fmt.Errorf("failed to cache downloaded file %s -> %w", fileName, err)
			}
			color.PrintInline(color.Hint, "✔ cached to pkgcache: %s\n", fileName)
			downloaded = cachedPath
		}
	}

	// Extract/deploy to destination.
	return r.deployToDestination(downloaded, destDir, needToDownload)
}

// deployToDestination handles extraction or copying to the final destination.
func (r *Repair) deployToDestination(downloaded, destDir string, needToDownload bool) error {
	// Single-file tools (folder is empty) don't need deployment
	if r.folder == "" {
		return nil
	}

	isSingleFile := strings.HasSuffix(downloaded, ".exe") || !IsSupportedArchive(downloaded)

	// Determine if deployment is needed
	needsDeployment := needToDownload
	if !needsDeployment {
		if isSingleFile {
			destFileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(downloaded))
			needsDeployment = !PathExists(filepath.Join(destDir, destFileName))
		} else {
			needsDeployment = !PathExists(destDir)
		}
	}

	if !needsDeployment {
		return nil
	}

	// Remove old destination
	if err := os.RemoveAll(destDir); err != nil {
		return err
	}

	if isSingleFile {
		return r.deploySingleFile(downloaded, destDir)
	}
	return r.deployArchive(downloaded, destDir)
}

// deploySingleFile copies a single file to destination.
func (r *Repair) deploySingleFile(downloaded, destDir string) error {
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to mkdir %s -> %w", destDir, err)
	}

	destFileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(downloaded))
	destFile := filepath.Join(destDir, destFileName)

	if err := CopyFile(downloaded, destFile); err != nil {
		return fmt.Errorf("failed to copy %s to %s -> %w", downloaded, destFile, err)
	}

	return nil
}

// deployArchive extracts archive to destination.
func (r *Repair) deployArchive(downloaded, destDir string) error {
	if err := Extract(downloaded, destDir); err != nil {
		return fmt.Errorf("failed to extract %s -> %w", downloaded, err)
	}

	if err := moveNestedFolderIfExist(destDir); err != nil {
		return fmt.Errorf("failed to move nested folder in %s -> %w", destDir, err)
	}

	return nil
}

// handleLocalFile processes file:/// URLs.
func (r *Repair) handleLocalFile() error {
	localPath := strings.TrimPrefix(r.downloader.url, "file:///")
	state, err := os.Stat(localPath)
	if err != nil {
		return fmt.Errorf("%s is not accessible -> %w", r.downloader.url, err)
	}

	if state.IsDir() {
		return nil
	}

	simpleName := Base(r.downloader.url)
	destDir := filepath.Join(r.destDir, simpleName)

	if PathExists(destDir) {
		return nil
	}

	return r.deployArchive(localPath, destDir)
}

// fileCheckedKey returns a unique key for file checked based on the download URL and destination.
func (r Repair) fileCheckedKey() string {
	fileName := expr.If(r.downloader.archive != "", r.downloader.archive, filepath.Base(r.downloader.url))
	return r.downloader.url + "|" + r.destDir + "|" + fileName
}

func (r Repair) needToDownload(archive, sha256 string) (needToDownload bool, err error) {
	destFilePath := filepath.Join(r.ctx.Downloads(), archive)
	if !PathExists(destFilePath) {
		// Skip downloading in offline mode.
		if r.ctx.Offline() {
			return false, fmt.Errorf("downloading has been ignored since you are currently in offline mode.")
		}
		return true, nil
	}

	// Skip checking filesize and re-download.
	if r.ctx.Offline() {
		return false, nil
	}

	// Verify sha256, not matches indicate file is corrupted or outdated, need to re-download.
	if sha256 != "" {
		computedSha256, err := ComputeSHA256(destFilePath)
		if err != nil {
			return false, fmt.Errorf("failed to compute sha256 for %s -> %w", archive, err)
		}
		if computedSha256 != sha256 {
			return true, nil
		}
	}

	return false, nil
}

// tryRestoreFromCache attempts to find and verify cached file by comparing sha256.
// Returns cached file path if found empty string otherwise.
func (r *Repair) tryRestoreFromCache(cacheDir, fileName string) (string, error) {
	if r.sha256 == "" {
		return "", nil
	}

	if r.ctx.Offline() {
		return "", nil
	}

	// First, find cached file by sha-256.
	cachedFile, err := FindCachedFile(cacheDir, fileName, r.sha256)
	if err != nil {
		return "", err
	}
	if cachedFile == "" {
		return "", nil
	}

	// Return cached file.
	return cachedFile, nil
}
