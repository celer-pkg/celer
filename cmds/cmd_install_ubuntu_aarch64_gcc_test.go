//go:build linux && amd64 && test_aarch64_gcc

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

const ubuntu_aarch64_gcc_11_5_0 = "aarch64-linux-ubuntu-22.04-gcc-11.5.0"

func TestInstall_AArch64_GCC_Makefiles(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, "", "x264@stable", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "x264@stable", false)
	})
}

func TestInstall_AArch64_GCC_CMake(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, "", "glog@0.6.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "glog@0.6.0", false)
	})
}

func TestInstall_AArch64_GCC_B2(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, "", "boost@1.87.0", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "boost@1.87.0", false)
	})
}

func TestInstall_AArch64_GCC_Gyp(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, "", "nss@3.55", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "nss@3.55", false)
	})
}

func TestInstall_AArch64_GCC_Meson(t *testing.T) {
	t.Run("local_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, "", "pixman@0.44.2", false)
	})

	t.Run("portable_gcc", func(t *testing.T) {
		buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "pixman@0.44.2", false)
	})
}

func TestInstall_AArch64_GCC_Prebuilt(t *testing.T) {
	buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "prebuilt-x264@stable", false)
}

func TestInstall_AArch64_GCC_Nobuild(t *testing.T) {
	buildWithAArch64GCC(t, ubuntu_aarch64_gcc_11_5_0, "gnulib@master", true)
}

func TestInstall_AArch64_GCC_DevDependencies(t *testing.T) {
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
	check(celer.SetPlatform(ubuntu_aarch64_gcc_11_5_0))
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
		t.Errorf("SPIRV-Tools-optConfig.cmake 应该在 glslang 的 tmpDepsDir 中找到，但未找到。搜索目录: %s", parentTmpDepsDir)
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

func buildWithAArch64GCC(t *testing.T, platform, nameVersion string, nobuild bool) {
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
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
}
