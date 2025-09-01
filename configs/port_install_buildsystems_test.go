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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "x264@stable", celer.BuildType()))

	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "glog@0.6.0", celer.BuildType()))
	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_B2(t *testing.T) {
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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "boost@1.87.0", celer.BuildType()))
	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_GYP(t *testing.T) {
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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "nss@3.55", celer.BuildType()))
	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Meson(t *testing.T) {
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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_01"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "pixman@0.44.2", celer.BuildType()))
	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_PreBuilt(t *testing.T) {
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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_02"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "prebuilt-x264@stable", celer.BuildType()))
	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Nobuild(t *testing.T) {
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
		check(celer.SetPlatform("x86_64-windows-msvc-14.44"))
	} else {
		check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	}

	// Change project.
	check(celer.SetProject("test_project_02"))

	// This will setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "gnulib@master", celer.BuildType()))
	check(port.installFromSource())

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}
