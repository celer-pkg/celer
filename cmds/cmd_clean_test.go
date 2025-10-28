package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

func TestClean(t *testing.T) {
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
	const (
		platform = "x86_64-linux-ubuntu-22.04"
		project  = "project_test_clean"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	check(celer.Deploy())

	cleanCmd := cleanCmd{celer: celer}

	var (
		buildDir = func(nameVersion string, dev bool) string {
			if dev {
				hostPlatform := celer.Platform().GetHostName() + "-dev"
				return fmt.Sprintf("%s/%s/%s", dirs.BuildtreesDir, nameVersion, hostPlatform)
			} else {
				return fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, nameVersion, platform, project, celer.BuildType())
			}
		}
	)

	t.Run("clean port", func(t *testing.T) {
		cleanCmd.dev = false
		cleanCmd.recurse = false
		cleanCmd.all = false
		check(cleanCmd.clean("x264@stable"))
		if fileio.PathExists(buildDir("x264@stable", false)) {
			t.Fatal("x264@stable build dir should be removed")
		}
	})

	t.Run("clean port for dev", func(t *testing.T) {
		cleanCmd.dev = true
		cleanCmd.recurse = false
		cleanCmd.all = false
		check(cleanCmd.clean("m4@1.4.19"))
		if fileio.PathExists(buildDir("m4@1.4.19", true)) {
			t.Fatal("m4@1.4.19 build dir should be removed")
		}
	})

	t.Run("clean recursive", func(t *testing.T) {
		cleanCmd.dev = true
		cleanCmd.recurse = true
		cleanCmd.all = false
		check(cleanCmd.clean("automake@1.18"))

		checkList := map[string]bool{
			"nasm@2.16.03":  true,
			"automake@1.18": true,
			"autoconf@2.72": true,
			"m4@1.4.19":     true,
		}
		for nameVersion, dev := range checkList {
			if fileio.PathExists(buildDir(nameVersion, dev)) {
				t.Fatal(nameVersion + " build dir should be removed")
			}
		}
	})

	t.Run("clean all", func(t *testing.T) {
		cleanCmd.dev = true
		cleanCmd.recurse = true
		cleanCmd.all = true

		check(os.RemoveAll(dirs.InstalledDir))
		check(os.RemoveAll(dirs.PackagesDir))

		check(celer.Deploy())
		check(cleanCmd.cleanAll())

		checkList := map[string]bool{
			"x264@stable":   false,
			"automake@1.18": true,
			"autoconf@2.72": true,
			"m4@1.4.19":     true,
		}
		for nameVersion, dev := range checkList {
			if fileio.PathExists(buildDir(nameVersion, dev)) {
				t.Fatal(nameVersion + " build dir should be removed")
			}
		}

		// nasm is build in source, it should not be cleaned.
		modified, err := git.IsModified(filepath.Join(dirs.BuildtreesDir, "nasm@2.16.03", "src"))
		check(err)
		if modified {
			t.Fatal("nasm@2.16.03 src dir should be cleaned")
		}
	})
}
