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

func TestInstall_BuildType(t *testing.T) {
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
	check(celer.SetProject("test_project_01"))

	// Setup build envs.
	check(celer.Platform().Setup())

	t.Run("install globally build type as Release", func(t *testing.T) {
		check(celer.SetBuildType("Release"))

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}

		// Check if installed.
		installed, err := port.Installed()
		check(err)

		if !installed {
			t.Fatal("package is not installed")
		}

		// Clean up.
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recurse:    true,
			BuildCache: true,
		}
		check(port.Remove(removeOptions))
	})

	t.Run("install globally build type as Debug", func(t *testing.T) {
		check(celer.SetBuildType("Debug"))

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@debug")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@debug")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}

		// Check if installed.
		installed, err := port.Installed()
		check(err)

		if !installed {
			t.Fatal("package is not installed")
		}

		// Clean up.
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recurse:    true,
			BuildCache: true,
		}
		check(port.Remove(removeOptions))
	})

	t.Run("install private build type as Release", func(t *testing.T) {
		check(celer.SetBuildType("Debug"))

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "eigen@3.4.0", "Release"))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}

		// Check if installed.
		installed, err := port.Installed()
		check(err)
		if !installed {
			t.Fatal("package is not installed")
		}

		// Clean up.
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recurse:    true,
			BuildCache: true,
		}
		check(port.Remove(removeOptions))
	})

	t.Run("install private build as Debug", func(t *testing.T) {
		check(celer.SetBuildType("Release"))

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "eigen@3.4.0", "Debug"))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@debug")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@debug")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}

		// Check if installed.
		installed, err := port.Installed()
		check(err)
		if !installed {
			t.Fatal("package is not installed")
		}

		// Clean up.
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recurse:    true,
			BuildCache: true,
		}
		check(port.Remove(removeOptions))
	})
}
