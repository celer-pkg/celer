package configs

import (
	"celer/buildsystems"
	"celer/context"
	"celer/pkgcache"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
)

type fakePkgCache struct {
	dir      string
	writable bool
}

func (f fakePkgCache) GetDir() string                           { return f.dir }
func (f fakePkgCache) IsWritable() bool                         { return f.writable }
func (f fakePkgCache) GetArtifactCache() context.AritifactCache { return nil }
func (f fakePkgCache) GetRepoCache() context.RepoCache          { return pkgcache.NewRepo(f.dir, f.writable) }

// creates a local bare repo that acts like remote origin.
// Using a local origin keeps this test deterministic and network-independent.
func setupGitOriginRepo(t *testing.T, tmpWorkspace string) string {
	t.Helper()

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

func TestBuildConfigClone_GitRepoCache(t *testing.T) {
	// Redirect workspace globals to a temp dir so test side effects stay isolated.
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
		if err := buildConfig.Clone(originURL, "", "", "", 0); err != nil {
			t.Fatal(err)
		}

		commit, err := git.GetCurrentCommit(repoDir)
		if err != nil {
			t.Fatal(err)
		}

		archivePath := filepath.Join(cacheDir, pkgcache.RepoCacheDir, "x264", commit+".tar.gz")
		if !fileio.PathExists(archivePath) {
			t.Fatalf("expected git repo cache archive: %s", archivePath)
		}
	})

	t.Run("offline restore from repo cache", func(t *testing.T) {
		// First clone online once to seed repo cache for a known commit.
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
		if err := onlineBuildConfig.Clone(originURL, "", "", "", 0); err != nil {
			t.Fatal(err)
		}

		commit, err := git.GetCurrentCommit(repoDir)
		if err != nil {
			t.Fatal(err)
		}

		// Remove source tree so offline clone must restore from cache instead of reusing local dir.
		if err := os.RemoveAll(repoDir); err != nil {
			t.Fatal(err)
		}

		offlineCtx := fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
			offline:  true,
			pkgCache: fakePkgCache{
				dir:      cacheDir,
				writable: false,
			},
		}
		offlineBuildConfig := newBuildConfig(offlineCtx, repoDir)
		if err := offlineBuildConfig.Clone(originURL, "", commit, "", 0); err != nil {
			t.Fatal(err)
		}

		restoredCommit, err := git.GetCurrentCommit(repoDir)
		if err != nil {
			t.Fatal(err)
		}
		if restoredCommit != commit {
			t.Fatalf("expected restored commit %s, got %s", commit, restoredCommit)
		}
	})
}
