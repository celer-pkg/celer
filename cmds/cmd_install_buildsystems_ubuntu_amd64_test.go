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

const ubuntu_gcc_11_5 = "x86_64-linux-ubuntu-22.04-gcc-11.5"
const ubuntu_clang_21_1_4 = "x86_64-linux-ubuntu-22.04-clang-21.1.4"

func TestInstall_Buildsystems_Makefiles(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildPackage(t, "gcc", "x264@stable", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildPackage(t, ubuntu_gcc_11_5, "x264@stable", false)
	})

	t.Run("local_clang", func(t *testing.T) {
		buildPackage(t, "clang", "x264@stable", false)
	})

	t.Run("portable_clang", func(t *testing.T) {
		buildPackage(t, ubuntu_clang_21_1_4, "x264@stable", false)
	})
}

func TestInstall_Buildsystems_CMake(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildPackage(t, "gcc", "glog@0.6.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildPackage(t, ubuntu_gcc_11_5, "glog@0.6.0", false)
	})

	t.Run("local clang", func(t *testing.T) {
		buildPackage(t, "clang", "glog@0.6.0", false)
	})

	t.Run("portable_clang", func(t *testing.T) {
		buildPackage(t, ubuntu_clang_21_1_4, "glog@0.6.0", false)
	})
}

func TestInstall_Buildsystems_B2(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildPackage(t, "gcc", "boost@1.87.0", false)
	})

	t.Run("portbal_gcc", func(t *testing.T) {
		buildPackage(t, ubuntu_gcc_11_5, "boost@1.87.0", false)
	})

	t.Run("local_clang", func(t *testing.T) {
		buildPackage(t, "clang", "boost@1.87.0", false)
	})

	t.Run("portbal_clang", func(t *testing.T) {
		buildPackage(t, ubuntu_clang_21_1_4, "boost@1.87.0", false)
	})
}

func TestInstall_Buildsystems_Gyp(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildPackage(t, "gcc", "nss@3.55", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildPackage(t, ubuntu_gcc_11_5, "nss@3.55", false)
	})

	t.Run("local_clang", func(t *testing.T) {
		buildPackage(t, "clang", "nss@3.55", false)
	})

	t.Run("portable_clang", func(t *testing.T) {
		buildPackage(t, ubuntu_clang_21_1_4, "nss@3.55", false)
	})
}

func TestInstall_Buildsystems_Meson(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildPackage(t, "gcc", "pixman@0.44.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildPackage(t, ubuntu_gcc_11_5, "pixman@0.44.2", false)
	})

	t.Run("local_clang", func(t *testing.T) {
		buildPackage(t, "clang", "pixman@0.44.2", false)
	})

	t.Run("portable_clang", func(t *testing.T) {
		buildPackage(t, ubuntu_clang_21_1_4, "pixman@0.44.2", false)
	})
}

func TestInstall_Buildsystems_Prebuilt(t *testing.T) {
	buildPackage(t, ubuntu_gcc_11_5, "prebuilt-x264-single-target@stable", false)
}

func TestInstall_Buildsystems_Nobuild(t *testing.T) {
	buildPackage(t, ubuntu_gcc_11_5, "gnulib@master", true)
}

func buildPackage(t *testing.T, platform, nameVersion string, nobuild bool) {
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
