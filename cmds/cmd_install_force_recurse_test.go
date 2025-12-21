package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"os"
	"path/filepath"
	"runtime"
	"testing"
	"time"
)

func TestInstall_With_Force(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	var modTime = func(path string) time.Time {
		info, err := os.Stat(path)
		check(err)
		return info.ModTime()
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "glog@0.6.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var (
		options       configs.InstallOptions
		glogLibName   = expr.If(runtime.GOOS == "windows", "glog.lib", "libglog.so.0.6.0")
		gflagsLibName = expr.If(runtime.GOOS == "windows", "gflags.lib", "libgflags.so.2.2.2")
	)

	// glog@0.6.0
	var glogPort configs.Port
	check(glogPort.Init(celer, nameVersion))
	_, err := glogPort.Install(options)
	check(err)
	lastGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))

	// gflags@2.2.2 (dependency of glog@0.6.0)
	var gflagsPort configs.Port
	check(gflagsPort.Init(celer, "gflags@2.2.2"))

	lastGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	// Re-install with --force.
	options.Force = true
	check(glogPort.Init(celer, nameVersion))
	_, err = glogPort.Install(options)
	check(err)
	newGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))
	newGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	if newGlogModTime.Equal(lastGlogModTime) {
		t.Fatal("package was not re-installed with --force")
	}

	if !newGflagsModTime.Equal(lastGflagsModTime) {
		t.Fatal("dependency package shoud not be re-installed")
	}
}

func TestInstall_With_Force_Recursive(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	var modTime = func(path string) time.Time {
		info, err := os.Stat(path)
		check(err)
		return info.ModTime()
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "glog@0.6.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPlatform(platform))
	check(celer.Setup())
	check(buildtools.CheckTools(celer, "git", "cmake"))

	var (
		options       configs.InstallOptions
		glogLibName   = expr.If(runtime.GOOS == "windows", "glog.lib", "libglog.so.0.6.0")
		gflagsLibName = expr.If(runtime.GOOS == "windows", "gflags.lib", "libgflags.so.2.2.2")
	)

	// glog@0.6.0
	var glogPort configs.Port
	check(glogPort.Init(celer, nameVersion))
	_, err := glogPort.Install(options)
	check(err)
	lastGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))

	// gflags@2.2.2 (dependency of glog@0.6.0)
	var gflagsPort configs.Port
	check(gflagsPort.Init(celer, "gflags@2.2.2"))

	lastGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	// Re-install with --force.
	options.Force = true
	options.Recursive = true
	check(glogPort.Init(celer, nameVersion))
	_, err = glogPort.Install(options)
	check(err)
	newGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))
	newGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	if newGlogModTime.Equal(lastGlogModTime) {
		t.Fatal("package should be re-installed with --force")
	}

	if newGflagsModTime.Equal(lastGflagsModTime) {
		t.Fatal("dependency package shoud be re-installed")
	}
}
