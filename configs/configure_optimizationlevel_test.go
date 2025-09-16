package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"errors"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestConfigure_OptimizationLevel(t *testing.T) {
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
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))
	check(celer.SetOffline(true))

	if celer.Global.Offline != true {
		t.Fatalf("offline should be `true`")
	}

	if fileio.PathExists(dirs.DownloadedDir) {
		check(os.Rename(dirs.DownloadedDir, dirs.DownloadedDir+".bak"))
	}

	t.Cleanup(func() {
		check(os.RemoveAll(dirs.DownloadedDir))
		if fileio.PathExists(dirs.DownloadedDir + ".bak") {
			check(os.Rename(dirs.DownloadedDir+".bak", dirs.DownloadedDir))
		}
	})

	if err := celer.Platform().Setup(); err == nil || !errors.Is(err, fileio.ErrOffline) {
		t.Fatal("setup should fail due to offline")
	}
}
