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

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SyncConf("https://github.com/celer-pkg/test-conf.git", ""))

	const platformName = "x86_64-linux-ubuntu-test"
	check(celer.CreatePlatform(platformName))

	// Check if platform really created.
	platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
	if !fileio.PathExists(platformPath) {
		t.Fatalf("platform %s should be created", platformName)
	}

	// Cleanup.
	t.Cleanup(func() {
		if err := os.Remove(platformPath); err != nil {
			t.Fatal(err)
		}
	})
}

func TestCreate_Platform_EmptyName(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SyncConf("https://github.com/celer-pkg/test-conf.git", ""))

	if err := celer.CreatePlatform(""); err == nil {
		t.Fatal("it should be failed")
	}
}
