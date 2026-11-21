package dirs

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// useTempWorkspace points WorkspaceDir at a throwaway dir for the duration of
// the test. Only WorkspaceDir matters here: both NewTmpFilesDir and
// NewTmpStagingDir derive their parent from it.
func useTempWorkspace(t *testing.T) string {
	t.Helper()
	orig := WorkspaceDir
	ws := t.TempDir()
	WorkspaceDir = ws
	t.Cleanup(func() { WorkspaceDir = orig })
	return ws
}

// NewTmpFilesDir must create a unique dir directly under <workspace>/tmp,
// never return the parent itself, and never collide across calls.
func TestNewTmpFilesDir(t *testing.T) {
	ws := useTempWorkspace(t)
	tmpRoot := filepath.Join(ws, "tmp")

	// tmp itself does not need to pre-exist.
	first, err := NewTmpFilesDir()
	if err != nil {
		t.Fatalf("NewTmpFilesDir failed: %v", err)
	}
	if filepath.Dir(first) != tmpRoot {
		t.Fatalf("NewTmpFilesDir returned %q, want a dir directly under %q", first, tmpRoot)
	}
	if info, err := os.Stat(first); err != nil || !info.IsDir() {
		t.Fatalf("expected created dir %q, stat err: %v", first, err)
	}
	if base := filepath.Base(first); !strings.HasPrefix(base, "files-") {
		t.Fatalf("dir name %q does not start with 'files-'", base)
	}

	second, err := NewTmpFilesDir()
	if err != nil {
		t.Fatalf("NewTmpFilesDir failed: %v", err)
	}
	if second == first {
		t.Fatalf("two calls returned the same dir %q", first)
	}
}

// NewTmpStagingDir must create a unique staging dir under <workspace>/tmp,
// prefixed with the port's name@version.
func TestNewTmpStagingDir(t *testing.T) {
	ws := useTempWorkspace(t)
	tmpRoot := filepath.Join(ws, "tmp")

	const nameVersion = "ffmpeg@5.1.6"
	first, err := NewTmpStagingDir(nameVersion)
	if err != nil {
		t.Fatalf("NewTmpStagingDir failed: %v", err)
	}
	if filepath.Dir(first) != tmpRoot {
		t.Fatalf("NewTmpStagingDir returned %q, want a dir directly under %q", first, tmpRoot)
	}
	if info, err := os.Stat(first); err != nil || !info.IsDir() {
		t.Fatalf("expected created dir %q, stat err: %v", first, err)
	}
	if base := filepath.Base(first); !strings.HasPrefix(base, "staging-"+nameVersion+"-") {
		t.Fatalf("dir name %q does not start with 'staging-%s-'", base, nameVersion)
	}

	second, err := NewTmpStagingDir(nameVersion)
	if err != nil {
		t.Fatalf("NewTmpStagingDir failed: %v", err)
	}
	if second == first {
		t.Fatalf("two calls returned the same dir %q", first)
	}

	// Different ports must never share a staging dir.
	other, err := NewTmpStagingDir("x264@stable")
	if err != nil {
		t.Fatalf("NewTmpStagingDir failed: %v", err)
	}
	if other == first {
		t.Fatalf("different ports returned the same staging dir %q", first)
	}
}
