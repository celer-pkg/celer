package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"runtime"
	"slices"
	"testing"
)

func TestAutoRemove_With_Purge(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	var equals = func(list1, list2 []string) bool {
		if len(list1) != len(list2) {
			return false
		}
		for _, item := range list1 {
			if !slices.Contains(list2, item) {
				return false
			}
		}
		return true
	}

	// Init celer.
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_autoremove"
		portNameVersion = "sqlite3@3.49.0"
	)
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	autoremoveCmd := autoremoveCmd{celer: celer}

	for _, nameVersion := range celer.Project().GetPorts() {
		check(autoremoveCmd.collectPackages(nameVersion))
		check(autoremoveCmd.collectDevPackages(nameVersion))
	}

	check(celer.Deploy(true))

	// ================= test autoremove ================= //
	var (
		buildType  = celer.BuildType()
		packageDir = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, portNameVersion, platform, project, buildType)
		buildDir   = fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, portNameVersion, platform, project, buildType)
	)

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, portNameVersion))
	check(port.InstallFromSource(options))

	autoremoveCmd.purge = true
	autoremoveCmd.buildCache = false
	check(autoremoveCmd.autoremove())

	// Check packages.
	expectedPackages := []string{
		"gflags@2.2.2",
		"x264@stable",
	}
	if !equals(expectedPackages, autoremoveCmd.packages) {
		t.Fatalf("expected %v, got %v", expectedPackages, autoremoveCmd.packages)
	}

	// Check dev packages.
	var expectedDevPackages []string
	if runtime.GOOS == "windows" {
		expectedDevPackages = []string{"nasm@2.16.03"}
	} else {
		expectedDevPackages = []string{
			"nasm@2.16.03",
			"automake@1.18",
			"autoconf@2.72",
			"m4@1.4.19",
			"libtool@2.5.4",
		}
	}
	if !equals(expectedDevPackages, autoremoveCmd.devPackages) {
		t.Fatalf("expected %v, got %v", expectedDevPackages, autoremoveCmd.devPackages)
	}

	if fileio.PathExists(packageDir) {
		t.Fatal("sqlite3 package should be removed.")
	}

	if !fileio.PathExists(buildDir) {
		t.Fatal("sqlite3 build cache should be exists.")
	}
}

func TestAutoRemove_With_BuildCache(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	var equals = func(list1, list2 []string) bool {
		if len(list1) != len(list2) {
			return false
		}
		for _, item := range list1 {
			if !slices.Contains(list2, item) {
				return false
			}
		}
		return true
	}

	// Init celer.
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_autoremove"
		portNameVersion = "sqlite3@3.49.0"
	)
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	autoremoveCmd := autoremoveCmd{celer: celer}

	for _, nameVersion := range celer.Project().GetPorts() {
		check(autoremoveCmd.collectPackages(nameVersion))
		check(autoremoveCmd.collectDevPackages(nameVersion))
	}

	check(celer.Deploy(true))

	validatePackages := func(packages, devPackages []string) error {
		// Check packages.
		expectedPackages := []string{
			"gflags@2.2.2",
			"x264@stable",
		}
		if !equals(expectedPackages, packages) {
			return fmt.Errorf("expected %v, got %v", expectedPackages, packages)
		}

		// Check dev packages.
		var expectedDevPackages []string
		if runtime.GOOS == "windows" {
			expectedDevPackages = []string{"nasm@2.16.03"}
		} else {
			expectedDevPackages = []string{
				"nasm@2.16.03",
				"automake@1.18",
				"autoconf@2.72",
				"m4@1.4.19",
				"libtool@2.5.4",
			}
		}
		if !equals(expectedDevPackages, devPackages) {
			return fmt.Errorf("expected %v, got %v", expectedDevPackages, devPackages)
		}

		return nil
	}

	var (
		buildType  = celer.BuildType()
		packageDir = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, portNameVersion, platform, project, buildType)
		buildDir   = fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, portNameVersion, platform, project, buildType)
	)

	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, portNameVersion))
	check(port.InstallFromSource(options))

	autoremoveCmd.purge = false
	autoremoveCmd.buildCache = true
	check(autoremoveCmd.autoremove())
	check(validatePackages(autoremoveCmd.packages, autoremoveCmd.devPackages))

	t.Cleanup(func() {
		remoteOptions := configs.RemoveOptions{
			Purge:      true,
			Recursive:  true,
			BuildCache: true,
		}
		check(port.Remove(remoteOptions))
	})

	if !fileio.PathExists(packageDir) {
		t.Fatal("sqlite3 package should not be removed.")
	}

	if fileio.PathExists(buildDir) {
		t.Fatal("sqlite3 build cache should be removed.")
	}
}
