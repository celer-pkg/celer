package integration

import (
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/fileio"
)

func TestInstall_FromPackage(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "eigen@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))

	packageDir := filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)

	t.Run("install success", func(t *testing.T) {

		var port configs.Port
		var options = configs.InstallOptions{Prefer: configs.PreferSource}
		check(t, port.Init(celer, nameVersion))

		if _, err := port.Install(options); err != nil {
			t.Fatal(err)
		}

		if !fileio.PathExists(packageDir) {
			t.Fatal("package cannot found")
		}

		removeOptions := configs.RemoveOptions{
			Purge:      false,
			Recursive:  true,
			BuildCache: true,
		}
		check(t, port.Remove(removeOptions))

		options.Prefer = configs.PreferPackage
		if _, err := port.Install(options); err != nil {
			t.Fatal(err)
		}

		t.Cleanup(func() {
			removeOptions := configs.RemoveOptions{
				Purge:      true,
				Recursive:  true,
				BuildCache: true,
			}
			check(t, port.Remove(removeOptions))
		})

	})

	t.Run("install failed", func(t *testing.T) {

		var port configs.Port
		var options = configs.InstallOptions{Prefer: configs.PreferSource}
		check(t, port.Init(celer, nameVersion))
		if _, err := port.Install(options); err != nil {
			t.Fatal(err)
		}

		if !fileio.PathExists(packageDir) {
			t.Fatal("package cannot found")
		}
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recursive:  true,
			BuildCache: true,
		}
		check(t, port.Remove(removeOptions))

		options.Prefer = configs.PreferPackage
		if _, err := port.Install(options); err != nil {
			t.Fatal(err)
		}
	})
}
