//go:build linux && amd64 && test_clang

package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"path/filepath"
	"testing"
)

const ubuntu_x86_64_clang_21_1_4 = "x86_64-linux-ubuntu-22.04-clang-21.1.4"

func TestInstall_x86_64_Clang_Makefiles(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "x264@stable", false)
}

func TestInstall_x86_64_Clang_CMake(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "glog@0.6.0", false)
}

func TestInstall_x86_64_Clang_B2(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "boost@1.87.0", false)
}

func TestInstall_x86_64_Clang_Meson(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "pixman@0.44.2", false)
}

func TestInstall_x86_64_Clang_Custom(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "qpOASES_e@3.1.2", false)
}

func TestInstall_x86_64_Clang_Prebuilt(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "prebuilt-x264@stable", false)
}

func TestInstall_x86_64_Clang_Nobuild(t *testing.T) {
	buildWithAMD64Clang(t, ubuntu_x86_64_clang_21_1_4, "gnulib@master", true)
}

func buildWithAMD64Clang(t *testing.T, platform, nameVersion string, nobuild bool) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	const project = "project_test_install"
	celer := configs.NewCeler()
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.InitWithPlatform(platform))
	check(celer.SetBuildType("Release"))
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
			t.Fatal("package dir cannot found: " + packageDir)
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
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
}
