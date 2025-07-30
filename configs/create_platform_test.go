package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"testing"
)

func TestCreate_Platform(t *testing.T) {
	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(dirs.ParentDir(currentDir, 1))

	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}

	const platformName = "x86_64-linux-ubuntu-test"
	if err := celer.CreatePlatform(platformName); err != nil {
		t.Fatal(err)
	}

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
	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(dirs.ParentDir(currentDir, 1))

	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}

	if err := celer.CreatePlatform(""); err == nil {
		t.Fatal("it should be failed")
	}
}
