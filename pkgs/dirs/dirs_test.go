package dirs

import (
	"os"
	"path/filepath"
	"regexp"
	"testing"
)

// NewTmpFilesDir must create a unique timestamped dir under TmpFilesDir, never
// return TmpFilesDir itself, and never collide across concurrent calls.
func TestNewTmpFilesDir(t *testing.T) {
	origTmp := TmpFilesDir
	TmpFilesDir = filepath.Join(t.TempDir(), "tmp", "files")
	defer func() { TmpFilesDir = origTmp }()

	// TmpFilesDir itself does not need to pre-exist.
	first, err := NewTmpFilesDir()
	if err != nil {
		t.Fatalf("NewTmpFilesDir failed: %v", err)
	}
	if first == TmpFilesDir {
		t.Fatalf("NewTmpFilesDir returned TmpFilesDir itself, want a subdirectory")
	}
	if !filepath.IsAbs(first) || filepath.Dir(filepath.Dir(first)) != filepath.Dir(TmpFilesDir) {
		t.Fatalf("NewTmpFilesDir returned %q, want a dir directly under %q", first, TmpFilesDir)
	}
	if info, err := os.Stat(first); err != nil || !info.IsDir() {
		t.Fatalf("expected created dir %q, stat err: %v", first, err)
	}

	// Dir name is timestamp-based and unique per call.
	pattern := regexp.MustCompile(`^\d+-\d+$`)
	if base := filepath.Base(first); !pattern.MatchString(base) {
		t.Fatalf("dir name %q does not match '<timestamp>-<random>'", base)
	}
	second, err := NewTmpFilesDir()
	if err != nil {
		t.Fatalf("NewTmpFilesDir failed: %v", err)
	}
	if second == first {
		t.Fatalf("two calls returned the same dir %q", first)
	}
}
