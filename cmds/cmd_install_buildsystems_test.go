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
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, celer.BuildType())

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, nameVersion))
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
		var (
			nameVersion   = "glog@0.6.0"
			platform      = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, celer.BuildType())

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, nameVersion))
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

	t.Run("install b2", func(t *testing.T) {
		var (
			nameVersion   = "boost@1.87.0"
			platform      = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, celer.BuildType())

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, nameVersion))
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

	t.Run("install gyp", func(t *testing.T) {
		var (
			nameVersion   = "nss@3.55"
			platform      = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, celer.BuildType())

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, nameVersion))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
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
		var (
			nameVersion   = "pixman@0.44.2"
			platform      = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, celer.BuildType())

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, nameVersion))
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

	t.Run("install prebuilt", func(t *testing.T) {
		var (
			nameVersion   = "prebuilt-x264@stable"
			platform      = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
			packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, projectName, celer.BuildType())

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, "prebuilt-x264@stable"))
		check(port.InstallFromSource(options))

		// Check if package dir exists.
		packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
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
		var (
			nameVersion = "gnulib@master"

			port    configs.Port
			options configs.InstallOptions
		)

		check(port.Init(celer, nameVersion))
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
