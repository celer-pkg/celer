// The workspace fixture of the integration tests: a materialized conf checkout,
// the test project, and the helpers that run a command and inspect what it
// prints.
package integration

import (
	"embed"
	"io"
	"io/fs"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
	"testing"

	"github.com/BurntSushi/toml"
	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/cmds"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/spf13/cobra"
)

const (
	testProjectName   = "test_project"
	testPortsBaseline = "master"
)

// testProject is the project the tests build against.
var testProject = configs.Project{
	Name:          testProjectName,
	PortsBaseline: testPortsBaseline,
}

//go:embed testdata/conf
var confFS embed.FS

// newTestCeler prepares a clean workspace for one test and returns an initialized
// celer for it.
func newTestCeler(t *testing.T, platform string, project configs.Project) *configs.Celer {
	t.Helper()
	dirs.RemoveAllForTest()

	// Keep what explains a failure (build logs, celer.toml, environment script)
	// when the run uses `go test -artifacts`.
	t.Cleanup(func() { preserveWorkspaceArtifacts(t) })

	// Place the generated conf where a clone would land: celer resolves the conf dir from
	// the workspace.
	confDir := dirs.ConfDir
	if err := os.RemoveAll(confDir); err != nil {
		t.Fatalf("failed to clean the conf dir -> %v", err)
	}
	t.Cleanup(func() {
		// The generated conf must not leak into the next test or into a developer workspace.
		_ = os.RemoveAll(confDir)
	})

	if project.PortsBaseline == "" {
		project.PortsBaseline = testPortsBaseline
	}
	materializeTestConf(t)
	writeTestProject(t, project)

	celer := configs.NewCeler()
	if err := celer.InitWithPlatform(platform, configs.InitOption{}); err != nil {
		t.Fatal(err)
	}

	if platform != "" {
		if got := testCelerTomlPlatform(t); got != platform {
			t.Fatalf("celer.toml should record the fixture platform `%s`, got `%s`", platform, got)
		}
	}

	if err := buildtools.CheckTools(celer, "git"); err != nil {
		t.Fatalf("git is required by the cmd tests -> %v", err)
	}

	initTestConfRepo(t)

	return celer
}

// initTestConfRepo makes the generated conf a git repository with a local origin, so the
// tests that update the conf repo have a real repo to fetch from.
func initTestConfRepo(t *testing.T) {
	t.Helper()

	runGit := func(args ...string) {
		t.Helper()

		command := exec.Command("git", append([]string{"-C", dirs.ConfDir, "-c", "user.name=celer", "-c", "user.email=celer@test"}, args...)...)
		if output, err := command.CombinedOutput(); err != nil {
			t.Fatalf("git %v -> %v\n%s", args, err, output)
		}
	}

	runGit("init", "-b", "master")
	runGit("add", "-A")
	runGit("commit", "-m", "test conf")
	runGit("remote", "add", "origin", dirs.ConfDir)
}

// swapPortsDir moves the shared ports clone aside for the rest of the test and restores it
// afterwards, so a test can exercise the "no ports" cases without breaking the tests that
// run later. When empty is true, an empty ports dir is left in place.
func swapPortsDir(t *testing.T, empty bool) {
	t.Helper()

	backupDir := filepath.Join(dirs.TmpDir, "ports-backup", t.Name())
	check(t, os.MkdirAll(filepath.Dir(backupDir), os.ModePerm))
	check(t, os.Rename(dirs.PortsDir, backupDir))
	t.Cleanup(func() {
		_ = os.RemoveAll(dirs.PortsDir)
		_ = os.Rename(backupDir, dirs.PortsDir)
	})

	if empty {
		check(t, os.MkdirAll(dirs.PortsDir, os.ModePerm))
	}
}

// materializeTestConf copies the embedded conf fixtures (the project-local port
// definitions) into the conf dir, so tests see the layout of a cloned conf.
func materializeTestConf(t *testing.T) {
	t.Helper()

	root := "testdata/conf"
	err := fs.WalkDir(confFS, root, func(path string, entry fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		rel, err := filepath.Rel(root, path)
		if err != nil {
			return err
		}
		if rel == "." {
			return nil
		}

		target := filepath.Join(dirs.ConfDir, rel)
		if entry.IsDir() {
			return os.MkdirAll(target, os.ModePerm)
		}

		bytes, err := confFS.ReadFile(path)
		if err != nil {
			return err
		}
		return os.WriteFile(target, bytes, os.ModePerm)
	})
	check(t, err)
}

// testCelerTomlPlatform reads the platform recorded in the generated celer.toml.
func testCelerTomlPlatform(t *testing.T) string {
	t.Helper()

	var conf struct {
		Main configs.Main `toml:"main"`
	}
	if _, err := toml.DecodeFile(filepath.Join(dirs.WorkspaceDir, "celer.toml"), &conf); err != nil {
		t.Fatalf("failed to read the generated celer.toml -> %v", err)
	}
	return conf.Main.Platform
}

func writeTestProject(t *testing.T, project configs.Project) {
	t.Helper()

	path := filepath.Join(dirs.ConfProjectsDir, project.Name+".toml")
	if err := os.MkdirAll(filepath.Dir(path), os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}

	bytes, err := toml.Marshal(project)
	check(t, err)
	if err := os.WriteFile(path, bytes, os.ModePerm); err != nil {
		t.Fatalf("WriteFile() error = %v", err)
	}
}

