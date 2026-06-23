package configs

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
)

// This file verifies repo-source caching from the caller's perspective:
// BuildConfig.Clone should store a source tree into pkgcache when online,
// and later restore that exact source tree from pkgcache before touching the
// remote source again.

type fakePkgCacheConfig struct {
	dir      string
	writable bool
}

func (f fakePkgCacheConfig) GetDir(dirType context.PkgCacheDirType) string {
	switch dirType {
	case context.PkgCacheDirRepos:
		return filepath.Join(f.dir, "repos")
	case context.PkgCacheDirArtifacts:
		return filepath.Join(f.dir, "artifacts-test")
	case context.PkgCacheDirDownloads:
		return filepath.Join(f.dir, "downloads")
	default:
		return f.dir
	}
}
func (f fakePkgCacheConfig) IsWritable() bool                         { return f.writable }
func (f fakePkgCacheConfig) GetCacheArtifacts() bool                  { return true }
func (f fakePkgCacheConfig) GetCacheDownloads() bool                  { return true }
func (f fakePkgCacheConfig) GetArtifactCache() context.AritifactCache { return nil }
func (f fakePkgCacheConfig) GetRepoCache() context.RepoCache {
	return pkgcache.NewRepoConfig(fakeContext{pkgCacheConfig: f}, f.writable)
}

