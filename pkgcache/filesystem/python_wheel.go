package filesystem

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// sha256Ext is the suffix of the sidecar file that records a wheel's sha256.
const sha256Ext = ".sha256"

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
		return false, fmt.Errorf("failed to mkdir wheelhouse %s -> %w", destDir, err)
	}

	for _, entity := range entries {
		if entity.IsDir() || !strings.HasSuffix(entity.Name(), ".whl") {
			continue
		}
		cachedWheel := filepath.Join(keyDir, entity.Name())
		sha256Hex, _ := os.ReadFile(cachedWheel + sha256Ext)

		destWheel := filepath.Join(destDir, entity.Name())
		if fileio.PathExists(destWheel) && fileio.VerifyFileSHA256(destWheel, string(sha256Hex)) {
			continue
		}
		if err := fileio.CopyFile(cachedWheel, destWheel); err != nil {
			return false, fmt.Errorf("failed to restore wheel %s -> %w", entity.Name(), err)
		}
	}
	return true, nil
}

// Store copies every *.whl under wheelhouseDir into the cache entry for
// cacheKey, along with a <name>.whl.sha256 sidecar per wheel.
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

		srcWheel := filepath.Join(wheelhouseDir, entity.Name())
		sha256, err := fileio.SHA256Sum(srcWheel)
		if err != nil {
			return fmt.Errorf("failed to compute sha256 for '%s' -> %w", entity.Name(), err)
		}

		// uploadFile stages through the cache root tmp dir and atomically
		// renames; it no-ops when the remote already matches sha256.
		destWheel := filepath.Join(keyDir, entity.Name())
		if err := p.uploadFile(srcWheel, destWheel, sha256, entity.Name()); err != nil {
			return fmt.Errorf("failed to cache wheel '%s' -> %w", entity.Name(), err)
		}

		// Write the sha256 sidecar (tiny, no progress bar needed).
		destPath := filepath.Join(keyDir, entity.Name()+sha256Ext)
		if fileio.PathExists(destPath) {
			return nil
		}
		return os.WriteFile(destPath, []byte(sha256), os.ModePerm)
	}
	return nil
}

// hasWheels reports whether any entry is a .whl file.
func hasWheels(entries []os.DirEntry) bool {
	for _, e := range entries {
		if !e.IsDir() && strings.HasSuffix(e.Name(), ".whl") {
			return true
		}
	}
	return false
}
