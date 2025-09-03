package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"testing"
)

func TestCreate_Platform(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))

	const platformName = "x86_64-linux-ubuntu-test"
	check(celer.CreatePlatform(platformName))

	// Check if platform really created.
	platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
	if !fileio.PathExists(platformPath) {
		t.Fatalf("platform %s should be created", platformName)
	}

	check(os.RemoveAll(platformPath))
}

func TestCreate_Platform_EmptyName(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))

	if err := celer.CreatePlatform(""); err == nil {
		t.Fatal("it should be failed")
	}

	check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
}
