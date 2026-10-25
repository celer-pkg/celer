package filesystem

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// BinaryCache stores the celer binary of a version on a shared directory, so a node runs
// the one that exported its schedule instead of whatever its PATH has.
type BinaryCache struct {
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewBinaryCache(ctx context.Context) *BinaryCache {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil {
		return nil
	}

	return &BinaryCache{
		ctx:      ctx,
		cacheDir: pkgCache.GetFS().GetDir(pkgcache.DirBinaries, ctx.Version()),
		writable: pkgCache.IsWritable(),
	}
}

func (b BinaryCache) Store() error {
	// Skip when offline or the cache is read-only.
	if b.ctx.Offline() || !b.writable {
		return nil
	}

	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's binary path -> %w", err)
	}

	// The cache is authoritative: an existing entry is never overwritten.
	destPath := filepath.Join(b.cacheDir, pkgcache.CelerBinaryName(b.ctx.Version()))
	if fileio.PathExists(destPath) {
		return nil
	}

	if err := os.MkdirAll(b.cacheDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create '%s' -> %w", b.cacheDir, err)
	}
	if err := fileio.CopyFile(executable, destPath); err != nil {
		return fmt.Errorf("failed to store celer binary '%s' -> %w", destPath, err)
	}

	return os.Chmod(destPath, os.ModePerm)
}

func (b BinaryCache) Restore(version string) (string, error) {
	if b.ctx.Offline() {
		return "", fmt.Errorf("can not restore celer binary because offline is 'ON'")
	}

	version = strings.TrimSpace(version)
	if version == "" {
		return "", fmt.Errorf("no celer binary version provided to restore")
	}

	// The requested version, never the running one: this runs precisely because the
	// running binary is another version than the schedule's.
	binaryName := pkgcache.CelerBinaryName(version)
	srcPath := filepath.Join(b.cacheDir, binaryName)
	if !fileio.PathExists(srcPath) {
		return "", fmt.Errorf("no celer binary found with version '%s' in '%s'", version, b.cacheDir)
	}

	// Copy into a task-owned tmp dir and run it from there.
	tmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return "", err
	}

	destPath := filepath.Join(tmpDir, binaryName)
	if err := fileio.CopyFile(srcPath, destPath); err != nil {
		os.RemoveAll(tmpDir)
		return "", fmt.Errorf("failed to restore celer binary '%s' -> %w", destPath, err)
	}
	if err := os.Chmod(destPath, 0o755); err != nil {
		os.RemoveAll(tmpDir)
		return "", err
	}

	return destPath, nil
}
