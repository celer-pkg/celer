package cmds

import (
	"celer/buildtools"
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

func TestInstall_BuildType(t *testing.T) {
	fmt.Printf("-- GITHUB_ACTIONS: %s\n", expr.If(os.Getenv("GITHUB_ACTIONS") != "", os.Getenv("GITHUB_ACTIONS"), "false"))

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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())
	check(buildtools.CheckTools(celer, "git"))

	t.Run("install with build type Release", func(t *testing.T) {
		check(celer.SetBuildType("Release"))

		var (
			nameVersion     = "eigen@3.4.0"
			windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
			platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
			packageDir      = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion, platform, project, celer.BuildType())
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion))
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

	t.Run("install with build type Debug", func(t *testing.T) {
		check(celer.SetBuildType("Debug"))

		var (
			nameVersion     = "eigen@3.4.0"
			windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
			platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
			packageDir      = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion, platform, project, celer.BuildType())
		)

		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion))
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
