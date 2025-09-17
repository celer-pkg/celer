package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"testing"
)

func TestCreate_Project_Success(t *testing.T) {
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

	const projectName = "test_project_03"
	check(celer.CreateProject(projectName))

	projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	if !fileio.PathExists(projectPath) {
		t.Fatalf("project %s does not exists", projectName)
	}

	t.Cleanup(func() {
		check(os.Remove(projectPath))
	})

	// Check default opt level.
	var project Project
	check(project.Init(celer, projectName))
	if project.OptFlags.Debug != "-g" ||
		project.OptFlags.Release != "-O3" ||
		project.OptFlags.RelWithDebInfo != "-O2 -g" ||
		project.OptFlags.MinSizeRel != "-Os" {
		t.Fatalf("default opt level is not right, expect '-g -O3 -O2 -g -Os', got '%s %s %s %s'",
			project.OptFlags.Debug,
			project.OptFlags.Release,
			project.OptFlags.RelWithDebInfo,
			project.OptFlags.MinSizeRel,
		)
	}
}

func TestCreate_Project_EmptyName(t *testing.T) {
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

	if err := celer.CreateProject(""); err == nil {
		t.Fatal("it should be failed")
	}
}
