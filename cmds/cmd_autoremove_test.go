package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"testing"
)

func TestCollectProjectPackages(t *testing.T) {
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
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/add_test_cases"))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	check(celer.SetProject("test_project_autoremove"))

	autoremoveCmd := autoremoveCmd{celer: celer}

	for _, nameVersion := range celer.Project().GetPorts() {
		check(autoremoveCmd.collectProjectPackages(nameVersion))
		check(autoremoveCmd.collectProjectDevPackages(nameVersion))
	}

	// Check packages.
	expectedPackages := map[string]bool{
		"glog@0.6.0":   true,
		"gflags@2.2.2": true,
		"x264@stable":  true,
	}
	if len(autoremoveCmd.projectPackages) != len(expectedPackages) {
		t.Fatalf("expected %d packages, got %d", len(expectedPackages), len(autoremoveCmd.projectPackages))
	}
	for _, nameVersion := range autoremoveCmd.projectPackages {
		if !expectedPackages[nameVersion] {
			t.Fatalf("unexpected package: %s", nameVersion)
		}
	}

	// Check dev packages.
	expectedDevPackages := map[string]bool{
		"nasm@2.16.03":  true,
		"automake@1.18": true,
		"autoconf@2.72": true,
		"m4@1.4.19":     true,
		"libtool@2.5.4": true,
	}
	if len(autoremoveCmd.projectDevPackages) != len(expectedDevPackages) {
		t.Fatalf("expected %d dev packages, got %d", len(expectedDevPackages), len(autoremoveCmd.projectDevPackages))
	}
	for _, nameVersion := range autoremoveCmd.projectDevPackages {
		if !expectedDevPackages[nameVersion] {
			t.Fatalf("unexpected dev package: %s", nameVersion)
		}
	}
}
