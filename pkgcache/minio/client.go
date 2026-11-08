package minio

import (
	gocontext "context"
	"fmt"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/minio/minio-go/v7"
)

type ClientCache struct {
	Minio
	ctx      context.Context
	cacheDir string
	version  string
	readOnly bool
}

func NewClientCache(ctx context.Context, client *minio.Client) *ClientCache {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetMinio() == nil {
		return nil
	}

	return &ClientCache{
		ctx:      ctx,
		Minio:    Minio{client: client, bucketName: bucketName},
		cacheDir: pkgCache.GetMinio().GetDir(pkgcache.DirBinaries, ctx.Version()),
		version:  ctx.Version(),
		readOnly: pkgCache.IsReadOnly(),
	}
}

func NewClientCacheFrom(ctx context.Context, client *minio.Client, cfg *pkgcache.Minio, readOnly bool) *ClientCache {
	return &ClientCache{
		ctx:      ctx,
		Minio:    Minio{client: client, bucketName: bucketName},
		cacheDir: cfg.GetDir(pkgcache.DirBinaries, ctx.Version()),
		version:  ctx.Version(),
		readOnly: readOnly,
	}
}

func (c ClientCache) Store() error {
	// Skip when offline.
	if c.ctx.Offline() {
		return nil
	}

	// Skip when the cache is read-only.
	if c.readOnly {
		return nil
	}

	// Create default bucket if not exist.
	if err := c.CreateBucketIfNotExist(); err != nil {
		return err
	}

	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's binary path -> %w", err)
	}

	// Build object name like: 'celer-x86_64-linux-v0.5.1'.
	binaryName := pkgcache.ClientName(c.version)
	objectName := path.Join(c.cacheDir, binaryName)

	// Seeking celer binary with its name is enough.
	if info, err := c.GetFileInfo(objectName); err != nil {
		return fmt.Errorf("failed to check cached celer binary '%s' -> %w", objectName, err)
	} else if info != nil {
		return nil
	}

	// Upload celer with progress.
	return c.uploadFile(executable, objectName, binaryName)
}

func (c ClientCache) Restore(version string) (string, error) {
	// skip when offline.
	if c.ctx.Offline() {
		return "", fmt.Errorf("can not restore celer binary because offline is 'ON'")
	}

	version = strings.TrimSpace(version)
	if version == "" {
		return "", fmt.Errorf("no celer binary version provided to restore")
	}

	// The requested version, never b.version: this runs precisely because the running
	// binary is another version than the schedule's.
	binaryName := pkgcache.ClientName(version)
	objectName := path.Join(c.cacheDir, binaryName)
	remoteInfo, err := c.GetFileInfo(objectName)
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

	tmpDownloaded, err := c.downloadFile(localTmpDir, objectName, binaryName)
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

func (c ClientCache) List() ([]string, error) {
	seen := make(map[string]bool)
	var versions []string

	opts := minio.ListObjectsOptions{Prefix: c.cacheDir + "/", Recursive: true}
	for object := range c.client.ListObjects(gocontext.Background(), c.bucketName, opts) {
		if object.Err != nil {
			resp := minio.ToErrorResponse(object.Err)
			if resp.Code == minio.NoSuchBucket {
				return nil, nil
			}
			return nil, fmt.Errorf("failed to list celer binaries in minio -> %w", object.Err)
		}

		version, ok := pkgcache.ClientVersion(path.Base(object.Key))
		if !ok || seen[version] {
			continue
		}
		seen[version] = true
		versions = append(versions, version)
	}

	pkgcache.SortVersions(versions)
	return versions, nil
}

func (c ClientCache) Fetch(version, destPath string) error {
	version = strings.TrimSpace(version)
	if version == "" {
		return fmt.Errorf("no celer binary version provided to fetch")
	}

	binaryName := pkgcache.ClientName(version)
	objectName := path.Join(c.cacheDir, binaryName)
	remoteInfo, err := c.GetFileInfo(objectName)
	if err != nil {
		return err
	}
	if remoteInfo == nil {
		return fmt.Errorf("no celer binary found with version '%s' in minio", version)
	}

	destDir := filepath.Dir(destPath)
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create '%s' -> %w", destDir, err)
	}

	tmpDownloaded, err := c.downloadFile(destDir, objectName, binaryName)
	if err != nil {
		return err
	}
	if err := fileio.MoveFile(tmpDownloaded, destPath); err != nil {
		os.Remove(tmpDownloaded)
		return fmt.Errorf("failed to place celer binary at '%s' -> %w", destPath, err)
	}

	return os.Chmod(destPath, 0o755)
}
