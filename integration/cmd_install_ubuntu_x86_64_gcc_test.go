//go:build linux && amd64 && test_gcc

package integration

import (
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/fileio"
)

const ubuntu_x86_64_gcc_11_5_0 = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"

func TestInstall_x86_64_GCC_Makefiles(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "x264@stable", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "x264@stable", false)
	})
}

func TestInstall_x86_64_MSVC_Makefiles_with_perl(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "openssl@1.1.1w", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "openssl@1.1.1w", false)
	})
}

func TestInstall_x86_64_GCC_CMake(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "glog@0.6.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "glog@0.6.0", false)
	})
}

func TestInstall_x86_64_GCC_B2(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "boost@1.91.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "boost@1.91.0", false)
	})
}

func TestInstall_x86_64_GCC_Gyp(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "nss@3.55", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "nss@3.55", false)
	})
}

func TestInstall_x86_64_GCC_Meson(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "pixman@0.44.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "pixman@0.44.2", false)
	})
}

func TestInstall_x86_64_GCC_Bazel(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "skcms@mirror", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "skcms@mirror", false)
	})
}

func TestInstall_x86_64_GCC_Custom(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, "", "qpOASES_e@3.1.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "qpOASES_e@3.1.2", false)
	})
}

func TestInstall_x86_64_GCC_Prebuilt(t *testing.T) {
	buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "prebuilt-x264@stable", false)
}

func TestInstall_x86_64_GCC_Nobuild(t *testing.T) {
	buildWithAMD64GCC(t, ubuntu_x86_64_gcc_11_5_0, "gnulib@1.0", true)
}

func buildWithAMD64GCC(t *testing.T, platform, nameVersion string, nobuild bool) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Init celer.
	const project = testProjectName
	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	if platform != "" {
		check(t, celer.SetPlatform(platform))
	} else {
		platform = celer.Platform().GetHostName()
	}
	check(t, celer.SetProject(project))

	var (
		packageFolder = filepath.Join(platform, project, celer.BuildType(), nameVersion)
		port          configs.Port
		options       = configs.InstallOptions{Prefer: configs.PreferSource}
	)

	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	// Check if package dir exists.
	if !nobuild {
		packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
		if !fileio.PathExists(packageDir) {
			t.Fatalf("package dir cannot found : %s", packageDir)
		}
	}

	// Check if installed.
	installed, err := port.Installed()
	check(t, err)
	if !installed {
		t.Fatal("package is not installed")
	}

	// Clean up.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(t, port.Remove(removeOptions))
}
