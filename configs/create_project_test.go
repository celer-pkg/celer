package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"testing"
)

func TestCreate_Project(t *testing.T) {
	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(dirs.ParentDir(currentDir, 1))

	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}

	const projectName = "test_project_03"
	if err := celer.CreateProject(projectName); err != nil {
		t.Fatal(err)
	}

	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	if !fileio.PathExists(projectPath) {
		t.Fatalf("project %s does not exists", projectName)
	}

	t.Cleanup(func() {
		if err := os.Remove(projectPath); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("create project failed: empty name", func(t *testing.T) {
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
	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(dirs.ParentDir(currentDir, 1))

	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}

	if err := celer.CreateProject(""); err == nil {
		t.Fatal("it should be failed")
	}
}
