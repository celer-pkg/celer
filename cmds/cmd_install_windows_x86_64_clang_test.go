//go:build windows && amd64 && test_clang_cl

package cmds

import (
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func TestInstall_x86_64_Clang_CMake(t *testing.T) {
	buildWithAMD64Clang(t, "x86_64-windows-clang-21.1.4", "gflags@2.2.2", false)
}

// TODO: it works in local but fails in test.
// func TestInstall_B2_x86_64_Clang(t *testing.T) {
// 	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-clang-cl-enterprise-14", "x86_64-windows-clang-cl-community-14")
// 	buildWithAMD64Clang(t, platform, "boost@1.91.0", false)
// }

func TestInstall_x86_64_Clang_Meson(t *testing.T) {
	buildWithAMD64Clang(t, "x86_64-windows-clang-21.1.4", "pkgconf@2.4.3", false)
}

func TestInstall_x86_64_Clang_Prebuilt(t *testing.T) {
	buildWithAMD64Clang(t, "x86_64-windows-clang-21.1.4", "prebuilt-x264@stable", false)
}

func TestInstall_x86_64_Clang_Nobuild(t *testing.T) {
	buildWithAMD64Clang(t, "x86_64-windows-clang-21.1.4", "gnulib@1.0", true)
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

	// Init celer
	const project = "project_test_install"
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	if platform != "" {
		check(celer.SetPlatform(platform))
	} else {
		platform = celer.Platform().GetHostName()
	}

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
		packageFolder := filepath.Join(platform, project, celer.BuildType(), nameVersion)
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
