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

const ubuntu_arm64_gcc_11_5_0 = "aarch64-linux-ubuntu-22.04-gcc-11.5.0"

func TestInstall_Makefiles_ARM64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, "", "x264@stable", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "x264@stable", false)
	})
}

func TestInstall_CMake_ARM64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, "", "glog@0.6.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "glog@0.6.0", false)
	})
}

func TestInstall_B2_ARM64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, "", "boost@1.87.0", false)
	})

	t.Run("portbal_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "boost@1.87.0", false)
	})
}

func TestInstall_Gyp_ARM64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, "", "nss@3.55", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "nss@3.55", false)
	})
}

func TestInstall_Meson_ARM64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, "", "pixman@0.44.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "pixman@0.44.2", false)
	})
}

func TestInstall_Prebuilt_ARM64_GCC(t *testing.T) {
	buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "prebuilt-x264@stable", false)
}

func TestInstall_Nobuild_ARM64_GCC(t *testing.T) {
	buildWithARM64GCC(t, ubuntu_arm64_gcc_11_5_0, "gnulib@master", true)
}

func buildWithARM64GCC(t *testing.T, platform, nameVersion string, nobuild bool) {
	if os.Getenv("TEST_ARM64_GCC") != "true" {
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
