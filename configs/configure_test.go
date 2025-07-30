package configs

import (
	"celer/pkgs/dirs"
	"os"
	"testing"
)

func TestConfigure_Platform(t *testing.T) {
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

	t.Run("configure platform success", func(t *testing.T) {
		var oldName = celer.Platform().Name

		// Rollback
		defer func() {
			if err := celer.ChangePlatform(oldName); err != nil {
				t.Fatal(err)
			}
			if celer.platform.Name != oldName {
				t.Fatalf("platform should be rollbacked to `%s`", oldName)
			}
		}()

		const newName = "x86_64-linux-ubuntu-22.04"
		if err := celer.ChangePlatform(newName); err != nil {
			t.Fatal(err)
		}
		if celer.platform.Name != newName {
			t.Fatalf("platform should be `%s`", newName)
		}
	})

	t.Run("configure platform failed: none exist platform", func(t *testing.T) {
		if err := celer.ChangePlatform("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure platform failed: empty platform", func(t *testing.T) {
		if err := celer.ChangePlatform(""); err != nil {
			if err.Error() != "platform name is empty" {
				t.Fatal("error should be 'platform name is empty'")
			}
		} else {
			t.Fatal("it should be failed")
		}
	})
}

func TestConfigure_Project(t *testing.T) {
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

	t.Run("configure project success", func(t *testing.T) {
		const projectName = "test_project_01"
		if err := celer.ChangeProject(projectName); err != nil {
			t.Fatal(err)
		}
		if celer.project.Name != projectName {
			t.Fatalf("project should be `%s`", projectName)
		}
	})

	t.Run("configure project failed: none exist project", func(t *testing.T) {
		if err := celer.ChangeProject("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure project failed: empty project", func(t *testing.T) {
		if err := celer.ChangeProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})
}
