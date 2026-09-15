package configs

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/pkgs/dirs"
)

func TestApplyTmpDepsRoot(t *testing.T) {
	dirs.Init(t.TempDir())
	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}

	libraryDir := filepath.Join("plat", "proj", "release")
	p := Port{
		ctx: celer,
		BuildConfigs: []buildsystems.BuildConfig{{
			PortConfig: buildsystems.PortConfig{
				HostName:   "x86_64-linux",
				LibraryDir: libraryDir,
			},
		}},
	}
	p.MatchedConfig = &p.BuildConfigs[0]

	jobDir := filepath.Join(dirs.TmpDepsDir, "job-test")
	p.applyTmpDepsRoot(jobDir)

	if p.tmpDepsRoot != jobDir {
		t.Fatalf("tmpDepsRoot = %q, want %q", p.tmpDepsRoot, jobDir)
	}
	if p.MatchedConfig.PortConfig.TmpDepsRoot != jobDir {
		t.Fatalf("PortConfig.TmpDepsRoot = %q, want %q", p.MatchedConfig.PortConfig.TmpDepsRoot, jobDir)
	}
	wantDeps := filepath.Join(jobDir, libraryDir)
	if p.tmpDepsDir != wantDeps {
		t.Fatalf("tmpDepsDir = %q, want %q", p.tmpDepsDir, wantDeps)
	}
	if got, ok := p.exprVars.Lookup("DEPS_DIR"); !ok || got != wantDeps {
		t.Fatalf("DEPS_DIR = %q ok=%v, want %q", got, ok, wantDeps)
	}
	wantDev := filepath.Join(jobDir, "x86_64-linux-dev")
	if got, ok := p.exprVars.Lookup("DEV_DEPS_DIR"); !ok || got != wantDev {
		t.Fatalf("DEV_DEPS_DIR = %q ok=%v, want %q", got, ok, wantDev)
	}
}

func TestMkdirTempJobDirsAreIsolated(t *testing.T) {
	dirs.Init(t.TempDir())
	if err := os.MkdirAll(dirs.TmpDepsDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	jobA, err := os.MkdirTemp(dirs.TmpDepsDir, "job-*")
	if err != nil {
		t.Fatal(err)
	}
	jobB, err := os.MkdirTemp(dirs.TmpDepsDir, "job-*")
	if err != nil {
		t.Fatal(err)
	}
	if jobA == jobB {
		t.Fatal("expected distinct tmp/deps job dirs")
	}
	if !strings.HasPrefix(jobA, dirs.TmpDepsDir) || !strings.HasPrefix(jobB, dirs.TmpDepsDir) {
		t.Fatalf("job dirs must live under tmp/deps: %q %q", jobA, jobB)
	}

	markerA := filepath.Join(jobA, "a.txt")
	markerB := filepath.Join(jobB, "b.txt")
	if err := os.WriteFile(markerA, []byte("a"), 0o644); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(markerB, []byte("b"), 0o644); err != nil {
		t.Fatal(err)
	}
	if err := os.RemoveAll(jobA); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(markerB); err != nil {
		t.Fatalf("removing job A must not touch job B: %v", err)
	}
}
