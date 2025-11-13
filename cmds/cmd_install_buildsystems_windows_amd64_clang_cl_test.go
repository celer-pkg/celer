//go:build windows && amd64

package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

func TestInstall_CMake_Clang(t *testing.T) {
	t.Run("detect_clang_cl", func(t *testing.T) {
		buildWithClang(t, "clang-cl", "gflags@2.2.2", false)
	})

	t.Run("fixed_clang_cl", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
		buildWithClang(t, platform, "gflags@2.2.2", false)
	})
}

// func TestInstall_B2_Clang(t *testing.T) {
// 	t.Run("detect_clang", func(t *testing.T) {
// 		buildWithClang(t, "clang-cl", "boost@1.87.0", false)
// 	})

// 	t.Run("fixed_clang", func(t *testing.T) {
// 		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
// 		buildWithClang(t, platform, "boost@1.87.0", false)
// 	})
// }

func TestInstall_Meson_Clang(t *testing.T) {
	t.Run("detect_clang_cl", func(t *testing.T) {
		buildWithClang(t, "clang-cl", "pkgconf@2.4.3", false)
	})

	t.Run("fixed_clang_cl", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
		buildWithClang(t, platform, "pkgconf@2.4.3", false)
	})
}

func TestInstall_Prebuilt_Clang(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
	buildWithClang(t, platform, "prebuilt-x264@stable", false)
}

func TestInstall_Nobuild_Clang(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
	buildWithClang(t, platform, "gnulib@master", true)
}

func buildWithClang(t *testing.T, platform, nameVersion string, nobuild bool) {
	if os.Getenv("TEST_CLANG_CL") != "true" {
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

	// Init celer
	celer := configs.NewCeler()

	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/add_test_cases_for_windows_clang"))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var (
		port    configs.Port
		options configs.InstallOptions
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
		if platform == "clang-cl" || platform == "msvc" {
			platform = "x86_64-windows"
		}
		packageFolder := fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, project, celer.BuildType())
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
