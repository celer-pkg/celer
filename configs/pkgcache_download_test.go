package configs

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/context"
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

	fakeCtx := fakeContext{
		platform: "x86_64-linux",
		project:  "proj",
		build:    "release",
	}
	pkgCache := NewPkgCache(fakeCtx, cacheDir, true)
	fakeCtx.pkgCache = pkgCache
	pkgCache.ctx = fakeCtx
	if err := pkgCache.Refresh(); err != nil {
		t.Fatal(err)
	}

	cachedDownloadsDir := pkgCache.GetDir(context.PkgCacheDirDownloads)
	chattrFS := fileio.NewChattrFS(pkgCache.GetDir(context.PkgCacheDirRoot))

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

		// Save to cache.
		cachedPath, err := fileio.SaveCachedFile(srcFile, cachedDownloadsDir, fileName, sha256, chattrFS)
		if err != nil {
			t.Fatalf("SaveCachedFile failed: %v", err)
		}

		if !fileio.PathExists(cachedPath) {
			t.Fatalf("expected cached file at %s", cachedPath)
		}

		// Verify the downloads directory was created.
		if !fileio.PathExists(cachedDownloadsDir) {
			t.Fatal("expected downloads cache directory to exist")
		}

		// Find the cached file.
		foundPath, err := fileio.FindCachedFile(cachedDownloadsDir, fileName, sha256)
		if err != nil {
			t.Fatalf("FindCachedFile failed: %v", err)
		}
		if foundPath == "" {
			t.Fatal("expected to find cached file")
		}
		if foundPath != cachedPath {
			t.Fatalf("expected found path %s, got %s", cachedPath, foundPath)
		}

		// Verify content integrity.
		computedHash, err := fileio.ComputeSHA256(cachedPath)
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

		cachedPath1, err := fileio.SaveCachedFile(srcFile, cachedDownloadsDir, fileName, sha256, chattrFS)
		if err != nil {
			t.Fatalf("first SaveCachedFile failed: %v", err)
		}

		cachedPath2, err := fileio.SaveCachedFile(srcFile, cachedDownloadsDir, fileName, sha256, chattrFS)
		if err != nil {
			t.Fatalf("second SaveCachedFile failed: %v", err)
		}

		if cachedPath1 != cachedPath2 {
			t.Fatalf("expected same path on repeated save, got %s and %s", cachedPath1, cachedPath2)
		}
	})

	t.Run("find non-existent file returns empty", func(t *testing.T) {
		foundPath, err := fileio.FindCachedFile(cachedDownloadsDir, "nonexistent.tar.gz", "abc123")
		if err != nil {
			t.Fatalf("FindCachedFile failed: %v", err)
		}
		if foundPath != "" {
			t.Fatalf("expected empty path for non-existent file, got %s", foundPath)
		}
	})
}
