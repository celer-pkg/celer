package configs

import (
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstall(t *testing.T) {
	// Check error.
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

	// Clear exist config files.
	check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "conf")))
	check(os.Remove(filepath.Join(dirs.WorkspaceDir, "celer.toml")))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SyncConf("https://github.com/celer-pkg/test-conf.git", ""))

	// Change platform
	if runtime.GOOS == "windows" {
		check(celer.ChangePlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.ChangePlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.ChangeProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "x264@stable", celer.BuildType()))
	check(port.installFromSource())
}
