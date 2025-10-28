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
		nameVersion = "prebuilt-x264-single-target@stable"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04-gcc-11.5")
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

	// Clear up.
	check(port.Remove(configs.RemoveOptions{
		Purge:      true,
		BuildCache: true,
		Recurse:    true,
	}))
}

func TestInstall_Generate_CMake_Prebuilt_Interface_Libraries(t *testing.T) {
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
		nameVersion = "prebuilt-ffmpeg-interface@5.1.6"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04-gcc-11.5")
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

	// Build test project.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() {
		check(os.RemoveAll(buildDir))
	})

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_prebuilt/interface_libraries"),
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
		Recurse:    true,
	}))
}

func TestInstall_Generate_CMake_Prebuilt_Muti_Components(t *testing.T) {
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
		nameVersion = "prebuilt-ffmpeg-multi-components@5.1.6"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04-gcc-11.5")
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

	// Build test project.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() {
		check(os.RemoveAll(buildDir))
	})

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_prebuilt/muti_components"),
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
		Recurse:    true,
	}))
}

func TestInstall_Generate_CMake_Source_Single_Target(t *testing.T) {
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
		nameVersion = "x264@stable"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04-gcc-11.5")
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
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_source/single_target"),
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
		Recurse:    true,
	}))
}

func TestInstall_Generate_CMake_Source_Multi_Components(t *testing.T) {
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
		nameVersion = "ffmpeg@5.1.6"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04-gcc-11.5")
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
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_source/muti_components"),
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
		Recurse:    true,
	}))
}

func TestInstall_Generate_CMake_Prebuilt_Interface_Head_Only(t *testing.T) {
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
		nameVersion = "prebuilt-eigen-interface@3.4.0"
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

	// Build test project.
	buildDir := filepath.Join(os.TempDir(), "build_cmake_test")
	check(os.MkdirAll(buildDir, os.ModePerm))
	t.Cleanup(func() {
		check(os.RemoveAll(buildDir))
	})

	// Build test project.
	executer := cmd.NewExecutor("configure test project", "cmake",
		"-D", fmt.Sprintf("CMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir),
		"-S", filepath.Join(dirs.WorkspaceDir, "testdata/gen_cmake_configs_prebuilt/interface_head_only"),
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
		Recurse:    true,
	}))
}
