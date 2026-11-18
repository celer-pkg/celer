//go:build linux && amd64 && test_gcc

package cmds

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

func TestInstall_x86_64_GCC_CUDA(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "cuda_toolkit@12.9.1"
		platform    = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
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

	// Regenerate toolchain file after CUDA installation so it can detect installed CUDA files.
	check(t, celer.GenerateToolchainFile())

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(t, os.RemoveAll(buildDir))
	check(t, os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() { os.RemoveAll(buildDir) })

	// Build test project.
	if err := buildtools.CheckTools(celer, "cmake"); err != nil {
		t.Fatal(err)
	}

	installedDir := filepath.Join(dirs.InstalledDir,
		celer.Platform().GetName(),
		celer.Project().GetName(),
		celer.BuildType())

	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("STAGING_DIR=%s", installedDir),
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/cuda_test"),
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
