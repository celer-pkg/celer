package netfs

import (
	"fmt"
	"io"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func InitPkgCache(ctx context.Context) (pkgcache.DownloadCache, pkgcache.RepoCache, pkgcache.AritifactCache) {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() != nil && pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil, nil, nil
	}

	fs := pkgCache.GetFS()
	writable := pkgCache.GetOptions().Writable
	netfsCache := netfsCache{cacheDirRoot: fs.GetDir(pkgcache.DirRoot, ctx.Version())}

	downloadConfig := DownloadConfig{
		netfsCache: netfsCache,
		ctx:        ctx,
		cacheDir:   fs.GetDir(pkgcache.DirDownloads, ctx.Version()),
		writable:   writable,
	}

	artifactConfig := ArtifactConfig{
		netfsCache: netfsCache,
		ctx:        ctx,
		cacheDir:   fs.GetDir(pkgcache.DirArtifacts, ctx.Version()),
		writable:   writable,
		maxRetries: 3,
	}

	repoConfig := RepoConfig{
		netfsCache: netfsCache,
		ctx:        ctx,
		cacheDir:   fs.GetDir(pkgcache.DirRepos, ctx.Version()),
		writable:   writable,
	}

	return &downloadConfig, &repoConfig, &artifactConfig
}

type netfsCache struct {
	cacheDirRoot string // Remote cache root.
}

// DownloadFile copy remote file to local file in workspace, verifying sha256
// when provided.
func (n netfsCache) DownloadFile(remotePath, localPath, sha256 string) error {
	// Check if the file exists.
	if !fileio.PathExists(remotePath) {
		return fmt.Errorf("remote file not exist for '%s'", remotePath)
	}

	fileName := filepath.Base(remotePath)
	if sha256 != "" {
		// Verify file with sha256.
		color.Printf(color.Title, "\n%s\n", fmt.Sprintf("[validating file: %s]", fileName))
		color.PrintInline(color.Hint, "[-] validating '%s' with sha256: %s", fileName, sha256)
		remoteSha256, err := fileio.SHA256Sum(remotePath)
		if err != nil || remoteSha256 != sha256 {
			color.PrintInline(color.Hint, "[✘] validate '%s' with sha256: %s\n", fileName, sha256)
			return nil
		}
		color.PrintInline(color.Hint, "[✔] validate '%s' with sha256: %s\n", fileName, sha256)
	}

	// Make sure parent dir is available.
	if err := os.MkdirAll(filepath.Dir(localPath), os.ModePerm); err != nil {
		return err
	}

	return n.copyWithProgress(remotePath, localPath,
		"download '"+fileName+"'", fileName+" is downloaded")
}

// copyWithProgress copies srcPath to dstPath with a progress bar, the dest is
// flushed before returning so it can be consumed right away (esp. on Windows).
func (n netfsCache) copyWithProgress(srcPath, dstPath, title, done string) error {
	// Get file src info to get file size.
	srcInfo, err := os.Stat(srcPath)
	if err != nil {
		return fmt.Errorf("failed to get file info for '%s' -> %w", srcPath, err)
	}

	// This is the file to copy from.
	srcFile, err := os.Open(srcPath)
	if err != nil {
		return fmt.Errorf("failed to open file '%s' -> %w", srcPath, err)
	}
	defer srcFile.Close()

	// Create the file to copy to.
	dstFile, err := os.Create(dstPath)
	if err != nil {
		return fmt.Errorf("failed to create file '%s' -> %w", dstPath, err)
	}
	defer dstFile.Close()

	// Copy in progress.
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Pass, "[✔] %s (%s) in %s\n", done, formattedSize, formattedTimeCost)
	}
	progress := fileio.NewProgressBar(title, srcInfo.Size(), completed)
	if _, err := io.Copy(io.MultiWriter(dstFile, progress), srcFile); err != nil {
		return fmt.Errorf("failed to copy '%s' -> %w", srcPath, err)
	}

	// Flush and close before it's consumed.
	if err := dstFile.Sync(); err != nil {
		return fmt.Errorf("failed to sync file '%s' -> %w", dstPath, err)
	}
	return dstFile.Close()
}

// UploadFile uploads a local file to the remote cache with a progress bar.
func (n netfsCache) UploadFile(localPath, remotePath, sha256 string) error {
	return n.uploadFile(localPath, remotePath, sha256, true)
}

// uploadFile uploads localPath to remotePath atomically.
// It skips the upload when the remote file already matches sha256, and otherwise stages the file
// in the remote tmp dir and renames it into place.
func (n netfsCache) uploadFile(localPath, remotePath, sha256 string, withProgress bool) error {
	// Make sure parent dir is available.
	if err := os.MkdirAll(filepath.Dir(remotePath), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create cache dir for '%s' -> %w", remotePath, err)
	}

	// If cache file exists and SHA256 matches, return it directly.
	if fileio.PathExists(remotePath) && fileio.VerifyFileSHA256(remotePath, sha256) {
		return nil
	}

	// Write to tmp dir first then atomically rename into final location to avoid partial reads.
	remoteTmpDir := filepath.Join(n.cacheDirRoot, "tmp")
	if err := os.MkdirAll(remoteTmpDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create tmp dir for '%s' -> %w", remoteTmpDir, err)
	}

	// Create a remote tmp file as the copy dest.
	fileName := filepath.Base(localPath)
	destTmpFile, err := os.CreateTemp(remoteTmpDir, fmt.Sprintf("%s-*", fileName))
	if err != nil {
		return fmt.Errorf("failed to create tmp file for '%s' -> %w", fileName, err)
	}
	defer func() {
		destTmpFile.Close()
		os.Remove(destTmpFile.Name())
	}()

	// This is the file to copy from.
	srcFile, err := os.Open(localPath)
	if err != nil {
		return fmt.Errorf("failed to open src file for '%s' -> %w", localPath, err)
	}
	defer srcFile.Close()

	var dest io.Writer = destTmpFile
	if withProgress {
		srcInfo, err := os.Stat(localPath)
		if err != nil {
			return fmt.Errorf("failed to get file info for '%s' -> %w", localPath, err)
		}

		// Cache to pkgcache in progress.
		completed := func(formattedTimeCost, formattedSize string) {
			color.PrintInline(color.Pass, "[✔] %s (%s) in %s\n", fileName+" is stored", formattedSize, formattedTimeCost)
		}
		dest = io.MultiWriter(destTmpFile, fileio.NewProgressBar("upload '"+fileName+"' to remote fs", srcInfo.Size(), completed))
	}
	if _, err := io.Copy(dest, srcFile); err != nil {
		return fmt.Errorf("failed to upload '%s' to remote fs -> %w", fileName, err)
	}

	// Flush and close before rename.
	if err := destTmpFile.Sync(); err != nil {
		return fmt.Errorf("failed to sync file to cache -> %w", err)
	}
	if err := destTmpFile.Close(); err != nil {
		return fmt.Errorf("failed to close tmp file -> %w", err)
	}

	if err := os.Rename(destTmpFile.Name(), remotePath); err != nil {
		// Dest may have appeared between our check and rename (another user won the race).
		if fileio.VerifyFileSHA256(remotePath, sha256) {
			return nil
		}

		// Rename failed — just try again.
		if err := fileio.CopyFile(localPath, remotePath); err != nil {
			return fmt.Errorf("failed to copy file to cache -> %w", err)
		}
	}

	return nil
}
