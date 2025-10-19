package cmds

import (
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

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion = "glog@0.6.0"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
		project     = "project_test_install"
	)

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPlatform(platform))
	check(celer.Platform().Setup())

	var options configs.InstallOptions

	// glog@0.6.0
	var glogPort configs.Port
	check(glogPort.Init(celer, nameVersion))
	_, err := glogPort.Install(options)
	check(err)
	lastGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", "libglog.so.0.6.0"))

	// gflags@2.2.2 (dependency of glog@0.6.0)
	var gflagsPort configs.Port
	check(gflagsPort.Init(celer, "gflags@2.2.2"))

	lastGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", "libgflags.so.2.2.2"))

	// Re-install with --force.
	options.Force = true
	check(glogPort.Init(celer, nameVersion))
	_, err = glogPort.Install(options)
	check(err)
	newGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", "libglog.so.0.6.0"))
	newGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", "libgflags.so.2.2.2"))

	if newGlogModTime.Equal(lastGlogModTime) {
		t.Fatal("package was not re-installed with --force")
	}

	if !newGflagsModTime.Equal(lastGflagsModTime) {
		t.Fatal("dependency package shoud not be re-installed")
	}
}

func TestInstall_With_Force_Recurse(t *testing.T) {
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

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion = "glog@0.6.0"
		platform    = expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")
		project     = "project_test_install"
	)

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPlatform(platform))
	check(celer.Platform().Setup())

	var options configs.InstallOptions

	// glog@0.6.0
	var glogPort configs.Port
	check(glogPort.Init(celer, nameVersion))
	_, err := glogPort.Install(options)
	check(err)
	lastGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", "libglog.so.0.6.0"))

	// gflags@2.2.2 (dependency of glog@0.6.0)
	var gflagsPort configs.Port
	check(gflagsPort.Init(celer, "gflags@2.2.2"))

	lastGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", "libgflags.so.2.2.2"))

	// Re-install with --force.
	options.Force = true
	options.Recurse = true
	check(glogPort.Init(celer, nameVersion))
	_, err = glogPort.Install(options)
	check(err)
	newGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", "libglog.so.0.6.0"))
	newGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", "libgflags.so.2.2.2"))

	if newGlogModTime.Equal(lastGlogModTime) {
		t.Fatal("package should be re-installed with --force")
	}

	if newGflagsModTime.Equal(lastGflagsModTime) {
		t.Fatal("dependency package shoud be re-installed")
	}
}
