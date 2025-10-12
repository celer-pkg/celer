package configs

import (
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"testing"
)

func TestConfigure_Project(t *testing.T) {
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
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))

	t.Run("configure project success", func(t *testing.T) {
		const projectName = "test_project_01"
		check(celer.SetProject(projectName))
		if celer.project.Name != projectName {
			t.Fatalf("project should be `%s`", projectName)
		}
	})

	t.Run("configure project error: none exist project", func(t *testing.T) {
		if err := celer.SetProject("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure project error: empty project", func(t *testing.T) {
		if err := celer.SetProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})
}
