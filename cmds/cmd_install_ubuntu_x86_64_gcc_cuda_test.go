//go:build linux && amd64 && test_gcc

package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstall_Cuda(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "cuda_toolkit@12.9.1"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(err)

	// Regenerate toolchain file after CUDA installation so it can detect installed CUDA files.
	check(celer.GenerateToolchainFile())

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.RemoveAll(buildDir))
	check(os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() { os.RemoveAll(buildDir) })

	// Build test project.
	if err := buildtools.CheckTools(celer, "cmake"); err != nil {
		t.Fatal(err)
	}

	installedDir := filepath.Join(dirs.InstalledDir,
		fmt.Sprintf("%s@%s@%s",
			celer.Platform().GetName(),
			celer.Project().GetName(),
			celer.BuildType()))

	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("TMP_DEP_DIR=%s", installedDir),
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/cuda_test"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(executer.Execute())

	// Clear up.
	check(port.Remove(configs.RemoveOptions{
		Purge:      true,
		BuildCache: true,
		Recursive:  true,
	}))
}
