package configs

import (
	"celer/pkgs/dirs"
	"os"
	"runtime"
	"testing"
)

func TestInstall(t *testing.T) {
	// Convenient check function.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(dirs.ParentDir(currentDir, 1))

	celer := NewCeler()
	if runtime.GOOS == "windows" {
		check(celer.ChangePlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.ChangePlatform("x86_64-linux-ubuntu-22.04"))
	}
	check(celer.Init())

	var port Port
	check(port.Init(celer, "x264@stable", celer.BuildType()))

	t.Run("install from source", func(t *testing.T) {
		check(port.installFromSource())
	})
}
