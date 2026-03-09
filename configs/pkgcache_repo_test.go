package configs

import (
	"celer/buildsystems"
	"celer/context"
	"celer/pkgcache"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
)

// This file verifies repo-source caching from the caller's perspective:
// BuildConfig.Clone should store a source tree into pkgcache when online,
// and later restore that exact source tree from pkgcache before touching the
// remote source again.
//
// The tests are intentionally end-to-end at the BuildConfig layer instead of
// testing pkgcache.Repo in isolation, because the behavior we care about is
// the integration between Clone(), pkgcache wiring, and the
// on-disk cache layout.

type fakePkgCache struct {
	dir      string
	writable bool
}

func (f fakePkgCache) GetDir() string                           { return f.dir }
func (f fakePkgCache) IsWritable() bool                         { return f.writable }
func (f fakePkgCache) GetArtifactCache() context.AritifactCache { return nil }
func (f fakePkgCache) GetRepoCache() context.RepoCache {
	return pkgcache.NewRepo(fakeContext{}, f.dir, f.writable)
}

// creates a local bare repo that acts like remote origin.
// Using a local origin keeps this test deterministic and network-independent.
func setupGitOriginRepo(t *testing.T, tmpWorkspace string) string {
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

	// BuildConfig.Clone expects to clone from a remote-like URL. A local bare
	// repository gives us that behavior without using the network.
	originURL := filepath.Join(tmpWorkspace, "x264.git")
	out, err := exec.Command("git", "clone", "--bare", repoRoot, originURL).CombinedOutput()
	if err != nil {
		t.Fatalf("git clone --bare failed: %v, output: %s", err, string(out))
	}

	return originURL
}

func newBuildConfig(ctx context.Context, repoDir string) buildsystems.BuildConfig {
	return buildsystems.BuildConfig{
		Ctx: ctx,
		PortConfig: buildsystems.PortConfig{
			LibName:     "x264",
			LibVersion:  "stable",
			ProjectName: "proj",
			RepoDir:     repoDir,
			CacheRepo:   true,
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

	sha, err := fileio.CalculateChecksum(archivePath)
	if err != nil {
		t.Fatal(err)
	}
	return archivePath, sha
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

	cacheDir := filepath.Join(tmpWorkspace, "pkgcache")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	originURL := setupGitOriginRepo(t, tmpWorkspace)
	repoDir := filepath.Join(tmpWorkspace, "buildtrees", "x264@stable", "src")

	t.Run("store repo cache after clone", func(t *testing.T) {
		// This subtest only checks the write path:
		// Clone() should create the source tree and also create the repo cache
		// archive named by the resolved git commit.
		onlineCtx := fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
			pkgCache: fakePkgCache{
				dir:      cacheDir,
				writable: true,
			},
		}

		buildConfig := newBuildConfig(onlineCtx, repoDir)
		if err := buildConfig.Clone(originURL, "", "", 0); err != nil {
			t.Fatal(err)
		}

		// The cache key for git repos is the checked-out commit hash.
		commit, err := git.GetCurrentCommit(repoDir)
		if err != nil {
			t.Fatal(err)
		}

		archivePath := filepath.Join(cacheDir, pkgcache.RepoCacheDir, "x264", commit+".tar.gz")
		if !fileio.PathExists(archivePath) {
			t.Fatalf("expected git repo cache archive: %s", archivePath)
		}
	})

	t.Run("restore from repo cache before remote access", func(t *testing.T) {
		// First clone online once so pkgcache contains a known commit archive.
		onlineCtx := fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
			pkgCache: fakePkgCache{
				dir:      cacheDir,
				writable: true,
			},
		}
		onlineBuildConfig := newBuildConfig(onlineCtx, repoDir)
		if err := onlineBuildConfig.Clone(originURL, "", "", 0); err != nil {
			t.Fatal(err)
		}

		// Record the exact commit that should be restorable later.
		commit, err := git.GetCurrentCommit(repoDir)
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
			pkgCache: fakePkgCache{
				dir:      cacheDir,
				writable: false,
			},
		}
		restoreBuildConfig := newBuildConfig(restoreCtx, repoDir)
		if err := restoreBuildConfig.Clone(originURL, commit, "", 0); err != nil {
			t.Fatal(err)
		}

		// Restoring from cache must give us the exact same checked-out commit.
		restoredCommit, err := git.GetCurrentCommit(repoDir)
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

	cacheDir := filepath.Join(tmpWorkspace, "pkgcache")
	downloadsDir := filepath.Join(tmpWorkspace, "downloads")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(downloadsDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	archivePath, archiveSha := setupArchiveFile(t, tmpWorkspace)
	repoURL := fmt.Sprintf("file:///%s", archivePath)
	repoDir := filepath.Join(tmpWorkspace, "buildtrees", "x264@archive", "src")

	// First pass: unpack the archive and populate repo pkgcache.
	onlineCtx := fakeContext{
		platform:  "x86_64-linux",
		project:   "proj",
		build:     "release",
		downloads: downloadsDir,
		pkgCache: fakePkgCache{
			dir:      cacheDir,
			writable: true,
		},
	}
	onlineBuildConfig := newBuildConfig(onlineCtx, repoDir)
	if err := onlineBuildConfig.Clone(repoURL, "file:"+archiveSha, "", 0); err != nil {
		t.Fatal(err)
	}

	if !fileio.PathExists(filepath.Join(repoDir, "hello.txt")) {
		t.Fatalf("expected archive extracted into %s", repoDir)
	}

	// Archive sources use the archive checksum as the cache key, so the stored
	// repo cache is expected to be <repo-name>/<sha>.tar.gz.
	cacheArchivePath := filepath.Join(cacheDir, pkgcache.RepoCacheDir, "x264-archive.tar.gz", archiveSha+".tar.gz")
	if !fileio.PathExists(cacheArchivePath) {
		t.Fatalf("expected archive repo cache exists: %s", cacheArchivePath)
	}

	// Make sure restore is truly from repo cache:
	// 1) remove extracted repo dir, 2) remove original local archive file.
	if err := os.RemoveAll(repoDir); err != nil {
		t.Fatal(err)
	}
	if err := os.Remove(archivePath); err != nil {
		t.Fatal(err)
	}

	// Second pass: with both the working tree and original archive removed,
	// success now proves that Clone() restored from repo pkgcache.
	restoreCtx := fakeContext{
		platform:  "x86_64-linux",
		project:   "proj",
		build:     "release",
		downloads: downloadsDir,
		pkgCache: fakePkgCache{
			dir:      cacheDir,
			writable: false,
		},
	}
	restoreBuildConfig := newBuildConfig(restoreCtx, repoDir)
	if err := restoreBuildConfig.Clone(repoURL, "file:"+archiveSha, "", 0); err != nil {
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
}
