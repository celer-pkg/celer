//go:build windows && amd64

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

const windows_msvc_14_44 = "x86_64-windows-msvc-14.44"

func TestInstall_Makefiles_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithMSVC(t, "", "x264@stable", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		buildWithMSVC(t, windows_msvc_14_44, "x264@stable", false)
	})
}

func TestInstall_CMake_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithMSVC(t, "", "gflags@2.2.2", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		buildWithMSVC(t, windows_msvc_14_44, "gflags@2.2.2", false)
	})
}

func TestInstall_B2_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithMSVC(t, "", "boost@1.87.0", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		buildWithMSVC(t, windows_msvc_14_44, "boost@1.87.0", false)
	})
}

func TestInstall_Meson_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithMSVC(t, "", "pkgconf@2.4.3", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		buildWithMSVC(t, windows_msvc_14_44, "pkgconf@2.4.3", false)
	})
}

func TestInstall_Prebuilt_MSVC(t *testing.T) {
	buildWithMSVC(t, windows_msvc_14_44, "prebuilt-x264-single-target@stable", false)
}

func TestInstall_Nobuild_MSVC(t *testing.T) {
	buildWithMSVC(t, windows_msvc_14_44, "gnulib@master", true)
}

func buildWithMSVC(t *testing.T, platform, nameVersion string, nobuild bool) {
	if os.Getenv("GO_TEST_MSVC") != "TEST_MSVC" {
		//t.SkipNow()
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
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))
	if platform != "" {
		check(celer.SetPlatform(platform))
	} else {
		platform = "x86_64-windows"
	}
	check(celer.SetProject(project))
	check(celer.Platform().Setup())

	var (
		packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, project, celer.BuildType())
		port          configs.Port
		options       configs.InstallOptions
	)

	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	// Check if package dir exists.
	if !nobuild {
		packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
		if !fileio.PathExists(packageDir) {
			t.Fatalf("package dir cannot found, expect: %s", packageDir)
		}
	}

	// Clean up.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recurse:    true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
}
