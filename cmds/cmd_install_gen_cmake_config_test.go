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

func TestInstall_Generate_CMake_Config_Single(t *testing.T) {
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
		nameVersion     = "prebuilt-x264@stable"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
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

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.RemoveAll(buildDir))
	check(os.MkdirAll(buildDir, os.ModePerm))
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

func TestInstall_Generate_CMake_Config_Interface(t *testing.T) {
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
		nameVersion     = "prebuilt-ffmpeg-interface@5.1.6"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())
	check(buildtools.CheckTools(celer, "git", "cmake"))

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(err)

	// Build test project.
	buildDir := filepath.Join(dirs.TmpFilesDir, "build_cmake_test")
	check(os.RemoveAll(buildDir))
	check(os.MkdirAll(buildDir, os.ModePerm))

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/interface"),
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

func TestInstall_Generate_CMake_Config_Components(t *testing.T) {
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
		nameVersion     = "prebuilt-ffmpeg-components@5.1.6"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())
	check(buildtools.CheckTools(celer, "git", "cmake"))

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(err)

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.RemoveAll(buildDir))
	check(os.MkdirAll(buildDir, os.ModePerm))

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/components"),
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

func TestInstall_Generate_CMake_Interface_Head_Only(t *testing.T) {
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
		nameVersion     = "prebuilt-eigen-interface@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())
	check(buildtools.CheckTools(celer, "git", "cmake"))

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, nameVersion))
	_, err := port.Install(options)
	check(err)

	// Clear build dir.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.RemoveAll(buildDir))
	check(os.MkdirAll(buildDir, os.ModePerm))

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_config/interface_head_only"),
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
