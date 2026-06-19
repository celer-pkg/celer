package pkgcache

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
)

// ---- test fakes ----

type fakePkgCache struct {
	dir      string
	writable bool
}

func (f fakePkgCache) GetDir(dirType context.PkgCacheDirType) string {
	switch dirType {
	case context.PkgCacheDirRepos:
		return filepath.Join(f.dir, "repos")
	case context.PkgCacheDirArtifacts:
		return filepath.Join(f.dir, "artifacts")
	case context.PkgCacheDirDownloads:
		return filepath.Join(f.dir, "downloads")
	default:
		return f.dir
	}
}
func (f fakePkgCache) IsWritable() bool                         { return f.writable }
func (f fakePkgCache) GetArtifactCache() context.AritifactCache { return nil }
func (f fakePkgCache) GetRepoCache() context.RepoCache {
	return NewRepoConfig(fakeContext{pkgCache: f}, f.writable)
}

type fakeContext struct {
	pkgCache fakePkgCache
	offline  bool
}

func (fakeContext) Version() string                    { return "test" }
func (fakeContext) Platform() context.Platform         { return nil }
func (fakeContext) RootFS() context.RootFS             { return nil }
func (fakeContext) Project() context.Project           { return nil }
func (fakeContext) BuildType() string                  { return "release" }
func (fakeContext) LibraryFolder() string              { return "" }
func (fakeContext) Downloads() string                  { return "" }
func (fakeContext) Jobs() int                          { return 1 }
func (f fakeContext) Offline() bool                    { return f.offline }
func (fakeContext) Verbose() bool                      { return false }
func (fakeContext) InstalledDir() string               { return "" }
func (fakeContext) InstalledDevDir() string            { return "" }
func (f fakeContext) PkgCache() context.PkgCache       { return f.pkgCache }
func (fakeContext) ProxyHostPort() (string, int)       { return "", 0 }
func (fakeContext) CCacheEnabled() bool                { return false }
func (fakeContext) GenerateToolchainFile() error       { return nil }
func (fakeContext) ExprVars() *context.ExprVars        { return nil }
func (fakeContext) PythonConfig() context.PythonConfig { return nil }
func (fakeContext) Experiment() context.Experiment     { return nil }

// ---- helpers ----

