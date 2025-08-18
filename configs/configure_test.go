package configs

import (
	"testing"
)

func TestConfigure_Platform(t *testing.T) {
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

	t.Run("configure platform success", func(t *testing.T) {
		const newName = "x86_64-linux-ubuntu-22.04"
		check(celer.ChangePlatform(newName))
		if celer.platform.Name != newName {
			t.Fatalf("platform should be `%s`", newName)
		}
	})

	t.Run("configure platform error: none exist platform", func(t *testing.T) {
		if err := celer.ChangePlatform("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure platform error: empty platform", func(t *testing.T) {
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

	t.Run("configure project success", func(t *testing.T) {
		const projectName = "test_project_01"
		if err := celer.ChangeProject(projectName); err != nil {
			t.Fatal(err)
		}
		if celer.project.Name != projectName {
			t.Fatalf("project should be `%s`", projectName)
		}
	})

	t.Run("configure project error: none exist project", func(t *testing.T) {
		if err := celer.ChangeProject("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure project error: empty project", func(t *testing.T) {
		if err := celer.ChangeProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})
}
