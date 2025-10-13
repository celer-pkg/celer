package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"slices"
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/add_test_cases"))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	check(celer.SetProject("test_project_autoremove"))

	autoremoveCmd := autoremoveCmd{celer: celer}

	for _, nameVersion := range celer.Project().GetPorts() {
		check(autoremoveCmd.collectProjectPackages(nameVersion))
		check(autoremoveCmd.collectProjectDevPackages(nameVersion))
	}

	check(celer.Deploy())

	// The sqlite3 will be autoremoved later.
	var port configs.Port
	var options configs.InstallOptions
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	check(port.InstallFromSource(options))

	check(autoremoveCmd.autoremove(true, true))

	// Check packages.
	expectedPackages := []string{
		"gflags@2.2.2",
		"x264@stable",
	}
	if !equals(expectedPackages, autoremoveCmd.projectPackages) {
		t.Fatalf("expected %v, got %v", expectedPackages, autoremoveCmd.projectPackages)
	}

	// Check dev packages.
	expectedDevPackages := []string{
		"nasm@2.16.03",
		"automake@1.18",
		"autoconf@2.72",
		"m4@1.4.19",
		"libtool@2.5.4",
	}
	if !equals(expectedDevPackages, autoremoveCmd.projectDevPackages) {
		t.Fatalf("expected %v, got %v", expectedDevPackages, autoremoveCmd.projectDevPackages)
	}
}
