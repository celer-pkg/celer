package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

func TestCreate(t *testing.T) {
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
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))

	// ============= Create platform ============= //
	t.Run("Create platform success", func(t *testing.T) {
		const platformName = "x86_64-linux-ubuntu-test"
		check(celer.CreatePlatform(platformName))

		// Check if platform really created.
		platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
		if !fileio.PathExists(platformPath) {
			t.Fatalf("platform %s should be created", platformName)
		}

		check(os.RemoveAll(platformPath))
	})

	t.Run("Create platform failed: empyt name", func(t *testing.T) {
		if err := celer.CreatePlatform(""); err == nil {
			t.Fatal("it should be failed")
		}

		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
	})
	check(celer.SetBuildType("Release"))

	// ============= Create project ============= //
	t.Run("Create project success", func(t *testing.T) {
		const projectName = "project_test_create"
		check(celer.CreateProject(projectName))

		projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
		if !fileio.PathExists(projectPath) {
			t.Fatalf("project does not exist: %s", projectName)
		}

		t.Cleanup(func() {
			check(os.Remove(projectPath))
		})
	})

	t.Run("Create project failed: empyt name", func(t *testing.T) {
		if err := celer.CreateProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})

	// ============= Create port ============= //
	t.Run("Create port success", func(t *testing.T) {
		const portName = "test_port_test"
		const portVersion = "1.0.0"
		check(celer.CreatePort(portName + "@" + portVersion))

		portPath := filepath.Join(dirs.PortsDir, fmt.Sprintf("%s/%s/port.toml", portName, portVersion))
		if !fileio.PathExists(portPath) {
			t.Fatalf("port does not exists: %s@%s", portName, portVersion)
		}

		t.Cleanup(func() {
			check(os.Remove(portPath))
		})
	})

	t.Run("Create port failed: empyt name", func(t *testing.T) {
		if err := celer.CreatePort(""); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("Create port failed: invalid port name", func(t *testing.T) {
		if err := celer.CreatePort("libxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})
}
