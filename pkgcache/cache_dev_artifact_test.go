package pkgcache

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func newTestDevArtifactCache(t *testing.T) *DevArtifactCache {
	t.Helper()

	// Redirect workspace so TmpFilesDir is inside the sandbox.
	oldWS := dirs.WorkspaceDir
	tmpWS := t.TempDir()
	dirs.Init(tmpWS)
	t.Cleanup(func() { dirs.Init(oldWS) })

	cacheDir := filepath.Join(tmpWS, "devcache")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	return NewDevArtifactCache(fakeContext{}, cacheDir)
}

// makePackageDir creates a fake built package directory with a couple of files
// inside, mimicking what doInstallFromSource leaves behind.
func makePackageDir(t *testing.T, nameVersion string) string {
	t.Helper()
	pkgDir := filepath.Join(t.TempDir(), "packages", nameVersion)
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(pkgDir, "libfoo.a"), []byte("fake archive"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(filepath.Join(pkgDir, "include"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(pkgDir, "include", "foo.h"), []byte("fake header"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	return pkgDir
}

func computeHash(meta string) string {
	return fmt.Sprintf("%x", sha256.Sum256([]byte(meta)))
}

// ---- Store tests ----

func TestDevStore_ArchiveCreated(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "gflags@2.2.2")

	const meta = "test-meta-content"
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatalf("Store failed: %v", err)
	}

	hash := computeHash(meta)
	archivePath := filepath.Join(cache.cacheDir, "gflags@2.2.2", hash+".tar.gz")
	metaPath := filepath.Join(cache.cacheDir, "gflags@2.2.2", "metas", hash+".meta")

	if !fileio.PathExists(archivePath) {
		t.Errorf("archive not created at %s", archivePath)
	}
	if !fileio.PathExists(metaPath) {
		t.Errorf("meta file not created at %s", metaPath)
	}

	// Meta content should match what was passed in.
	got, err := os.ReadFile(metaPath)
	if err != nil {
		t.Fatal(err)
	}
	if string(got) != meta {
		t.Errorf("meta content = %q, want %q", got, meta)
	}
}

func TestDevStore_DifferentMetaProducesDifferentHash(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "eigen@3.4.0")

	if err := cache.Store(pkgDir, "meta-v1"); err != nil {
		t.Fatal(err)
	}
	if err := cache.Store(pkgDir, "meta-v2"); err != nil {
		t.Fatal(err)
	}

	// Two different metas → two different archive files.
	entries, err := os.ReadDir(filepath.Join(cache.cacheDir, "eigen@3.4.0"))
	if err != nil {
		t.Fatal(err)
	}
	var tarCount int
	for _, e := range entries {
		if e.Name() != "metas" && filepath.Ext(e.Name()) == ".gz" {
			tarCount++
		}
	}
	if tarCount != 2 {
		t.Errorf("expected 2 archives (one per hash), got %d", tarCount)
	}
}

func TestDevStore_SameMetaIsIdempotent(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "glog@0.6.0")

	const meta = "same-meta"
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatal(err)
	}
	// Store again with identical meta → same hash, should overwrite not duplicate.
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatal(err)
	}

	hash := computeHash(meta)
	archivePath := filepath.Join(cache.cacheDir, "glog@0.6.0", hash+".tar.gz")
	if !fileio.PathExists(archivePath) {
		t.Errorf("archive should exist after idempotent store: %s", archivePath)
	}

	// Count .tar.gz files — should be exactly 1.
	entries, err := os.ReadDir(filepath.Join(cache.cacheDir, "glog@0.6.0"))
	if err != nil {
		t.Fatal(err)
	}
	var count int
	for _, e := range entries {
		if e.Name() != "metas" && filepath.Ext(e.Name()) == ".gz" {
			count++
		}
	}
	if count != 1 {
		t.Errorf("expected 1 archive after idempotent store, got %d", count)
	}
}

func TestDevStore_PackageDirNotExist(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	err := cache.Store(filepath.Join(t.TempDir(), "does-not-exist"), "meta")
	if err == nil {
		t.Fatal("expected error when packageDir doesn't exist")
	}
}

func TestDevStore_InvalidNameVersion(t *testing.T) {
	cache := newTestDevArtifactCache(t)

	// A package dir whose last path segment is not name@version.
	pkgDir := filepath.Join(t.TempDir(), "packages", "no-at-sign")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := cache.Store(pkgDir, "meta"); err == nil {
		t.Fatal("expected error for package dir without @ in name")
	}
}

// ---- Restore tests ----

