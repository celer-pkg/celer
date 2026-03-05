package configs

import (
	"celer/buildsystems"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
)

type fakePackageCache struct {
	dir      string
	writable bool
}

func (f fakePackageCache) GetDir() string                                       { return f.dir }
func (f fakePackageCache) IsWritable() bool                                     { return f.writable }
func (f fakePackageCache) Read(nameVersion, hash, destDir string) (bool, error) { return false, nil }
func (f fakePackageCache) Write(packageDir, meta string) error                  { return nil }

func TestBuildConfigClone_GitRepoCache_OfflineRestore(t *testing.T) {
	oldWorkspace := dirs.WorkspaceDir
	tmpWorkspace := t.TempDir()
	dirs.Init(tmpWorkspace)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	cacheDir := filepath.Join(tmpWorkspace, "package-cache")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

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

	repoDir := filepath.Join(tmpWorkspace, "buildtrees", "x264@stable", "src")
	onlineCtx := fakeContext{
		platform: "x86_64-linux",
		project:  "proj",
		build:    "release",
		packageCache: fakePackageCache{
			dir:      cacheDir,
			writable: true,
		},
	}
	buildConfig := buildsystems.BuildConfig{
		Ctx: onlineCtx,
		PortConfig: buildsystems.PortConfig{
			LibName:     "x264",
			LibVersion:  "stable",
			ProjectName: "proj",
			RepoDir:     repoDir,
		},
	}

	if err := buildConfig.Clone(originURL, "", "", "", 0); err != nil {
		t.Fatal(err)
	}

	commit, err := git.GetCurrentCommit(repoDir)
	if err != nil {
		t.Fatal(err)
	}

	archivePath := filepath.Join(cacheDir, "repos", fmt.Sprintf("x264-%s.tar.gz", commit))
	if !fileio.PathExists(archivePath) {
		t.Fatalf("expected git repo cache archive: %s", archivePath)
	}

	if err := os.RemoveAll(repoDir); err != nil {
		t.Fatal(err)
	}

	offlineCtx := fakeContext{
		platform: "x86_64-linux",
		project:  "proj",
		build:    "release",
		offline:  true,
		packageCache: fakePackageCache{
			dir:      cacheDir,
			writable: false,
		},
	}
	offlineBuildConfig := buildsystems.BuildConfig{
		Ctx: offlineCtx,
		PortConfig: buildsystems.PortConfig{
			LibName:     "x264",
			LibVersion:  "stable",
			ProjectName: "proj",
			RepoDir:     repoDir,
		},
	}
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
}
