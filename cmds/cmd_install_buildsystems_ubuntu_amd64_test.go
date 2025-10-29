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

const platform_x86_64_ubuntu = "x86_64-linux-ubuntu-22.04-gcc-11.5"

func TestInstall_Buildsystems_Makefiles(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "x264@stable")
}

func TestInstall_Buildsystems_CMake(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "glog@0.6.0")
}

func TestInstall_Buildsystems_B2(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "boost@1.87.0")
}

func TestInstall_Buildsystems_Gyp(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "nss@3.55")
}

func TestInstall_Buildsystems_Meson(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "pixman@0.44.2")
}

func TestInstall_Buildsystems_Prebuilt(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "prebuilt-x264-single-target@stable")
}

func TestInstall_Buildsystems_PNobuild(t *testing.T) {
	buildPackage(t, platform_x86_64_ubuntu, "gnulib@master")
}

func buildPackage(t *testing.T, platform, nameVersion string) {
	const project = "project_test_install"

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
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Platform().Setup())

	var (
		packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, project, celer.BuildType())
		port          configs.Port
		options       configs.InstallOptions
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
}
