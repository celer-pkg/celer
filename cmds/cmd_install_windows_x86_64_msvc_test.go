//go:build windows && amd64 && test_msvc

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

func TestInstall_Makefiles_x86_64_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithX86_64MSVC(t, "", "x264@stable", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithX86_64MSVC(t, platform, "x264@stable", false)
	})
}

func TestInstall_CMake_x86_64_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithX86_64MSVC(t, "", "gflags@2.2.2", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithX86_64MSVC(t, platform, "gflags@2.2.2", false)
	})
}

func TestInstall_B2_x86_64_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithX86_64MSVC(t, "", "boost@1.87.0", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithX86_64MSVC(t, platform, "boost@1.87.0", false)
	})
}

func TestInstall_Meson_x86_64_MSVC(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithX86_64MSVC(t, "", "pkgconf@2.4.3", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithX86_64MSVC(t, platform, "pkgconf@2.4.3", false)
	})
}

func TestInstall_Prebuilt_x86_64_MSVC(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
	buildWithX86_64MSVC(t, platform, "prebuilt-x264@stable", false)
}

func TestInstall_Nobuild_x86_64_MSVC(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
	buildWithX86_64MSVC(t, platform, "gnulib@master", true)
}

func buildWithX86_64MSVC(t *testing.T, platform, nameVersion string, nobuild bool) {
	if os.Getenv("TEST_MSVC") != "true" {
		t.SkipNow()
	}

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
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
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
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
}