// setupTestEnv creates a temp workspace, redirects dirs globals, creates a
// fake port entry so shouldCacheRepo returns true, and returns the paths
// needed by most tests. Caller must NOT call t.Cleanup for dirs.Init — it
// is handled automatically.
func setupTestEnv(t *testing.T) (tmpDir, pkgCacheDir, portName string) {
	t.Helper()
	oldWorkspace := dirs.WorkspaceDir
	tmpDir = t.TempDir()
	dirs.Init(tmpDir)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	pkgCacheDir = filepath.Join(tmpDir, "pkgcache")
	if err := os.MkdirAll(pkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	// Create a fake port so shouldCacheRepo returns true.
	portName = "x264"
	portPath := filepath.Join(dirs.PortsDir, "x", portName, "stable")
	if err := os.MkdirAll(filepath.Dir(portPath), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	return tmpDir, pkgCacheDir, portName
}

func newRepoConfig(pkgCacheDir string, writable bool) *RepoConfig {
	return NewRepoConfig(fakeContext{
		pkgCache: fakePkgCache{dir: pkgCacheDir, writable: writable},
	}, writable)
}

// createArchiveWithSingleDir creates a tar.gz archive that contains a single
// wrapping directory (e.g. lib-1.0/hello.txt). Returns archive path and SHA256.
func createArchiveWithSingleDir(t *testing.T, tmpDir, dirName, fileName, content string) (archivePath, checksum string) {
	t.Helper()
	srcDir := filepath.Join(tmpDir, "archive-src", dirName)
	if err := os.MkdirAll(srcDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(srcDir, fileName), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	archivePath = filepath.Join(tmpDir, dirName+".tar.gz")
	// includeFolder=true with srcDir makes tar archive the dirName folder itself,
	// so the archive root contains dirName/hello.txt (a single wrapping directory).
	if err := fileio.Targz(archivePath, srcDir, true); err != nil {
		t.Fatal(err)
	}

	sha, err := fileio.ComputeSHA256(archivePath)
	if err != nil {
		t.Fatal(err)
	}
	return archivePath, sha
}

// createArchiveFlat creates a tar.gz archive with files at root level (no wrapping dir).
func createArchiveFlat(t *testing.T, tmpDir, fileName, content string) (archivePath, checksum string) {
	t.Helper()
	srcDir := filepath.Join(tmpDir, "archive-src")
	if err := os.MkdirAll(srcDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(srcDir, fileName), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	archivePath = filepath.Join(tmpDir, "flat-archive.tar.gz")
	if err := fileio.Targz(archivePath, srcDir, false); err != nil {
		t.Fatal(err)
	}

	sha, err := fileio.ComputeSHA256(archivePath)
	if err != nil {
		t.Fatal(err)
	}
	return archivePath, sha
}

// createGitRepo sets up a minimal git repo and returns its directory and commit hash.
func createGitRepo(t *testing.T, tmpDir string) (repoDir, commitHash string) {
	t.Helper()
	repoDir = filepath.Join(tmpDir, "git-repo")
	if err := os.MkdirAll(repoDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(repoDir, "hello.txt"), []byte("hello-git"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := git.InitAsLocalRepo(repoDir, "init"); err != nil {
		t.Fatal(err)
	}
	hash, err := git.GetCommitHash(repoDir)
	if err != nil {
		t.Fatal(err)
	}
	return repoDir, hash
}

// ---- Store tests ----

func TestStore_ArchiveCopiesOriginal(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	archivePath, checksum := createArchiveFlat(t, tmpDir, "hello.txt", "hello")
	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")

	stored, err := rc.Store("x264@stable", "https://example.com/lib.tar.gz", repoDir, archivePath)
	if err != nil {
		t.Fatal(err)
	}

	// Stored path should use the original extension.
	expected := filepath.Join(pkgCacheDir, "repos", "x264@stable", checksum+".tar.gz")
	if stored != expected {
		t.Fatalf("expected stored path %s, got %s", expected, stored)
	}

	// The cached file should be a byte-for-byte copy of the original.
	origSHA, _ := fileio.ComputeSHA256(archivePath)
	cachedSHA, _ := fileio.ComputeSHA256(stored)
	if origSHA != cachedSHA {
		t.Fatalf("cached archive SHA256 mismatch: orig %s, cached %s", origSHA, cachedSHA)
	}
}

func TestStore_ArchiveIdempotent(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	archivePath, _ := createArchiveFlat(t, tmpDir, "hello.txt", "hello")
	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")

	if _, err := rc.Store("x264@stable", "https://example.com/lib.tar.gz", repoDir, archivePath); err != nil {
		t.Fatal(err)
	}

	// Second store should return ("", nil) without error.
	second, err := rc.Store("x264@stable", "https://example.com/lib.tar.gz", repoDir, archivePath)
	if err != nil {
		t.Fatal(err)
	}
	if second != "" {
		t.Fatalf("expected empty path on duplicate store, got %s", second)
	}

	// Only one cached file should exist.
	entries, _ := os.ReadDir(filepath.Join(pkgCacheDir, "repos", "x264@stable"))
	if len(entries) != 1 {
		t.Fatalf("expected 1 cached file, got %d", len(entries))
	}
}

func TestStore_ArchiveSkipsWhenFileNotExists(t *testing.T) {
	_, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	repoDir := "/nonexistent/src"
	archiveFile := "/nonexistent/archive.tar.gz"

	stored, err := rc.Store("x264@stable", "https://example.com/lib.tar.gz", repoDir, archiveFile)
	if err != nil {
		t.Fatal(err)
	}
	if stored != "" {
		t.Fatalf("expected empty path when archiveFile missing, got %s", stored)
	}
}

func TestStore_WritableFalse(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, false)

	archivePath, _ := createArchiveFlat(t, tmpDir, "hello.txt", "hello")
	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")

	stored, err := rc.Store("x264@stable", "https://example.com/lib.tar.gz", repoDir, archivePath)
	if err != nil {
		t.Fatal(err)
	}
	if stored != "" {
		t.Fatalf("expected empty path when not writable, got %s", stored)
	}
}

func TestStore_GitRepo(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	repoDir, commitHash := createGitRepo(t, tmpDir)
	repoURL := "https://example.com/lib.git"

	stored, err := rc.Store("x264@stable", repoURL, repoDir, "")
	if err != nil {
		t.Fatal(err)
	}

	expected := filepath.Join(pkgCacheDir, "repos", "x264@stable", commitHash+".tar.gz")
	if stored != expected {
		t.Fatalf("expected stored path %s, got %s", expected, stored)
	}
}

// ---- Restore tests ----

func TestRestore_ArchiveCorrectness(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	archivePath, checksum := createArchiveFlat(t, tmpDir, "hello.txt", "hello-from-archive")
	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")
	repoURL := "https://example.com/lib.tar.gz"

	// Store first.
	if _, err := rc.Store("x264@stable", repoURL, repoDir, archivePath); err != nil {
		t.Fatal(err)
	}

	// Restore into a clean dir.
	restored, err := rc.Restore("x264@stable", repoURL, repoDir, checksum)
	if err != nil {
		t.Fatal(err)
	}
	if restored == "" {
		t.Fatal("expected non-empty restored path")
	}

	content, err := os.ReadFile(filepath.Join(repoDir, "hello.txt"))
	if err != nil {
		t.Fatal(err)
	}
	if string(content) != "hello-from-archive" {
		t.Fatalf("unexpected restored content: %q", string(content))
	}
}

func TestRestore_ArchiveFlattensNestedDir(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	// Archive with a single wrapping directory "lib-1.0".
	archivePath, checksum := createArchiveWithSingleDir(t, tmpDir, "lib-1.0", "hello.txt", "nested-hello")
	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")
	repoURL := "https://example.com/lib-1.0.tar.gz"

	if _, err := rc.Store("x264@stable", repoURL, repoDir, archivePath); err != nil {
		t.Fatal(err)
	}

	restored, err := rc.Restore("x264@stable", repoURL, repoDir, checksum)
	if err != nil {
		t.Fatal(err)
	}
	if restored == "" {
		t.Fatal("expected non-empty restored path")
	}

	// After flattening, hello.txt should be directly in repoDir.
	content, err := os.ReadFile(filepath.Join(repoDir, "hello.txt"))
	if err != nil {
		t.Fatal(err)
	}
	if string(content) != "nested-hello" {
		t.Fatalf("unexpected restored content: %q", string(content))
	}

	// The wrapping directory should no longer exist.
	if fileio.PathExists(filepath.Join(repoDir, "lib-1.0")) {
		t.Fatal("expected wrapping directory to be removed after flattening")
	}
}

func TestRestore_ArchivePreservesIncludeDir(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	// Archive with a single directory named "include" — should NOT be flattened.
	srcDir := filepath.Join(tmpDir, "archive-src", "include")
	if err := os.MkdirAll(srcDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(srcDir, "header.h"), []byte("#pragma once"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	archivePath := filepath.Join(tmpDir, "include-archive.tar.gz")
	if err := fileio.Targz(archivePath, filepath.Join(tmpDir, "archive-src"), true); err != nil {
		t.Fatal(err)
	}
	checksum, err := fileio.ComputeSHA256(archivePath)
	if err != nil {
		t.Fatal(err)
	}

	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")
	repoURL := "https://example.com/include-lib.tar.gz"

	if _, err := rc.Store("x264@stable", repoURL, repoDir, archivePath); err != nil {
		t.Fatal(err)
	}

	if _, err := rc.Restore("x264@stable", repoURL, repoDir, checksum); err != nil {
		t.Fatal(err)
	}

	// "include" directory should still exist as a subdirectory (not flattened).
	if !fileio.PathExists(filepath.Join(repoDir, "include", "header.h")) {
		t.Fatal("expected include/header.h to be preserved (not flattened)")
	}
}

func TestRestore_CacheNotExists(t *testing.T) {
	_, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	restored, err := rc.Restore("x264@stable", "https://example.com/lib.tar.gz", "/tmp/unused", "abc123")
	if err != nil {
		t.Fatal(err)
	}
	if restored != "" {
		t.Fatalf("expected empty path when cache missing, got %s", restored)
	}
}

func TestRestore_EmptyChecksum(t *testing.T) {
	_, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	restored, err := rc.Restore("x264@stable", "https://example.com/lib.tar.gz", "/tmp/unused", "")
	if err != nil {
		t.Fatal(err)
	}
	if restored != "" {
		t.Fatalf("expected empty path with empty checksum, got %s", restored)
	}

	restored, err = rc.Restore("x264@stable", "https://example.com/lib.tar.gz", "/tmp/unused", "   ")
	if err != nil {
		t.Fatal(err)
	}
	if restored != "" {
		t.Fatalf("expected empty path with whitespace checksum, got %s", restored)
	}
}

func TestRestore_TamperedCacheFails(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	archivePath, checksum := createArchiveFlat(t, tmpDir, "hello.txt", "hello")
	repoDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")
	repoURL := "https://example.com/lib.tar.gz"

	stored, err := rc.Store("x264@stable", repoURL, repoDir, archivePath)
	if err != nil {
		t.Fatal(err)
	}

	// Replace cached archive with a different valid archive (same format, different content/SHA256).
	// Unlock the cached file so we can overwrite it.
	_ = os.Chmod(stored, 0644)

	differentSrc := filepath.Join(tmpDir, "tamper-src")
	if err := os.MkdirAll(differentSrc, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(differentSrc, "hello.txt"), []byte("tampered"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	differentArchive := filepath.Join(tmpDir, "tampered.tar.gz")
	if err := fileio.Targz(differentArchive, differentSrc, false); err != nil {
		t.Fatal(err)
	}
	if err := fileio.CopyFile(differentArchive, stored); err != nil {
		t.Fatal(err)
	}

	_, err = rc.Restore("x264@stable", repoURL, repoDir, checksum)
	if err == nil {
		t.Fatal("expected error when cached archive is tampered")
	}
	if !strings.Contains(err.Error(), "checksum mismatch") {
		t.Fatalf("expected checksum mismatch error, got: %s", err)
	}
}

func TestRestore_GitRepo(t *testing.T) {
	tmpDir, pkgCacheDir, _ := setupTestEnv(t)
	rc := newRepoConfig(pkgCacheDir, true)

	repoDir, commitHash := createGitRepo(t, tmpDir)
	repoURL := "https://example.com/lib.git"

	if _, err := rc.Store("x264@stable", repoURL, repoDir, ""); err != nil {
		t.Fatal(err)
	}

	restoreDir := filepath.Join(tmpDir, "buildtrees", "x264@stable", "src")
	restored, err := rc.Restore("x264@stable", repoURL, restoreDir, commitHash)
	if err != nil {
		t.Fatal(err)
	}
	if restored == "" {
		t.Fatal("expected non-empty restored path")
	}

	// Verify commit hash matches.
	restoredCommit, err := git.GetCommitHash(restoreDir)
	if err != nil {
		t.Fatal(err)
	}
	if restoredCommit != commitHash {
		t.Fatalf("expected commit %s, got %s", commitHash, restoredCommit)
	}
}
