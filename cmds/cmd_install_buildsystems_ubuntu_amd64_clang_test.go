//go:build linux && amd64

package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

const ubuntu_clang_21_1_4 = "x86_64-linux-ubuntu-22.04-clang-21.1.4"

func TestInstall_Makefiles_Clang(t *testing.T) {
	buildWithClang(t, ubuntu_clang_21_1_4, "x264@stable", false)
}

func TestInstall_CMake_Clang(t *testing.T) {
	buildWithClang(t, ubuntu_clang_21_1_4, "glog@0.6.0", false)
}

func TestInstall_B2_Clang(t *testing.T) {
	buildWithClang(t, ubuntu_clang_21_1_4, "boost@1.87.0", false)
}

func TestInstall_Gyp_Clang(t *testing.T) {
	buildWithClang(t, ubuntu_clang_21_1_4, "nss@3.55", false)
}

func TestInstall_Meson_Clang(t *testing.T) {
	buildWithClang(t, ubuntu_clang_21_1_4, "pixman@0.44.2", false)
}

func TestInstall_FreeStyle_AMD64_Clang(t *testing.T) {
	buildWithClang(t, ubuntu_clang_21_1_4, "qpOASES_e@3.1.2", false)
}

func TestInstall_Prebuilt_Clang(t *testing.T) {
	buildWithClang(t, "clang", "prebuilt-x264@stable", false)
}

func TestInstall_Nobuild_Clang(t *testing.T) {
	buildWithClang(t, "clang", "gnulib@master", true)
}

func buildWithClang(t *testing.T, platform, nameVersion string, nobuild bool) {
	if os.Getenv("TEST_CLANG") != "ON" {
		t.SkipNow()
	}

	const project = "project_test_install"

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Setup test environment.
	dirs.Init(t.TempDir())

	// Cleanup function.
	cleanup := func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
		check(os.RemoveAll(dirs.ConfDir))
	}
	t.Cleanup(cleanup)

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	if platform != "" {
		check(celer.SetPlatform(platform))
	} else {
		platform = celer.Platform().GetHostName()
	}
	check(celer.SetProject(project))
	check(celer.Setup())

	var (
		packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, project, celer.BuildType())
		port          configs.Port
		options       configs.InstallOptions
	)

	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Check if package dir exists.
	if !nobuild {
		packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}
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
}
