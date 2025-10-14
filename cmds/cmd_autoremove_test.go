package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"
)

func TestAutoRemove(t *testing.T) {
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

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	const (
		platform        = "x86_64-linux-ubuntu-22.04"
		project         = "test_project_autoremove"
		portNameVersion = "sqlite3@3.49.0"
	)
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	autoremoveCmd := autoremoveCmd{celer: celer}

	for _, nameVersion := range celer.Project().GetPorts() {
		check(autoremoveCmd.collectPackages(nameVersion))
		check(autoremoveCmd.collectDevPackages(nameVersion))
	}

	check(celer.Deploy())

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
		expectedDevPackages := []string{
			"nasm@2.16.03",
			"automake@1.18",
			"autoconf@2.72",
			"m4@1.4.19",
			"libtool@2.5.4",
		}
		if !equals(expectedDevPackages, devPackages) {
			return fmt.Errorf("expected %v, got %v", expectedDevPackages, devPackages)
		}

		return nil
	}

	var (
		buildType  = strings.ToLower(celer.BuildType())
		packageDir = fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, portNameVersion, platform, project, buildType)
		buildDir   = fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, portNameVersion, platform, project, buildType)
	)

	t.Run("autoremove with purge", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, portNameVersion, celer.BuildType()))
		check(port.InstallFromSource(options))

		t.Cleanup(func() {
			check(port.Remove(true, true, true))
		})

		autoremoveCmd.purge = false
		autoremoveCmd.buildCache = true
		check(autoremoveCmd.autoremove())
		check(validatePackages(autoremoveCmd.packages, autoremoveCmd.devPackages))

		if fileio.PathExists(packageDir) {
			t.Fatal("sqlite3 package should be removed.")
		}

		if !fileio.PathExists(buildDir) {
			t.Fatal("sqlite3 build cache should be exists.")
		}
	})

	t.Run("autoremove with build-cache", func(t *testing.T) {
		var port configs.Port
		var options configs.InstallOptions
		check(port.Init(celer, portNameVersion, celer.BuildType()))
		check(port.InstallFromSource(options))

		autoremoveCmd.purge = true
		autoremoveCmd.buildCache = false
		check(autoremoveCmd.autoremove())
		check(validatePackages(autoremoveCmd.packages, autoremoveCmd.devPackages))

		t.Cleanup(func() {
			check(port.Remove(true, true, true))
		})

		if !fileio.PathExists(packageDir) {
			t.Fatal("sqlite3 package should not be removed.")
		}

		if fileio.PathExists(buildDir) {
			t.Fatal("sqlite3 build cache should be removed.")
		}
	})
}
