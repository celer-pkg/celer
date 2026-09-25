package minio

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/minio/minio-go/v7"
)

type PythonWheelConfig struct {
	minioCache
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewPythonWheelConfig(ctx context.Context, client *minio.Client) *PythonWheelConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetMinio() == nil {
		return nil
	}

	minioConfig := pkgCache.GetMinio()
	return &PythonWheelConfig{
		ctx: ctx,
		minioCache: minioCache{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir: minioConfig.GetDir(pkgcache.DirPythonWheels, ctx.Version()),
		writable: pkgCache.GetOptions().Writable,
	}
}

// keyPrefix returns the minio object prefix holding the wheelhouse for cacheKey.
func (p PythonWheelConfig) keyPrefix(cacheKey string) string {
	return filepath.Join(p.cacheDir, cacheKey)
}

// Restore lists every wheel object under cacheKey's prefix and downloads them
// into destDir (the pip --find-links target).
// return false when the prefix has no wheels.
func (p PythonWheelConfig) Restore(cacheKey, destDir string) (bool, error) {
	if p.ctx.Offline() {
		return false, nil
	}

	prefix := p.keyPrefix(cacheKey) + "/"

	// List every object under the prefix and collect the wheel names.
	var wheels []string
	for info := range p.client.ListObjects(context.Background(), p.bucketName, minio.ListObjectsOptions{
		Prefix:    prefix,
		Recursive: true,
	}) {
		if info.Err != nil {
			return false, fmt.Errorf("failed to list python wheels for '%s' -> %w", cacheKey, info.Err)
		}

		wheel := filepath.Base(info.Key)
		if !strings.HasSuffix(wheel, ".whl") {
			continue
		}

		wheels = append(wheels, wheel)
	}
	if len(wheels) == 0 {
		return false, nil
	}

	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir wheelhouse '%s' -> %w", destDir, err)
	}

	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return false, fmt.Errorf("failed to create tmp dir -> %w", err)
	}
	defer os.RemoveAll(localTmpDir)

	for _, wheel := range wheels {
		objectName := filepath.Join(p.cacheDir, cacheKey, wheel)
		tmpWheel, err := p.downloadFile(localTmpDir, objectName, wheel)
		if err != nil {
			return false, fmt.Errorf("failed to download wheel %s -> %w", wheel, err)
		}

		destWheel := filepath.Join(destDir, wheel)
		if err := fileio.CopyFile(tmpWheel, destWheel); err != nil {
			return false, fmt.Errorf("failed to place wheel '%s' -> %w", wheel, err)
		}
	}
	return true, nil
}

// Store uploads every *.whl under wheelhouseDir into the cache entry for cacheKey.
func (p PythonWheelConfig) Store(cacheKey, wheelhouseDir string) error {
	if p.ctx.Offline() {
		return nil
	}
	if !p.writable {
		return nil
	}

	// Make sure bucket is already created.
	if err := p.CreateBucketIfNotExist(); err != nil {
		return fmt.Errorf("failed to ensure bucket for python wheel cache -> %w", err)
	}

	entries, err := os.ReadDir(wheelhouseDir)
	if err != nil {
		return fmt.Errorf("failed to read wheelhouse '%s' -> %w", wheelhouseDir, err)
	}

	prefix := p.keyPrefix(cacheKey)
	for _, entity := range entries {
		if entity.IsDir() || !strings.HasSuffix(entity.Name(), ".whl") {
			continue
		}
		localWheel := filepath.Join(wheelhouseDir, entity.Name())
		localSha256, err := fileio.SHA256Sum(localWheel)
		if err != nil {
			return fmt.Errorf("failed to compute sha256 for '%s' -> %w", entity.Name(), err)
		}
		wheelObject := filepath.Join(prefix, entity.Name())

		// The cache is authoritative: never auto-overwrite an existing entry.
		if info, err := p.GetFileInfo(wheelObject); err != nil {
			return fmt.Errorf("failed to stat wheel '%s' -> %w", entity.Name(), err)
		} else if info != nil {
			remoteSha256 := p.metaSha256(info)
			if remoteSha256 == "" {
				return fmt.Errorf("'%s' is cached but has no sha256 metadata; please manually remove it before re-storing", entity.Name())
			}
			if remoteSha256 == localSha256 {
				continue // already cached with the same content.
			}
			return fmt.Errorf("'%s' is cached with sha256=%s, but storing sha256=%s; remove the cached object manually if you want to replace it", entity.Name(), remoteSha256, localSha256)
		}

		if err := p.uploadFile(localWheel, wheelObject, entity.Name()); err != nil {
			return fmt.Errorf("failed to cache wheel '%s' -> %w", entity.Name(), err)
		}
	}
	return nil
}
