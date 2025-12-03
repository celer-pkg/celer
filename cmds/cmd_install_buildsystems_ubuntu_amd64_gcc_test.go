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

const ubuntu_amd64_gcc_11_5_0 = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"

func TestInstall_Makefiles_AMD64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "x264@stable", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "x264@stable", false)
	})
}

func TestInstall_CMake_AMD64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "glog@0.6.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "glog@0.6.0", false)
	})
}

func TestInstall_B2_AMD64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "boost@1.87.0", false)
	})

	t.Run("portbal_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "boost@1.87.0", false)
	})
}

func TestInstall_Gyp_AMD64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "nss@3.55", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "nss@3.55", false)
	})
}

func TestInstall_Meson_AMD64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "pixman@0.44.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "pixman@0.44.2", false)
	})
}

func TestInstall_FreeStyle_AMD64_GCC(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "qpOASES_e@3.1.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "qpOASES_e@3.1.2", false)
	})
}

func TestInstall_Prebuilt_AMD64_GCC(t *testing.T) {
	buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "prebuilt-x264@stable", false)
}

func TestInstall_Nobuild_AMD64_GCC(t *testing.T) {
	buildWithAMD64GCC(t, ubuntu_amd64_gcc_11_5_0, "gnulib@master", true)
}

func buildWithAMD64GCC(t *testing.T, platform, nameVersion string, nobuild bool) {
	if os.Getenv("TEST_AMD64_GCC") != "true" {
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
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
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
			t.Fatalf("package dir cannot found : %s", packageDir)
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
