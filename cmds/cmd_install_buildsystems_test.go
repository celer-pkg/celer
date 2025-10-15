package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestInstall_Buildsystems(t *testing.T) {
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
	const projectName = "project_test_install"
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))
	check(celer.SetProject(projectName))

	// Setup build envs.
	check(celer.Platform().Setup())

	t.Run("install makefiles", func(t *testing.T) {
		var (
			nameVersion   = "x264@stable"
			platform      = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, strings.ToLower(celer.BuildType()))
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion, celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
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

	t.Run("install cmake", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "glog@0.6.0", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "glog@0.6.0@x86_64-windows-msvc-14.44@project_test_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "glog@0.6.0@x86_64-linux-ubuntu-22.04@project_test_01@release")
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

	t.Run("install b2", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "boost@1.87.0", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "boost@1.87.0@x86_64-windows-msvc-14.44@project_test_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "boost@1.87.0@x86_64-linux-ubuntu-22.04@project_test_01@release")
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

	t.Run("install gyp", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "nss@3.55", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "nss@3.55@x86_64-windows-msvc-14.44@project_test_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "nss@3.55@x86_64-linux-ubuntu-22.04@project_test_01@release")
		}
		if !fileio.PathExists(packageDir) {
			t.Fatal("package cannot found")
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

	t.Run("install meson", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "pixman@0.44.2", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		var packageDir string
		if runtime.GOOS == "windows" {
			packageDir = filepath.Join(dirs.PackagesDir, "pixman@0.44.2@x86_64-windows-msvc-14.44@project_test_01@release")
		} else {
			packageDir = filepath.Join(dirs.PackagesDir, "pixman@0.44.2@x86_64-linux-ubuntu-22.04@project_test_01@release")
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

	t.Run("install prebuilt", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "prebuilt-x264@stable", celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		packageDir := filepath.Join(dirs.PackagesDir, "prebuilt-x264@stable@x86_64-linux-ubuntu-22.04@project_test_02@release")
		if !fileio.PathExists(packageDir) {
			t.Fatal("package cannot found")
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

	t.Run("install nobuild", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, "gnulib@master", celer.BuildType()))
		check(port.InstallFromSource(options))

		if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
			t.Fatal("src dir cannot found")
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