// hostTestPlatform returns the platform the host toolchain is detected as.
func hostTestPlatform() string {
	if runtime.GOOS == "windows" {
		return "x86_64-windows-clang-22.1.8"
	}
	return "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
}

// buildTestPlatform returns the platform the tests build for on this host.
func buildTestPlatform() string {
	switch runtime.GOOS {
	case "windows":
		if os.Getenv("GITHUB_ACTIONS") == "true" {
			return "x86_64-windows-msvc-enterprise-14"
		} else {
			return "x86_64-windows-msvc-community-14"
		}
	default:
		return "aarch64-linux-ubuntu-22.04-gcc-11.5.0"
	}
}

// runCommand builds the cobra command from `command`, feeds it `args` just
// like a user would on the shell, captures everything the command writes to
// stderr, and returns (stderr, executeErr).
func runCommand(t *testing.T, cmd *cobra.Command, args ...string) (string, error) {
	t.Helper()

	// Redirect os.Stderr to a pipe. Anything the command writes to
	// stderr lands in `reader`. We restore the original os.Stderr before returning.
	stdErr := os.Stderr
	reader, writer, err := os.Pipe()
	if err != nil {
		t.Fatal(err)
	}
	os.Stderr = writer

	// Drain the pipe in a goroutine while the command runs.
	done := make(chan string, 1)
	go func() {
		var buf strings.Builder
		_, _ = io.Copy(&buf, reader)
		done <- buf.String()
	}()

	// Run the command, then close the write end so the goroutine sees
	// EOF and exits. Order matters: close must happen before <-done,
	// otherwise the receive blocks forever.
	cmd.SetArgs(args)
	execErr := cmd.Execute()
	_ = writer.Close()
	os.Stderr = stdErr
	return <-done, execErr
}

// equals reports whether list1 and list2 contain the same elements, ignoring order.
func equals(list1, list2 []string) bool {
	if len(list1) != len(list2) {
		return false
	}
	for _, item := range list1 {
		if !slices.Contains(list2, item) {
			return false
		}
	}
	return true
}

// check fails the test when err is not nil.
func check(t *testing.T, err error) {
	t.Helper()
	if err != nil {
		t.Fatal(err)
	}
}

const (
	test_conf_repo_url    = "https://github.com/celer-pkg/celer-conf.git"
	test_conf_repo_branch = ""
)

// The command tests drive the command layer through its exported API; these
// aliases keep their local naming.
type (
	autoremoveCmd = cmds.AutoremoveCmd
	cleanCmd      = cmds.CleanCmd
	clientCmd     = cmds.ClientCmd
	configureCmd  = cmds.ConfigureCmd
	createCmd     = cmds.CreateCmd
	deployCmd     = cmds.DeployCmd
	initCmd       = cmds.InitCmd
	installCmd    = cmds.InstallCmd
	integrateCmd  = cmds.IntegrateCmd
	removeCmd     = cmds.RemoveCmd
	reverseCmd    = cmds.ReverseCmd
	searchCmd     = cmds.SearchCmd
	stripCmd      = cmds.StripCmd
	treeCmd       = cmds.TreeCmd
	updateCmd     = cmds.UpdateCmd
	versionCmd    = cmds.VersionCmd
	portInfo      = cmds.PortInfo
)

// preserveWorkspaceArtifacts copies the parts of the workspace that explain a
// failed integration test into the test's artifact directory.
func preserveWorkspaceArtifacts(t *testing.T) {
	t.Helper()

	if !t.Failed() {
		return
	}

	artifactDir := t.ArtifactDir()

	// Files that pin down which build was running and in what environment.
	for _, name := range []string{"celer.toml", "environment.sh", "environment.bat", "dag.json"} {
		src := filepath.Join(dirs.WorkspaceDir, name)
		copyToArtifacts(t, src, filepath.Join(artifactDir, name))
	}

	// Build logs live in buildtrees/<name@version>/<...>.log and hold the
	// output of the failing configure/build/install command.
	logs, err := filepath.Glob(filepath.Join(dirs.BuildtreesDir, "*", "*.log"))
	if err != nil {
		t.Logf("failed to list build logs -> %v", err)
		return
	}
	for _, logPath := range logs {
		rel, err := filepath.Rel(dirs.BuildtreesDir, logPath)
		if err != nil {
			t.Logf("failed to make %s relative to buildtrees -> %v", logPath, err)
			continue
		}
		copyToArtifacts(t, logPath, filepath.Join(artifactDir, "buildtrees", rel))
	}
}

func copyToArtifacts(t *testing.T, src, dst string) {
	t.Helper()

	if !fileio.PathExists(src) {
		return
	}
	if err := os.MkdirAll(filepath.Dir(dst), os.ModePerm); err != nil {
		t.Logf("failed to create artifact dir for %s -> %v", dst, err)
		return
	}
	if err := fileio.CopyFile(src, dst); err != nil {
		t.Logf("failed to copy %s to artifacts -> %v", src, err)
	}
}
