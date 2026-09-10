package filesystem

import (
	"fmt"
	"io"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func InitPkgCache(ctx context.Context) (pkgcache.DownloadCache, pkgcache.RepoCache, pkgcache.AritifactCache) {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil || pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil, nil, nil
	}

	return NewDownloadConfig(ctx), NewRepoConfig(ctx), NewArtifactConfig(ctx)
}

type fsCache struct {
	cacheDirRoot string // Remote cache root.
}

// uploadFile uploads filePath to remotePath with a progress bar, mirroring the
// minio backend's uploadFile presentation.
func (f fsCache) uploadFile(kind pkgcache.Kind, filePath, remotePath, sha256, displayName string) error {
	srcInfo, err := os.Stat(filePath)
	if err != nil {
		return fmt.Errorf("failed to get file info for '%s' -> %w", filePath, err)
	}

	title := fmt.Sprintf("uploading '%s'", displayName)
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Success, "[✔] %-18s %-22s (%s) (%s)\n",
			fmt.Sprintf("[Store %s]", kind), displayName, formattedSize, formattedTimeCost)
	}
	return f.doUploadFile(filePath, remotePath, sha256, fileio.NewProgressBar(title, srcInfo.Size(), completed))
}

// uploadSilent uploads filePath to remotePath without any progress output.
func (f fsCache) uploadSilent(filePath, remotePath, sha256 string) error {
	return f.doUploadFile(filePath, remotePath, sha256, nil)
}

// downloadFile downloads remotePath to a tmp file with a progress bar.
func (f fsCache) downloadFile(kind pkgcache.Kind, remotePath, displayName string) (string, error) {
	srcInfo, err := os.Stat(remotePath)
	if err != nil {
		return "", fmt.Errorf("remote file not exist for '%s' -> %w", remotePath, err)
	}

	// Make sure the tmp dir is available before creating a tmp file inside it.
	if err := os.MkdirAll(dirs.TmpFilesDir, os.ModePerm); err != nil {
		return "", fmt.Errorf("failed to mkdir '%s' -> %w", dirs.TmpFilesDir, err)
	}
	localFile, err := os.CreateTemp(dirs.TmpFilesDir, "celer-pkgcache-*"+fileio.Ext(remotePath))
	if err != nil {
		return "", fmt.Errorf("failed to create tmp file in %s -> %w", dirs.TmpFilesDir, err)
	}
	defer localFile.Close()

	// Copy remote file to local with progress bar.
	remoteFile, err := os.Open(remotePath)
	if err != nil {
		return "", fmt.Errorf("failed to open file '%s' -> %w", remotePath, err)
	}
	defer remoteFile.Close()

	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Success, "[✔] %-14s %-22s %s (%s)\n",
			fmt.Sprintf("[Restore %s]", kind), displayName, formattedSize, formattedTimeCost)
	}
	progress := fileio.NewProgressBar(fmt.Sprintf("downloading '%s'", displayName), srcInfo.Size(), completed)
	if _, err := io.Copy(io.MultiWriter(localFile, progress), remoteFile); err != nil {
		return "", fmt.Errorf("failed to download '%s' -> %w", remotePath, err)
	}

	// Flush and close local file.
	if err := localFile.Sync(); err != nil {
		return "", fmt.Errorf("failed to sync file '%s' -> %w", localFile.Name(), err)
	}
	if err := localFile.Close(); err != nil {
		return "", fmt.Errorf("failed to close file '%s' -> %w", localFile.Name(), err)
	}
	return localFile.Name(), nil
}

// doUploadFile uploads a local file to the remote cache. When progress is
// non-nil it is used to render an upload progress bar; nil disables output.
func (f fsCache) doUploadFile(localPath, remotePath, sha256 string, progress io.Writer) error {
	// Make sure parent dir is available (+a compatible mkdir).
	if err := os.MkdirAll(filepath.Dir(remotePath), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create cache dir for '%s' -> %w", remotePath, err)
	}

	// If cache file exists and SHA256 matches, skip the upload.
	if fileio.PathExists(remotePath) && fileio.VerifyFileSHA256(remotePath, sha256) {
		return nil
	}

	// Write to tmp dir first (excluded from chattr +a), then atomically rename
	// into final location to avoid partial reads.
	remoteTmpDir := filepath.Join(f.cacheDirRoot, "tmp")
	if err := os.MkdirAll(remoteTmpDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create tmp dir for '%s' -> %w", remoteTmpDir, err)
	}

	// Create a remote tmp file as the copy dest.
	fileName := filepath.Base(localPath)
	destTmpFile, err := os.CreateTemp(remoteTmpDir, fmt.Sprintf("celer-pkgcache-%s-*.%s", fileName, fileio.Ext(localPath)))
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

	var destFile io.Writer = destTmpFile
	if progress != nil {
		destFile = io.MultiWriter(destTmpFile, progress)
	}
	if _, err := io.Copy(destFile, srcFile); err != nil {
		return fmt.Errorf("failed to upload '%s' to remote fs -> %w", fileName, err)
	}

	// Flush and close before rename so the data is durable on NFS.
	if err := destTmpFile.Sync(); err != nil {
		return fmt.Errorf("failed to sync file to cache -> %w", err)
	}
	if err := destTmpFile.Close(); err != nil {
		return fmt.Errorf("failed to close tmp file -> %w", err)
	}

	// Rename tmp file as dest file.
	if err := os.Rename(destTmpFile.Name(), remotePath); err != nil {
		// Dest may have appeared between our check and rename (another user won the race).
		if fileio.VerifyFileSHA256(remotePath, sha256) {
			return nil
		}

		// Rename failed — likely chattr +a dir with an existing corrupt dest.
		// Fall back to in-place overwrite (O_TRUNC), which +a allows, via chattrFS.
		if err := fileio.CopyFile(localPath, remotePath); err != nil {
			return fmt.Errorf("failed to copy file to cache -> %w", err)
		}
	}

	return nil
}
