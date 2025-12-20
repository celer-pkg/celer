//go:build windows && amd64 && test_clang_cl

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
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
	buildWithClang(t, platform, "gflags@2.2.2", false)
}

func TestInstall_B2_Clang(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
	buildWithClang(t, platform, "boost@1.87.0", false)
}

func TestInstall_Meson_Clang(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14.44", "x86_64-windows-clang-cl-community-14.44")
	buildWithClang(t, platform, "pkgconf@2.4.3", false)
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer
	const project = "project_test_install"
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	if platform != "" {
		check(celer.SetPlatform(platform))
	} else {
		platform = celer.Platform().GetHostName()
	}
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
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
}
