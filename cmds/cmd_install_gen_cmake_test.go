package cmds

import (
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

func TestInstall_Generate_CMake_Prebuilt_Single_Target(t *testing.T) {
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

	var (
		nameVersion = "prebuilt-x264@stable"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
		project     = "project_test_install"
	)

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(err)

	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() {
		check(os.RemoveAll(buildDir))
	})

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_prebuilt/single_target"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(executer.Execute())
}

func TestInstall_Generate_CMake_Prebuilt_Interface(t *testing.T) {
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

	var (
		nameVersion = "prebuilt-ffmpeg@5.1.6"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
		project     = "project_test_install"
	)

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "add_prebuilt_ffmpeg"))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(err)

	// Build test project.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() {
		check(os.RemoveAll(buildDir))
	})

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_prebuilt/interface"),
		"-B", buildDir,
	)
	executer.SetWorkDir(buildDir)
	check(executer.Execute())

	executer = cmd.NewExecutor("build test project", "cmake", "--build", buildDir)
	executer.SetWorkDir(buildDir)
	check(executer.Execute())
}
