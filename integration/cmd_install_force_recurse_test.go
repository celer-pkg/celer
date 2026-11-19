package integration

import (
	"os"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
)

func TestInstall_With_Force(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var modTime = func(path string) time.Time {
		info, err := os.Stat(path)
		check(t, err)
		return info.ModTime()
	}

	var (
		nameVersion = "glog@0.6.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	check(t, celer.SetPlatform(platform))

	var (
		options       configs.InstallOptions
		glogLibName   = expr.If(runtime.GOOS == "windows", "glog.lib", "libglog.so.0.6.0")
		gflagsLibName = expr.If(runtime.GOOS == "windows", "gflags.lib", "libgflags.so.2.2.2")
	)

	// glog@0.6.0
	var glogPort configs.Port
	check(t, glogPort.Init(celer, nameVersion))
	_, err := glogPort.Install(options)
	check(t, err)
	lastGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))

	// gflags@2.2.2 (dependency of glog@0.6.0)
	var gflagsPort configs.Port
	check(t, gflagsPort.Init(celer, "gflags@2.2.2"))

	lastGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	// Re-install with --force.
	options.Force = true
	check(t, glogPort.Init(celer, nameVersion))
	_, err = glogPort.Install(options)
	check(t, err)
	newGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))
	newGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	if !newGlogModTime.After(lastGlogModTime) {
		t.Fatalf("package was not re-installed with --force (mtime %v, want > %v)", newGlogModTime, lastGlogModTime)
	}

	if !newGflagsModTime.Equal(lastGflagsModTime) {
		t.Fatal("dependency package shoud not be re-installed")
	}
}

func TestInstall_With_Force_Recursive(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var modTime = func(path string) time.Time {
		info, err := os.Stat(path)
		check(t, err)
		return info.ModTime()
	}

	var (
		nameVersion = "glog@0.6.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	check(t, celer.SetPlatform(platform))
	check(t, buildtools.CheckTools(celer, "git", "cmake"))

	var (
		options       configs.InstallOptions
		glogLibName   = expr.If(runtime.GOOS == "windows", "glog.lib", "libglog.so.0.6.0")
		gflagsLibName = expr.If(runtime.GOOS == "windows", "gflags.lib", "libgflags.so.2.2.2")
	)

	// glog@0.6.0
	var glogPort configs.Port
	check(t, glogPort.Init(celer, nameVersion))
	_, err := glogPort.Install(options)
	check(t, err)
	lastGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))

	// gflags@2.2.2 (dependency of glog@0.6.0)
	var gflagsPort configs.Port
	check(t, gflagsPort.Init(celer, "gflags@2.2.2"))

	lastGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	// Re-install with --force.
	options.Force = true
	options.Recursive = true
	check(t, glogPort.Init(celer, nameVersion))
	_, err = glogPort.Install(options)
	check(t, err)
	newGlogModTime := modTime(filepath.Join(glogPort.InstalledDir, "lib", glogLibName))
	newGflagsModTime := modTime(filepath.Join(gflagsPort.InstalledDir, "lib", gflagsLibName))

	if !newGlogModTime.After(lastGlogModTime) {
		t.Fatalf("package should be re-installed with --force (mtime %v, want > %v)", newGlogModTime, lastGlogModTime)
	}

	if !newGflagsModTime.After(lastGflagsModTime) {
		t.Fatalf("dependency package shoud be re-installed (mtime %v, want > %v)", newGflagsModTime, lastGflagsModTime)
	}
}
