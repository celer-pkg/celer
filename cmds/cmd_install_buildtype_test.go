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

	var (
		platform = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
		project  = "project_test_install"
	)

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Setup build envs.
	check(celer.Platform().Setup())

	t.Run("install globally build type as Release", func(t *testing.T) {
		check(celer.SetBuildType("Release"))

		var (
			nameVersion = "eigen@3.4.0"
			platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageDir  = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion, platform, project, strings.ToLower(celer.BuildType()))
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion, celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
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

		var (
			nameVersion = "eigen@3.4.0"
			platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageDir  = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion, platform, project, strings.ToLower(celer.BuildType()))
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion, celer.BuildType()))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
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

		var (
			nameVersion = "eigen@3.4.0"
			buildType   = "Release"
			platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageDir  = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion, platform, project, strings.ToLower(buildType))
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion, buildType))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
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

		var (
			nameVersion = "eigen@3.4.0"
			buildType   = "Debug"
			platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageDir  = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion, platform, project, strings.ToLower(buildType))
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion, buildType))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
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
