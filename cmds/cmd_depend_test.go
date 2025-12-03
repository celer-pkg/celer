package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"slices"
	"testing"
)

func TestDepend_Without_Dev(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Cleanup function.
	cleanup := func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
		check(os.RemoveAll(dirs.ConfDir))
	}
	t.Cleanup(cleanup)

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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	cmdDepend := dependCmd{celer: celer}
	depedencies, err := cmdDepend.query("eigen@3.4.0")
	check(err)

	expected := []string{
		"ceres-solver@2.1.0",
		"gstreamer@1.26.0",
		"gtsam@4.2.0",
		"lbfgspp@0.3.0",
	}

	if !equals(depedencies, expected) {
		t.Fatalf("expected %s, but got %s", expected, depedencies)
	}
}

func TestDepend_With_Dev(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Cleanup function.
	cleanup := func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
		check(os.RemoveAll(dirs.ConfDir))
	}
	t.Cleanup(cleanup)

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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Search as default mode.
	cmdDepend := dependCmd{celer: celer}
	depedencies, err := cmdDepend.query("nasm@2.16.03")
	check(err)
	if len(depedencies) > 0 {
		t.Fatalf("expected no dependencies, but got %s", depedencies)
	}

	// Search as dev mode.
	cmdDepend.dev = true
	depedencies, err = cmdDepend.query("nasm@2.16.03")
	check(err)
	expected := []string{
		"ffmpeg@3.4.13",
		"ffmpeg@5.1.6",
		"openssl@3.5.0",
		"x264@stable",
	}
	if !equals(depedencies, expected) {
		t.Fatalf("expected %s, but got %s", expected, depedencies)
	}
}
