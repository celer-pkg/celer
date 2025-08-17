package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"testing"
)

func TestCreate_Project(t *testing.T) {
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
	check(celer.SyncConf("https://github.com/celer-pkg/test-conf.git", ""))

	const projectName = "test_project_03"
	check(celer.CreateProject(projectName))

	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	if !fileio.PathExists(projectPath) {
		t.Fatalf("project %s does not exists", projectName)
	}

	t.Cleanup(func() {
		check(os.Remove(projectPath))
	})

	t.Run("create project error: empty name", func(t *testing.T) {
		if err := celer.CreateProject(""); err == nil {
			t.Fatal("it should be failed")
		} else {
			if err.Error() != "project name is empty" {
				t.Fatal("error should be 'project name is empty'")
			}
		}
	})
}

func TestCreate_Project_EmptyName(t *testing.T) {
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
	check(celer.SyncConf("https://github.com/celer-pkg/test-conf.git", ""))

	if err := celer.CreateProject(""); err == nil {
		t.Fatal("it should be failed")
	}
}
