package configs

import (
	"runtime"
	"testing"
)

func TestInstall_MakeFiles(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))

	// Change platform
	if runtime.GOOS == "windows" {
		check(celer.ChangePlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.ChangePlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.ChangeProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "x264@stable", celer.BuildType()))
	check(port.installFromSource())
}

func TestInstall_CMake(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))

	// Change platform
	if runtime.GOOS == "windows" {
		check(celer.ChangePlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.ChangePlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.ChangeProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "glog@0.6.0", celer.BuildType()))
	check(port.installFromSource())
}

func TestInstall_B2(t *testing.T) {

}

func TestInstall_GYP(t *testing.T) {

}

func TestInstall_Meson(t *testing.T) {

}

func TestInstall_PreBuilt(t *testing.T) {

}

func TestInstall_NoBuild(t *testing.T) {

}
