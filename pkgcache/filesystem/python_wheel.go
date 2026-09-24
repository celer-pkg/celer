package filesystem

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

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
		writable: pkgCache.GetOptions().Writable,
	}
}

// Restore copies every cached wheel under cacheKey into destDir (the pip
// --find-links target). ok=false when the cache entry does not exist.
func (p PythonWheelConfig) Restore(cacheKey, destDir string) (bool, error) {
	keyDir := filepath.Join(p.cacheDir, cacheKey)
	entries, err := os.ReadDir(keyDir)
	if err != nil || !hasWheels(entries) {
		return false, nil
	}

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir wheelhouse '%s' -> %w", destDir, err)
	}

	for _, entity := range entries {
		if entity.IsDir() || !strings.HasSuffix(entity.Name(), ".whl") {
			continue
		}

		remoteWheel := filepath.Join(keyDir, entity.Name())
		destWheel := filepath.Join(destDir, entity.Name())
		if err := fileio.CopyFile(remoteWheel, destWheel); err != nil {
			return false, fmt.Errorf("failed to restore wheel %s -> %w", entity.Name(), err)
		}
	}
	return true, nil
}

// Store copies every *.whl under wheelhouseDir into the cache entry for cacheKey.
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
		if entity.IsDir() || !strings.HasSuffix(entity.Name(), ".whl") {
			continue
		}

		localWheel := filepath.Join(wheelhouseDir, entity.Name())
		localSha256, err := fileio.SHA256Sum(localWheel)
		if err != nil {
			return fmt.Errorf("failed to compute sha256 for '%s' -> %w", entity.Name(), err)
		}

		// The cache is authoritative: never auto-overwrite an existing entry.
		remoteWheel := filepath.Join(keyDir, entity.Name())
		if fileio.PathExists(remoteWheel) {
			cachedSha256, err := fileio.SHA256Sum(remoteWheel)
			if err != nil {
				return fmt.Errorf("failed to calculate sha256 of '%s' -> %w", entity.Name(), err)
			}
			if cachedSha256 == localSha256 {
				continue // already cached with the same content.
			}
			return fmt.Errorf("%w -> '%s' is cached with sha256=%s, but storing sha256=%s; please remove the cached file manually if you want to replace it",
				errors.ErrSha256Mismatch, entity.Name(), cachedSha256, localSha256)
		}

		if err := p.uploadFile(localWheel, remoteWheel, localSha256, entity.Name()); err != nil {
			return fmt.Errorf("failed to cache wheel '%s' -> %w", entity.Name(), err)
		}
	}
	return nil
}

// hasWheels reports whether any entry is a .whl file.
func hasWheels(entries []os.DirEntry) bool {
	for _, entity := range entries {
		if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".whl") {
			return true
		}
	}
	return false
}
