package minio

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/minio/minio-go/v7"
)

type BinaryCache struct {
	minioCache
	ctx      context.Context
	cacheDir string
	version  string
	writable bool
}

func NewBinaryCache(ctx context.Context, client *minio.Client) *BinaryCache {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetMinio() == nil {
		return nil
	}

	minioConfig := pkgCache.GetMinio()
	return &BinaryCache{
		ctx:        ctx,
		minioCache: minioCache{client: client, bucketName: bucketName},
		cacheDir:   minioConfig.GetDir(pkgcache.DirBinaries, ctx.Version()),
		version:    ctx.Version(),
		writable:   pkgCache.IsWritable(),
	}
}

func (d BinaryCache) Store() error {
	// Skip when offline.
	if d.ctx.Offline() {
		return nil
	}

	// Skip for not writable.
	if !d.writable {
		return nil
	}

	// Create default bucket if not exist.
	if err := d.CreateBucketIfNotExist(); err != nil {
		return err
	}

	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's binary path -> %w", err)
	}

	// Build object name like: 'celer-linux-x86_64-v0.5.1'.
	binaryName := pkgcache.CelerBinaryName(d.version)
	objectName := filepath.Join(d.cacheDir, binaryName)

	// Seeking celer binary with its name is enough.
	if info, err := d.GetFileInfo(objectName); err != nil {
		return fmt.Errorf("failed to check cached celer binary '%s' -> %w", objectName, err)
	} else if info != nil {
		return nil
	}

	// Upload celer with progress.
	return d.uploadFile(executable, objectName, binaryName)
}

func (d BinaryCache) Restore(version string) (string, error) {
	// skip when offline.
	if d.ctx.Offline() {
		return "", fmt.Errorf("can not restore celer binary because offline is 'ON'")
	}

	version = strings.TrimSpace(version)
	if version == "" {
		return "", fmt.Errorf("no celer binary version provided to restore")
	}

	// The requested version, never d.version: this runs precisely because the running
	// binary is another version than the schedule's.
	binaryName := pkgcache.CelerBinaryName(version)
	objectName := filepath.Join(d.cacheDir, binaryName)
	remoteInfo, err := d.GetFileInfo(objectName)
	if err != nil {
		return "", err
	}

	// No cache found, then download from source.
	if remoteInfo == nil {
		return "", fmt.Errorf("no celer binary found with version '%s' in minio", version)
	}

	// Download into a task-owned tmp dir and run it from there.
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return "", err
	}

	tmpDownloaded, err := d.downloadFile(localTmpDir, objectName, binaryName)
	if err != nil {
		os.RemoveAll(localTmpDir)
		return "", err
	}
	if err := os.Chmod(tmpDownloaded, 0o755); err != nil {
		os.RemoveAll(localTmpDir)
		return "", err
	}

	return tmpDownloaded, nil
}
