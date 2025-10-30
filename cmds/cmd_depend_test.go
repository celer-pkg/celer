package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"slices"
	"testing"
)

func TestDepend(t *testing.T) {
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
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	// ============= Depend platform ============= //
	t.Run("Search dependencies", func(t *testing.T) {
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
	})

	t.Run("Search dependencies for dev", func(t *testing.T) {
		// Search as default mode.
		cmdDepend := dependCmd{celer: celer}
		depedencies, err := cmdDepend.query("autoconf@2.72")
		check(err)
		if len(depedencies) > 0 {
			t.Fatalf("expected no dependencies, but got %s", depedencies)
		}

		// Search as dev mode.
		cmdDepend.dev = true
		depedencies, err = cmdDepend.query("autoconf@2.72")
		check(err)
		expected := []string{
			"automake@1.18",
			"flex@2.6.4",
		}
		if !equals(depedencies, expected) {
			t.Fatalf("expected %s, but got %s", expected, depedencies)
		}
	})
}
