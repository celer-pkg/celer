package fileio

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestReplaceContent(t *testing.T) {
	t.Run("empty file", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "config")
		if err := os.WriteFile(path, nil, 0644); err != nil {
			t.Fatal(err)
		}

		if err := ReplaceContent(path, "new line", func(string) bool { return false }); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, path, "new line\n")
	})

	t.Run("append", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "config")
		if err := os.WriteFile(path, []byte("first line\n"), 0644); err != nil {
			t.Fatal(err)
		}

		if err := ReplaceContent(path, "second line", func(string) bool {
			return false
		}); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, path, "first line\nsecond line\n")
	})

	t.Run("replace matching line", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "config")
		if err := os.WriteFile(path, []byte("keep\nold value\n"), 0644); err != nil {
			t.Fatal(err)
		}

		if err := ReplaceContent(path, "new value", func(line string) bool {
			return strings.HasPrefix(line, "old")
		}); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, path, "keep\nnew value\n")
	})

	t.Run("preserve unrelated lines", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "config")
		if err := os.WriteFile(path, []byte("before\nremove me\nafter\n"), 0644); err != nil {
			t.Fatal(err)
		}

		if err := ReplaceContent(path, "added", func(line string) bool {
			return line == "remove me"
		}); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, path, "before\nafter\nadded\n")
	})
}

func TestRemoveContent(t *testing.T) {
	t.Run("missing file", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "missing")

		if err := RemoveContent(path, func(string) bool { return true }); err != nil {
			t.Fatal(err)
		}
		if _, err := os.Stat(path); !os.IsNotExist(err) {
			t.Fatalf("os.Stat() error = %v, want missing file", err)
		}
	})

	t.Run("remove matching line", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "config")
		if err := os.WriteFile(path, []byte("keep\nremove me\nafter\n"), 0644); err != nil {
			t.Fatal(err)
		}

		if err := RemoveContent(path, func(line string) bool {
			return line == "remove me"
		}); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, path, "keep\nafter\n")
	})

	t.Run("remove all lines", func(t *testing.T) {
		path := filepath.Join(t.TempDir(), "config")
		if err := os.WriteFile(path, []byte("remove\nremove\n"), 0644); err != nil {
			t.Fatal(err)
		}

		if err := RemoveContent(path, func(line string) bool {
			return line == "remove"
		}); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, path, "")
	})
}

func assertFileContent(t *testing.T, path, want string) {
	t.Helper()
	got, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	if string(got) != want {
		t.Fatalf("file content = %q, want %q", string(got), want)
	}
}
