package buildsystems

import (
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/pkgs/dirs"
)

func TestPortConfig_DepsRoot(t *testing.T) {
	dirs.Init(t.TempDir())

	var empty PortConfig
	if got := empty.DepsRoot(); got != dirs.TmpDepsDir {
		t.Fatalf("empty TmpDepsRoot: DepsRoot() = %q, want %q", got, dirs.TmpDepsDir)
	}

	jobDir := filepath.Join(t.TempDir(), "job-abc")
	cfg := PortConfig{TmpDepsRoot: jobDir, LibraryDir: filepath.Join("plat", "proj", "release")}
	if got := cfg.DepsRoot(); got != jobDir {
		t.Fatalf("DepsRoot() = %q, want %q", got, jobDir)
	}

	got := cfg.DepsPath(cfg.LibraryDir, "lib")
	want := filepath.Join(jobDir, "plat", "proj", "release", "lib")
	if got != want {
		t.Fatalf("DepsPath() = %q, want %q", got, want)
	}
}
