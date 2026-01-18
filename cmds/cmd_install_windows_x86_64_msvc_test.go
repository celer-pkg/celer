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

func TestInstall_x86_64_MSVC_Makefiles(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithAMD64MSVC(t, "", "x264@stable", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithAMD64MSVC(t, platform, "x264@stable", false)
	})
}

func TestInstall_x86_64_MSVC_CMake(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithAMD64MSVC(t, "", "gflags@2.2.2", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithAMD64MSVC(t, platform, "gflags@2.2.2", false)
	})
}

func TestInstall_x86_64_MSVC_B2(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithAMD64MSVC(t, "", "boost@1.87.0", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithAMD64MSVC(t, platform, "boost@1.87.0", false)
	})
}

func TestInstall_x86_64_MSVC_Meson(t *testing.T) {
	t.Run("detect_msvc", func(t *testing.T) {
		buildWithAMD64MSVC(t, "", "pkgconf@2.4.3", false)
	})

	t.Run("fixed_msvc", func(t *testing.T) {
		platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		buildWithAMD64MSVC(t, platform, "pkgconf@2.4.3", false)
	})
}

func TestInstall_x86_64_MSVC_Prebuilt(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
	buildWithAMD64MSVC(t, platform, "prebuilt-x264@stable", false)
}

func TestInstall_x86_64_MSVC_Nobuild(t *testing.T) {
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
	buildWithAMD64MSVC(t, platform, "gnulib@master", true)
}

func TestInstall_x86_64_MSVC_DevDependencies(t *testing.T) {
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
	platform := expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, "glslang@1.4.335.0"))

	// Install glslang (will automatically install its dev_dependencies)
	check(port.InstallFromSource(options))

	// Verify: the SPIRV-Tools-optConfig.cmake file should exist in the glslang's tmpDepsDir
	parentTmpDepsDir := filepath.Join(dirs.TmpDepsDir, fmt.Sprintf("%s@%s@%s",
		celer.Platform().GetName(), project, celer.BuildType()))

	// Find the SPIRV-Tools-optConfig.cmake file
	var foundFile string
	var foundFiles []string
	err := filepath.WalkDir(parentTmpDepsDir, func(path string, d os.DirEntry, err error) error {
		if err != nil {
			return nil
		}
		if !d.IsDir() && filepath.Base(path) == "SPIRV-Tools-optConfig.cmake" {
			rel, _ := filepath.Rel(parentTmpDepsDir, path)
			foundFile = rel
			foundFiles = append(foundFiles, rel)
		}
		return nil
	})
	check(err)

	if foundFile == "" {
		t.Errorf("SPIRV-Tools-optConfig.cmake should be found in the glslang's tmpDepsDir, but not found. Search directory: %s", parentTmpDepsDir)
	}

	// Verify: the file should not exist in the spirv-tools's own tmpDepsDir
	devTmpDepsDir := filepath.Join(dirs.TmpDepsDir, celer.Platform().GetHostName()+"-dev")
	var foundInDevTmpDeps bool
	err = filepath.WalkDir(devTmpDepsDir, func(path string, d os.DirEntry, err error) error {
		if err != nil {
			return nil
		}
		if !d.IsDir() && filepath.Base(path) == "SPIRV-Tools-optConfig.cmake" {
			foundInDevTmpDeps = true
		}
		return nil
	})
	check(err)

	if foundInDevTmpDeps {
		t.Errorf("SPIRV-Tools-optConfig.cmake should not exist in the dev port's own tmpDepsDir: %s", devTmpDepsDir)
	}

	// Clean up.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
}

func buildWithAMD64MSVC(t *testing.T, platform, nameVersion string, nobuild bool) {
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
