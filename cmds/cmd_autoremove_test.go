package cmds

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func setupWorkspaceForAutoremove(t *testing.T) (*configs.Celer, string, string, string) {
	t.Helper()

	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_autoremove"
		portNameVersion = "sqlite3@3.49.0"
	)

	celer := newInitializedCeler(t)
	if _, err := runCommand(t, (&configureCmd{}).Command(celer), "--build-type=Release"); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, (&configureCmd{}).Command(celer), "--platform="+platform); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, (&configureCmd{}).Command(celer), "--project="+project); err != nil {
		t.Fatal(err)
	}

	return celer, platform, project, portNameVersion
}

func TestAutoRemove_With_Purge(t *testing.T) {
	celer, platform, project, portNameVersion := setupWorkspaceForAutoremove(t)

	var (
		buildType  = celer.BuildType()
		packageDir = filepath.Join(dirs.PackagesDir, platform, project, buildType, portNameVersion)
		buildDir   = fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, portNameVersion, platform, project, buildType)
	)

	// Install sqlite3 from source so there is something for autoremove to
	// scan (and so we can assert the package dir is removed afterwards).
	var port configs.Port
	if err := port.Init(celer, portNameVersion); err != nil {
		t.Fatal(err)
	}
	if err := port.InstallFromSource(configs.InstallOptions{}); err != nil {
		t.Fatal(err)
	}

	// Run `celer autoremove --purge` exactly as a user would.
	cmd := &autoremoveCmd{}
	if _, err := runCommand(t, cmd.Command(celer), "--purge"); err != nil {
		t.Fatal(err)
	}

	// Check packages.
	expectedPackages := []string{
		"gflags@2.2.2",
		"x264@stable",
	}
	if !equals(expectedPackages, cmd.packages) {
		t.Fatalf("expected %v, got %v", expectedPackages, cmd.packages)
	}

	// Check dev packages.
	var expectedDevPackages []string
	if runtime.GOOS == "windows" {
		expectedDevPackages = []string{"nasm@2.16.03"}
	} else {
		expectedDevPackages = []string{
			"nasm@2.16.03",
			"automake@1.18",
			"autoconf@2.72",
			"m4@1.4.19",
			"help2man@1.49.3",
			"libtool@2.5.4",
		}
	}
	if !equals(expectedDevPackages, cmd.devPackages) {
		t.Fatalf("expected %v, got %v", expectedDevPackages, cmd.devPackages)
	}

	if fileio.PathExists(packageDir) {
		t.Fatal("sqlite3 package should be removed.")
	}

	if !fileio.PathExists(buildDir) {
		t.Fatal("sqlite3 build cache should be exists.")
	}
}

func TestAutoRemove_With_BuildCache(t *testing.T) {
	celer, platform, project, portNameVersion := setupWorkspaceForAutoremove(t)

	var (
		buildType  = celer.BuildType()
		packageDir = filepath.Join(dirs.PackagesDir, platform, project, buildType, portNameVersion)
		buildDir   = fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, portNameVersion, platform, project, buildType)
	)

	var port configs.Port
	if err := port.Init(celer, portNameVersion); err != nil {
		t.Fatal(err)
	}
	if err := port.InstallFromSource(configs.InstallOptions{}); err != nil {
		t.Fatal(err)
	}

	validatePackages := func(packages, devPackages []string) error {
		// Check packages.
		expectedPackages := []string{
			"gflags@2.2.2",
			"x264@stable",
		}
		if !equals(expectedPackages, packages) {
			return fmt.Errorf("expected %v, got %v", expectedPackages, packages)
		}

		// Check dev packages.
		var expectedDevPackages []string
		if runtime.GOOS == "windows" {
			expectedDevPackages = []string{"nasm@2.16.03"}
		} else {
			expectedDevPackages = []string{
				"nasm@2.16.03",
				"automake@1.18",
				"autoconf@2.72",
				"m4@1.4.19",
				"help2man@1.49.3",
				"libtool@2.5.4",
			}
		}
		if !equals(expectedDevPackages, devPackages) {
			return fmt.Errorf("expected %v, got %v", expectedDevPackages, devPackages)
		}

		return nil
	}

	// First run: `celer autoremove --build-cache` — buildtrees gone, package kept.
	cmd1 := &autoremoveCmd{}
	if _, err := runCommand(t, cmd1.Command(celer), "--build-cache"); err != nil {
		t.Fatal(err)
	}
	if err := validatePackages(cmd1.packages, cmd1.devPackages); err != nil {
		t.Fatal(err)
	}

	t.Cleanup(func() {
		removeOptions := configs.RemoveOptions{
			Purge:      true,
			Recursive:  true,
			BuildCache: true,
		}
		if err := port.Remove(removeOptions); err != nil {
			t.Fatal(err)
		}
	})

	if !fileio.PathExists(packageDir) {
		t.Fatal("sqlite3 package should not be removed.")
	}

	if fileio.PathExists(buildDir) {
		t.Fatal("sqlite3 build cache should be removed.")
	}

	// Second run: `celer autoremove --purge` — even if trace/meta were removed
	// in the previous run, the package dir should still be removable.
	cmd2 := &autoremoveCmd{}
	if _, err := runCommand(t, cmd2.Command(celer), "--purge"); err != nil {
		t.Fatal(err)
	}

	if fileio.PathExists(packageDir) {
		t.Fatal("sqlite3 package should be removed by second autoremove with purge.")
	}
}
