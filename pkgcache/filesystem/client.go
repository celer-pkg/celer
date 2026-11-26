package filesystem

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/fileio"
	"github.com/celer-pkg/celer/pkgcache"
)

// ClientCache stores the celer client binary of a version on a shared directory, so a node runs
// the one that exported its schedule instead of whatever its PATH has.
type ClientCache struct {
	FileSystem
	ctx      context.Context
	readOnly bool
}

func NewClientCache(ctx context.Context) *ClientCache {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil {
		return nil
	}

	fs := pkgCache.GetFS()
	return &ClientCache{
		ctx:          ctx,
		cacheDirNode: fs.GetDir(pkgcache.DirClients, ctx.Version()),
		readOnly:     pkgCache.IsReadOnly(),
	}
}

func NewClientCacheFrom(ctx context.Context, fs *pkgcache.FS, readOnly bool) *ClientCache {
	return &ClientCache{
		ctx:          ctx,
		cacheDirNode: fs.GetDir(pkgcache.DirClients, ctx.Version()),
		readOnly:     readOnly,
	}
}

func (c ClientCache) Store() error {
	// Skip when offline or the cache is read-only.
	if c.ctx.Offline() || c.readOnly {
		return nil
	}

	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get client's binary path -> %w", err)
	}

	// The cache is authoritative: an existing entry is never overwritten.
	destPath := filepath.Join(c.cacheDirNode, pkgcache.ClientName(c.ctx.Version()))
	if fileio.PathExists(destPath) {
		return nil
	}

	if err := os.MkdirAll(c.cacheDirNode, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create '%s' -> %w", c.cacheDirNode, err)
	}
	if err := fileio.CopyFile(executable, destPath); err != nil {
		return fmt.Errorf("failed to store client binary '%s' -> %w", destPath, err)
	}

	return os.Chmod(destPath, os.ModePerm)
}

func (c ClientCache) Restore(version string) (string, error) {
	if c.ctx.Offline() {
		return "", fmt.Errorf("can not restore client binary because offline is 'ON'")
	}

	version = strings.TrimSpace(version)
	if version == "" {
		return "", fmt.Errorf("no client binary version provided to restore")
	}

	// The requested version, never the running one: this runs precisely because the
	// running binary is another version than the schedule's.
	binaryName := pkgcache.ClientName(version)
	srcPath := filepath.Join(c.cacheDirNode, binaryName)
	if !fileio.PathExists(srcPath) {
		return "", fmt.Errorf("no client binary found with version '%s' in '%s'", version, c.cacheDirNode)
	}

	// Copy into a task-owned tmp dir and run it from there.
	tmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return "", err
	}

	destPath := filepath.Join(tmpDir, binaryName)
	if err := fileio.CopyFile(srcPath, destPath); err != nil {
		os.RemoveAll(tmpDir)
		return "", fmt.Errorf("failed to restore client binary '%s' -> %w", destPath, err)
	}
	if err := os.Chmod(destPath, 0o755); err != nil {
		os.RemoveAll(tmpDir)
		return "", err
	}

	return destPath, nil
}

// List returns the versions of this platform's binaries in the cache, newest first.
// An empty cache is an empty list, not an error.
func (c ClientCache) List() ([]string, error) {
	entries, err := os.ReadDir(c.cacheDirNode)
	if err != nil {
		if os.IsNotExist(err) {
			return nil, nil
		}
		return nil, fmt.Errorf("failed to read '%s' -> %w", c.cacheDirNode, err)
	}

	seen := make(map[string]bool, len(entries))
	versions := make([]string, 0, len(entries))
	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}

		version, ok := pkgcache.ClientVersion(entry.Name())
		if !ok || seen[version] {
			continue
		}
		seen[version] = true
		versions = append(versions, version)
	}

	pkgcache.SortVersions(versions)
	return versions, nil
}

// Fetch writes the binary of version straight to destPath, through a tmp file in the
// destination directory so a partial download never lands on destPath.
func (c ClientCache) Fetch(version, destPath string) error {
	version = strings.TrimSpace(version)
	if version == "" {
		return fmt.Errorf("no client binary version provided to fetch")
	}

	srcPath := filepath.Join(c.cacheDirNode, pkgcache.ClientName(version))
	if !fileio.PathExists(srcPath) {
		return fmt.Errorf("no client binary found with version '%s' in '%s'", version, c.cacheDirNode)
	}

	// Copy next to the destination, then rename into place, so a partial copy never
	// lands on destPath and a running binary is only replaced atomically.
	if err := os.MkdirAll(filepath.Dir(destPath), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create '%s' -> %w", filepath.Dir(destPath), err)
	}
	tmpPath := destPath + ".tmp"
	os.Remove(tmpPath)
	if err := fileio.CopyFile(srcPath, tmpPath); err != nil {
		return fmt.Errorf("failed to fetch client binary '%s' -> %w", version, err)
	}
	if err := fileio.MoveFile(tmpPath, destPath); err != nil {
		os.Remove(tmpPath)
		return fmt.Errorf("failed to place client binary at '%s' -> %w", destPath, err)
	}

	return os.Chmod(destPath, 0o755)
}
