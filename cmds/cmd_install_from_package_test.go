package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstall_FromPackage(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))
	check(celer.SetProject("project_test_01"))

	// Setup build envs.
	check(celer.Platform().Setup())

	t.Run("install success", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
		check(port.InstallFromSource(options))

		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@project_test_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@project_test_01@release")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package cannot found")
		}

		removeOptions := configs.RemoveOptions{
			Purge:      false,
			Recurse:    true,
			BuildCache: true,
		}
		check(port.Remove(removeOptions))

		installed, err := port.InstallFromPackage(options)
		check(err)

		t.Cleanup(func() {
			removeOptions := configs.RemoveOptions{
				Purge:      true,
				Recurse:    true,
				BuildCache: true,
			}
			check(port.Remove(removeOptions))
		})

		if !installed {
			t.Fatal("should not be successfully installed from package")
		}
	})

	t.Run("install failed", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
		check(port.InstallFromSource(options))

		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@project_test_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@project_test_01@release")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package cannot found")
		}
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recurse:    true,
			BuildCache: true,
		}
		check(port.Remove(removeOptions))

		installed, err := port.InstallFromPackage(options)
		check(err)
		if installed {
			t.Fatal("it should be failed to install from package.")
		}
	})
}
