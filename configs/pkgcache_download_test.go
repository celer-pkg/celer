package configs

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgcache/netfs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func TestDownloadCache_SaveAndFind(t *testing.T) {
	oldWorkspace := dirs.WorkspaceDir
	tmpWorkspace := t.TempDir()
	dirs.Init(tmpWorkspace)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	cacheDir := filepath.Join(tmpWorkspace, "cache")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	pkgCacheConfig := NewPkgCacheConfig()
	pkgCacheConfig.Dir = cacheDir
	pkgCacheConfig.Writable = true
	cachedDownloadsDir := pkgCacheConfig.GetDir(pkgcache.PkgCacheDirDownloads)

	// Create download cache through the NFS implementation using a fake context.
	fakeCtx := fakeContext{pkgCacheConfig: pkgCacheConfig}
	downloadCache := netfs.NewDownloadConfig(fakeCtx)
	if downloadCache == nil {
		t.Fatal("expected download cache to be created")
	}

	t.Run("save and find cached download", func(t *testing.T) {
		// Create a temporary source file to cache.
		srcDir := t.TempDir()
		srcFile := filepath.Join(srcDir, "test-tool-1.0.tar.gz")
		content := []byte("download-cache-test-content")
		if err := os.WriteFile(srcFile, content, os.ModePerm); err != nil {
			t.Fatal(err)
		}

		sha256 := fmt.Sprintf("%x", sha256.Sum256(content))
		fileName := "test-tool-1.0.tar.gz"

		// Save to cache via DownloadCache.Store.
		cachedPath, err := downloadCache.Store(fileName, sha256, srcFile)
		if err != nil {
			t.Fatalf("Store failed: %v", err)
		}

		if !fileio.PathExists(cachedPath) {
			t.Fatalf("expected cached file at %s", cachedPath)
		}

		// Verify the downloads directory was created.
		if !fileio.PathExists(cachedDownloadsDir) {
			t.Fatal("expected downloads cache directory to exist")
		}

		// Find the cached file via DownloadCache.Restore.
		foundPath, err := downloadCache.Restore(fileName, sha256)
		if err != nil {
			t.Fatalf("Restore failed: %v", err)
		}
		if foundPath == "" {
			t.Fatal("expected to find cached file")
		}
		if foundPath != cachedPath {
			t.Fatalf("expected found path %s, got %s", cachedPath, foundPath)
		}

		// Verify content integrity.
		computedHash, err := fileio.SHA256Sum(cachedPath)
		if err != nil {
			t.Fatalf("ComputeSHA256 failed: %v", err)
		}
		if computedHash != sha256 {
			t.Fatalf("sha256 mismatch: expected %s, got %s", sha256, computedHash)
		}
	})

	t.Run("save same file again should be same", func(t *testing.T) {
		srcDir := t.TempDir()
		srcFile := filepath.Join(srcDir, "test-tool-1.0.tar.gz")
		content := []byte("download-cache-test-content")
		if err := os.WriteFile(srcFile, content, os.ModePerm); err != nil {
			t.Fatal(err)
		}

		sha256 := fmt.Sprintf("%x", sha256.Sum256(content))
		fileName := "test-tool-1.0.tar.gz"

		cachedPath1, err := downloadCache.Store(fileName, sha256, srcFile)
		if err != nil {
			t.Fatalf("first Store failed: %v", err)
		}

		cachedPath2, err := downloadCache.Store(fileName, sha256, srcFile)
		if err != nil {
			t.Fatalf("second Store failed: %v", err)
		}

		if cachedPath1 != cachedPath2 {
			t.Fatalf("expected same path on repeated save, got %s and %s", cachedPath1, cachedPath2)
		}
	})

	t.Run("find non-existent file returns empty", func(t *testing.T) {
		foundPath, err := downloadCache.Restore("nonexistent.tar.gz", "abc123")
		if err != nil {
			t.Fatalf("Restore failed: %v", err)
		}
		if foundPath != "" {
			t.Fatalf("expected empty path for non-existent file, got %s", foundPath)
		}
	})
}