func TestDevRestore_CacheHit(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "gflags@2.2.2")

	const meta = "restore-meta"
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatal(err)
	}
	hash := computeHash(meta)

	// Remove the original package dir, then Restore should recreate it.
	if err := os.RemoveAll(pkgDir); err != nil {
		t.Fatal(err)
	}

	// Restore to a fresh location (simulating a new workspace).
	destDir := filepath.Join(t.TempDir(), "restored", "gflags@2.2.2")
	fromPath, err := cache.Restore("gflags@2.2.2", hash, destDir)
	if err != nil {
		t.Fatalf("Restore failed: %v", err)
	}
	if fromPath == "" {
		t.Fatal("expected cache hit, got empty path")
	}

	// Verify restored files exist.
	if !fileio.PathExists(filepath.Join(destDir, "libfoo.a")) {
		t.Error("libfoo.a should be restored")
	}
	if !fileio.PathExists(filepath.Join(destDir, "include", "foo.h")) {
		t.Error("include/foo.h should be restored")
	}
}

func TestDevRestore_CacheMiss(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	destDir := filepath.Join(t.TempDir(), "dest")

	fromPath, err := cache.Restore("nonexistent@1.0", "somehash", destDir)
	if err != nil {
		t.Fatalf("unexpected error on cache miss: %v", err)
	}
	if fromPath != "" {
		t.Errorf("expected empty path on cache miss, got %s", fromPath)
	}
}

func TestDevRestore_TamperedMetaFails(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "glog@0.6.0")

	const meta = "original-meta"
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatal(err)
	}
	hash := computeHash(meta)

	// Tamper with the meta file content — hash no longer matches content.
	metaPath := filepath.Join(cache.cacheDir, "glog@0.6.0", "metas", hash+".meta")
	if err := os.WriteFile(metaPath, []byte("tampered"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	destDir := filepath.Join(t.TempDir(), "dest")
	fromPath, err := cache.Restore("glog@0.6.0", hash, destDir)
	if err != nil {
		t.Fatalf("Restore should not error on tampered meta, just miss: %v", err)
	}
	if fromPath != "" {
		t.Error("expected cache miss when meta is tampered, got cache hit")
	}
}

func TestDevRestore_DifferentHashMisses(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "eigen@3.4.0")

	if err := cache.Store(pkgDir, "meta-content"); err != nil {
		t.Fatal(err)
	}

	// Restore with a wrong hash → miss.
	destDir := filepath.Join(t.TempDir(), "dest")
	fromPath, err := cache.Restore("eigen@3.4.0", "wrong-hash", destDir)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if fromPath != "" {
		t.Error("expected miss for wrong hash")
	}
}

func TestDevRestore_OverwritesExistingPackageDir(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "gflags@2.2.2")

	const meta = "overwrite-meta"
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatal(err)
	}
	hash := computeHash(meta)

	// Create a stale dest dir with old content.
	destDir := filepath.Join(t.TempDir(), "stale", "gflags@2.2.2")
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(destDir, "old.txt"), []byte("old"), 0o644); err != nil {
		t.Fatal(err)
	}

	fromPath, err := cache.Restore("gflags@2.2.2", hash, destDir)
	if err != nil {
		t.Fatalf("Restore failed: %v", err)
	}
	if fromPath == "" {
		t.Fatal("expected cache hit")
	}

	// old.txt should be gone (dest was removed and replaced).
	if fileio.PathExists(filepath.Join(destDir, "old.txt")) {
		t.Error("stale old.txt should have been removed by Restore")
	}
	// Restored content should be present.
	if !fileio.PathExists(filepath.Join(destDir, "libfoo.a")) {
		t.Error("libfoo.a should be restored")
	}
}

// ---- Round-trip: Store then Restore gives back identical content ----

func TestDevStoreRestore_RoundTrip(t *testing.T) {
	cache := newTestDevArtifactCache(t)
	pkgDir := makePackageDir(t, "boost@1.82.0")

	const meta = "round-trip-meta"
	if err := cache.Store(pkgDir, meta); err != nil {
		t.Fatal(err)
	}
	hash := computeHash(meta)

	destDir := filepath.Join(t.TempDir(), "roundtrip", "boost@1.82.0")
	if _, err := cache.Restore("boost@1.82.0", hash, destDir); err != nil {
		t.Fatal(err)
	}

	// Compare original and restored file content.
	orig, err := os.ReadFile(filepath.Join(pkgDir, "libfoo.a"))
	if err != nil {
		t.Fatal(err)
	}
	got, err := os.ReadFile(filepath.Join(destDir, "libfoo.a"))
	if err != nil {
		t.Fatal(err)
	}
	if string(orig) != string(got) {
		t.Errorf("restored content differs: got %q, want %q", got, orig)
	}
}
