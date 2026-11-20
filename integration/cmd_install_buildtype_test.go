package integration

import (
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func TestInstall_BuildType(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		platform = buildTestPlatform()
		project  = testProjectName
	)

	// Init celer.
	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))

	t.Run("install with build type Release", func(t *testing.T) {
		check(t, celer.SetBuildType("Release"))

		var (
			nameVersion = "eigen@3.4.0"
			packageDir  = filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)
		)

		var port configs.Port
		var options = configs.InstallOptions{Prefer: configs.PreferSource}
		check(t, port.Init(celer, nameVersion))
		if _, err := port.Install(options); err != nil {
			t.Fatal(err)
		}

		// Check if package dir exists.
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}

		// Check if installed.
		installed, err := port.Installed()
		check(t, err)

		if !installed {
			t.Fatal("package is not installed")
		}

		// Clean up.
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recursive:  true,
			BuildCache: true,
		}
		check(t, port.Remove(removeOptions))
	})

	t.Run("install with build type Debug", func(t *testing.T) {
		check(t, celer.SetBuildType("Debug"))

		var (
			nameVersion = "eigen@3.4.0"
			packageDir  = filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)
		)

		var port configs.Port
		var options = configs.InstallOptions{Prefer: configs.PreferSource}
		check(t, port.Init(celer, nameVersion))
		if _, err := port.Install(options); err != nil {
			t.Fatal(err)
		}

		// Check if package dir exists.
		if !fileio.PathExists(packageDir) {
			t.Fatal("package dir cannot found")
		}

		// Check if installed.
		installed, err := port.Installed()
		check(t, err)

		if !installed {
			t.Fatal("package is not installed")
		}

		// Clean up.
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recursive:  true,
			BuildCache: true,
		}
		check(t, port.Remove(removeOptions))
	})
}
