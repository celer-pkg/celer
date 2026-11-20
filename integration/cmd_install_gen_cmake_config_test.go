package integration

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/dirs"
)

func TestInstall_Generate_CMake_Config_Single(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "prebuilt-x264@stable"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))

	var port configs.Port
	var options configs.InstallOptions
	check(t, port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(t, err)

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(t, os.RemoveAll(buildDir))
	check(t, os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() { os.RemoveAll(buildDir) })

	// Build test project.
	if err := buildtools.CheckTools(celer, "cmake"); err != nil {
		t.Fatal(err)
	}
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/single"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	// Clear up.
	check(t, port.Remove(configs.RemoveOptions{
		Purge:      true,
		BuildCache: true,
		Recursive:  true,
	}))
}

func TestInstall_Generate_CMake_Config_Interface(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "prebuilt-ffmpeg-interface@5.1.6"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))
	check(t, buildtools.CheckTools(celer, "git", "cmake"))

	var port configs.Port
	var options configs.InstallOptions
	check(t, port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(t, err)

	// Build test project.
	tmpFilesDir, err := dirs.NewTmpFilesDir()
	check(t, err)
	t.Cleanup(func() { os.RemoveAll(tmpFilesDir) })
	buildDir := filepath.Join(tmpFilesDir, "build_cmake_test")
	check(t, os.RemoveAll(buildDir))
	check(t, os.MkdirAll(buildDir, os.ModePerm))

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/interface"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	// Clear up.
	check(t, port.Remove(configs.RemoveOptions{
		Purge:      true,
		BuildCache: true,
		Recursive:  true,
	}))
}

func TestInstall_Generate_CMake_Config_Components(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "prebuilt-ffmpeg-components@5.1.6"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))
	check(t, buildtools.CheckTools(celer, "git", "cmake"))

	var port configs.Port
	var options configs.InstallOptions
	check(t, port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(t, err)

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(t, os.RemoveAll(buildDir))
	check(t, os.MkdirAll(buildDir, os.ModePerm))

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/components"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	// Clear up.
	check(t, port.Remove(configs.RemoveOptions{
		Purge:      true,
		BuildCache: true,
		Recursive:  true,
	}))
}

func TestInstall_Generate_CMake_Interface_Head_Only(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "prebuilt-eigen-interface@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))
	check(t, buildtools.CheckTools(celer, "git", "cmake"))

	var port configs.Port
	var options configs.InstallOptions
	check(t, port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(t, err)

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(t, os.RemoveAll(buildDir))
	check(t, os.MkdirAll(buildDir, os.ModePerm))

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/interface_head_only"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(t, executer.Execute())

	// Clear up.
	check(t, port.Remove(configs.RemoveOptions{
		Purge:      true,
		BuildCache: true,
		Recursive:  true,
	}))
}
