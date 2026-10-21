package filesystem

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// PythonWheelConfig implements pkgcache.PythonWheelCache on the shared FS cache.
type PythonWheelConfig struct {
	fsCache
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewPythonWheelConfig(ctx context.Context) *PythonWheelConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil || pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil
	}

	filesystem := pkgCache.GetFS()
	cacheRootDir := filesystem.GetDir(pkgcache.DirRoot, ctx.Version())
	return &PythonWheelConfig{
		fsCache: fsCache{
			cacheDirRoot: cacheRootDir,
		},
		ctx:      ctx,
		cacheDir: filesystem.GetDir(pkgcache.DirPythonWheels, ctx.Version()),
		writable: pkgCache.IsWritable(),
	}
}

// Restore copies every cached distribution under cacheKey into destDir (the pip
// --find-links target). ok=false when the cache entry does not exist.
func (p PythonWheelConfig) Restore(cacheKey, destDir string) (bool, error) {
	keyDir := filepath.Join(p.cacheDir, cacheKey)
	entries, err := os.ReadDir(keyDir)
	if err != nil || !hasDistributions(entries) {
		return false, nil
	}

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir wheelhouse '%s' -> %w", destDir, err)
	}

	for _, entity := range entries {
		if entity.IsDir() || !pkgcache.IsPythonDistribution(entity.Name()) {
			continue
		}

		remoteFile := filepath.Join(keyDir, entity.Name())
		destFile := filepath.Join(destDir, entity.Name())
		if err := fileio.CopyFile(remoteFile, destFile); err != nil {
			return false, fmt.Errorf("failed to restore python distribution %s -> %w", entity.Name(), err)
		}
	}
	return true, nil
}

// Store copies every python distribution under wheelhouseDir into the cache entry for cacheKey.
func (p PythonWheelConfig) Store(cacheKey, wheelhouseDir string) error {
	if p.ctx.Offline() {
		return nil
	}
	if !p.writable {
		return nil
	}

	keyDir := filepath.Join(p.cacheDir, cacheKey)
	if err := os.MkdirAll(keyDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to mkdir python wheel cache for '%s' -> %w", cacheKey, err)
	}

	entries, err := os.ReadDir(wheelhouseDir)
	if err != nil {
		return fmt.Errorf("failed to read wheelhouse %s -> %w", wheelhouseDir, err)
	}

	for _, entity := range entries {
		if entity.IsDir() || !pkgcache.IsPythonDistribution(entity.Name()) {
			continue
		}

		localFile := filepath.Join(wheelhouseDir, entity.Name())
		localSha256, err := fileio.SHA256Sum(localFile)
		if err != nil {
			return fmt.Errorf("failed to compute sha256 for '%s' -> %w", entity.Name(), err)
		}

		// The cache is authoritative: never auto-overwrite an existing entry.
		remoteFile := filepath.Join(keyDir, entity.Name())
		if fileio.PathExists(remoteFile) {
			cachedSha256, err := fileio.SHA256Sum(remoteFile)
			if err != nil {
				return fmt.Errorf("failed to calculate sha256 of '%s' -> %w", entity.Name(), err)
			}
			if cachedSha256 == localSha256 {
				continue // already cached with the same content.
			}
			return fmt.Errorf("%w -> '%s' is cached with sha256=%s, but storing sha256=%s; please remove the cached file manually if you want to replace it",
				errors.ErrSha256Mismatch, entity.Name(), cachedSha256, localSha256)
		}

		if err := p.uploadFile(localFile, remoteFile, localSha256, entity.Name()); err != nil {
			return fmt.Errorf("failed to cache python distribution '%s' -> %w", entity.Name(), err)
		}
	}
	return nil
}

// hasDistributions reports whether any entry is a python distribution file.
func hasDistributions(entries []os.DirEntry) bool {
	for _, entity := range entries {
		if !entity.IsDir() && pkgcache.IsPythonDistribution(entity.Name()) {
			return true
		}
	}
	return false
}
