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

func TestInstall_FromPackage(t *testing.T) {
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

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
		packageDir      = fmt.Sprintf("%s/%s@%s@%s@%s",
			dirs.PackagesDir, nameVersion, platform, project,
			celer.BuildType(),
		)
	)

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())
	check(buildtools.CheckTools(celer, "git"))

	t.Run("install success", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, nameVersion))
		check(port.InstallFromSource(options))

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
		check(port.Init(celer, nameVersion))
		check(port.InstallFromSource(options))

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