// creates a local bare repo that acts like remote origin.
// Using a local origin keeps this test deterministic and network-independent.
// creates a local bare repo that acts like remote origin.
// Using a local origin keeps this test deterministic and network-independent.
// Returns: (originURL, commitHash)
func setupGitOriginRepo(t *testing.T, tmpWorkspace string) (string, string) {
	t.Helper()

	// repo-src is the editable working repository that will contain one commit.
	repoRoot := filepath.Join(tmpWorkspace, "repo-src")
	if err := os.MkdirAll(repoRoot, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(repoRoot, "hello.txt"), []byte("hello"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := git.InitAsLocalRepo(repoRoot, "init source repo"); err != nil {
		t.Fatal(err)
	}

	// Get the commit hash from the working repo before converting to bare.
	commitHash, err := git.GetCommitHash(repoRoot)
	if err != nil {
		t.Fatalf("failed to get commit hash: %v", err)
	}

	// BuildConfig.Clone expects to clone from a remote-like URL. A local bare
	// repository gives us that behavior without using the network.
	originURL := filepath.Join(tmpWorkspace, "x264.git")
	out, err := exec.Command("git", "clone", "--bare", repoRoot, originURL).CombinedOutput()
	if err != nil {
		t.Fatalf("git clone --bare failed: %v, output: %s", err, string(out))
	}

	return originURL, commitHash
}

func newBuildConfig(ctx context.Context, repoDir string) buildsystems.BuildConfig {
	return buildsystems.BuildConfig{
		Ctx: ctx,
		PortConfig: buildsystems.PortConfig{
			LibName:     "x264",
			LibVersion:  "stable",
			ProjectName: "proj",
			RepoDir:     repoDir,
		},
	}
}

func setupArchiveFile(t *testing.T, tmpWorkspace string) (archivePath string, archiveSha string) {
	t.Helper()

	// Create a tiny source tree and compress it into a local archive so the
	// archive-cache test stays deterministic and does not depend on downloads.
	srcRoot := filepath.Join(tmpWorkspace, "archive-src")
	if err := os.MkdirAll(srcRoot, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(srcRoot, "hello.txt"), []byte("hello-from-archive"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	archivePath = filepath.Join(tmpWorkspace, "x264-archive.tar.gz")
	if err := fileio.Targz(archivePath, srcRoot, false); err != nil {
		t.Fatal(err)
	}

	checksum, err := fileio.ComputeSHA256(archivePath)
	if err != nil {
		t.Fatal(err)
	}
	return archivePath, checksum
}

func TestBuildConfigClone_GitRepoCache(t *testing.T) {
	// Goal:
	// 1. An online git clone should be archived into repo pkgcache.
	// 2. A later clone of the same commit should restore from pkgcache before
	//    touching the remote repository.
	//
	// Workspace globals are redirected into a temp dir so the test does not
	// leak state into the developer's real workspace.
	oldWorkspace := dirs.WorkspaceDir
	tmpWorkspace := t.TempDir()
	dirs.Init(tmpWorkspace)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	pkgCacheDir := filepath.Join(tmpWorkspace, "pkgcache")
	if err := os.MkdirAll(pkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	originURL, expectedCommit := setupGitOriginRepo(t, tmpWorkspace)

	// Create fake ports/x/x264, so that repo can be cached.
	portPath := filepath.Join(dirs.PortsDir, "x", "x264", "stable")
	if err := os.MkdirAll(filepath.Dir(portPath), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	repoDir := filepath.Join(tmpWorkspace, "buildtrees", "x264@stable", "src")

	t.Run("store repo cache after clone", func(t *testing.T) {
		// This subtest only checks the write path:
		// Clone() should create the source tree and also create the repo cache
		// archive named by the resolved git commit.
		ctx := fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
			pkgCacheConfig: fakePkgCacheConfig{
				dir:      pkgCacheDir,
				writable: true,
			},
		}

		buildConfig := newBuildConfig(ctx, repoDir)
		buildConfig.PortConfig.Checksum = expectedCommit
		if err := buildConfig.Clone(originURL, "", "", 0); err != nil {
			t.Fatal(err)
		}

		// The cache key for git repos is the checked-out commit hash.
		commit, err := git.GetCommitHash(repoDir)
		if err != nil {
			t.Fatal(err)
		}

		archivePath := filepath.Join(pkgCacheDir, "repos", "x264@stable", commit+".tar.gz")
		if !fileio.PathExists(archivePath) {
			t.Fatalf("expected git repo cache archive: %s", archivePath)
		}
	})

	t.Run("restore from pkgcache", func(t *testing.T) {
		// First clone online once so pkgcache contains a known commit archive.
		onlineCtx := fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
			pkgCacheConfig: fakePkgCacheConfig{
				dir:      pkgCacheDir,
				writable: true,
			},
		}
		onlineBuildConfig := newBuildConfig(onlineCtx, repoDir)
		onlineBuildConfig.PortConfig.Checksum = expectedCommit
		if err := onlineBuildConfig.Clone(originURL, "", "", 0); err != nil {
			t.Fatal(err)
		}

		// Record the exact commit that should be restorable later.
		commit, err := git.GetCommitHash(repoDir)
		if err != nil {
			t.Fatal(err)
		}

		// Remove the working source tree. If the next Clone() succeeds, it cannot
		// be because the old directory was reused.
		if err := os.RemoveAll(repoDir); err != nil {
			t.Fatal(err)
		}

		// Remove the origin repository too. If the next Clone() succeeds, it
		// proves the restore happened from pkgcache before any remote access.
		if err := os.RemoveAll(originURL); err != nil {
			t.Fatal(err)
		}

		// writable=false is deliberate here: this call is only allowed to read
		// from cache, not write new cache entries.
		restoreCtx := fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
			pkgCacheConfig: fakePkgCacheConfig{
				dir:      pkgCacheDir,
				writable: false,
			},
		}
		restoreBuildConfig := newBuildConfig(restoreCtx, repoDir)
		restoreBuildConfig.PortConfig.Checksum = commit
		if err := restoreBuildConfig.Clone(originURL, commit, "", 0); err != nil {
			t.Fatal(err)
		}

		// Restoring from cache must give us the exact same checked-out commit.
		restoredCommit, err := git.GetCommitHash(repoDir)
		if err != nil {
			t.Fatal(err)
		}
		if restoredCommit != commit {
			t.Fatalf("expected restored commit %s, got %s", commit, restoredCommit)
		}
	})
}

func TestBuildConfigClone_ArchiveRepoCache(t *testing.T) {
	// Goal:
	// 1. An archive source should be unpacked once and then stored into repo
	//    pkgcache using the archive checksum as cache key.
	// 2. A later clone should restore that unpacked source tree from repo
	//    pkgcache even if the original archive file has been removed.
	//
	// If archive repo-cache support is intentionally disabled in the product,
	// this test should be removed or skipped rather than kept half-working.
	oldWorkspace := dirs.WorkspaceDir
	tmpWorkspace := t.TempDir()
	dirs.Init(tmpWorkspace)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	pkgCacheDir := filepath.Join(tmpWorkspace, "pkgcache")
	downloadsDir := filepath.Join(tmpWorkspace, "downloads")
	if err := os.MkdirAll(pkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(downloadsDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	archivePath, checksum := setupArchiveFile(t, tmpWorkspace)

	// Copy the archive into the downloads directory so Store can compute the
	// SHA256 of the original archive file. For remote URLs Repair would have
	// downloaded it there; for file:/// URLs Repair extracts directly without
	// copying to downloads, so we do it explicitly for the test.
	archiveBasename := filepath.Base(archivePath)
	if err := fileio.CopyFile(archivePath, filepath.Join(downloadsDir, archiveBasename)); err != nil {
		t.Fatal(err)
	}

	repoURL := fmt.Sprintf("file:///%s", archivePath)
	repoDir := filepath.Join(tmpWorkspace, "buildtrees", "x264@stable", "src")

	// First pass: unpack the archive and populate repo pkgcache.
	onlineCtx := fakeContext{
		platform:  "x86_64-linux",
		project:   "proj",
		build:     "release",
		downloads: downloadsDir,
		pkgCacheConfig: fakePkgCacheConfig{
			dir:      pkgCacheDir,
			writable: true,
		},
	}

	// Create fake ports/x264/archive, so that repo can be cached.
	portPath := filepath.Join(dirs.PortsDir, "x", "x264", "stable")
	if err := os.MkdirAll(filepath.Dir(portPath), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	onlineBuildConfig := newBuildConfig(onlineCtx, repoDir)
	onlineBuildConfig.PortConfig.Checksum = checksum
	if err := onlineBuildConfig.Clone(repoURL, checksum, "", 0); err != nil {
		t.Fatal(err)
	}

	if !fileio.PathExists(filepath.Join(repoDir, "hello.txt")) {
		t.Fatalf("expected archive extracted into %s", repoDir)
	}

	// Archive sources use the archive checksum as the cache key, and the cached
	// file preserves the original archive extension. Since setupArchiveFile
	// creates a .tar.gz, the cached path is <repo-name>/<sha>.tar.gz.
	cacheArchivePath := filepath.Join(pkgCacheDir, "repos", "x264@stable", checksum+".tar.gz")
	if !fileio.PathExists(cacheArchivePath) {
		t.Fatalf("expected archive repo cache exists: %s", cacheArchivePath)
	}

	// Make sure restore is truly from repo cache:
	// 1) remove extracted repo dir, 2) remove original local archive file,
	// 3) remove downloads archive copy.
	if err := os.RemoveAll(repoDir); err != nil {
		t.Fatal(err)
	}
	if err := os.Remove(archivePath); err != nil {
		t.Fatal(err)
	}
	downloadsArchivePath := filepath.Join(downloadsDir, archiveBasename)
	if err := os.Remove(downloadsArchivePath); err != nil {
		t.Fatal(err)
	}

	// Second pass: with both the working tree and original archive removed,
	// success now proves that Clone() restored from repo pkgcache.
	restoreCtx := fakeContext{
		platform:  "x86_64-linux",
		project:   "proj",
		build:     "release",
		downloads: downloadsDir,
		pkgCacheConfig: fakePkgCacheConfig{
			dir:      pkgCacheDir,
			writable: false,
		},
	}
	restoreBuildConfig := newBuildConfig(restoreCtx, repoDir)
	restoreBuildConfig.PortConfig.Checksum = checksum
	if err := restoreBuildConfig.Clone(repoURL, checksum, "", 0); err != nil {
		t.Fatal(err)
	}

	// Validate restored file contents, not just file existence.
	content, err := os.ReadFile(filepath.Join(repoDir, "hello.txt"))
	if err != nil {
		t.Fatal(err)
	}
	if string(content) != "hello-from-archive" {
		t.Fatalf("unexpected restored content: %q", string(content))
	}

	if !fileio.PathExists(downloadsArchivePath) {
		t.Fatalf("expected restored downloads archive exists: %s", downloadsArchivePath)
	}
	restoredChecksum, err := fileio.ComputeSHA256(downloadsArchivePath)
	if err != nil {
		t.Fatal(err)
	}
	if restoredChecksum != checksum {
		t.Fatalf("expected restored downloads archive checksum %s, got %s", checksum, restoredChecksum)
	}
}
