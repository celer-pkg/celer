package fileio

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

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

func TestFindNestedFolderRos2Scenario(t *testing.T) {
	// Simulate buildtrees/ros2@humble/src containing multiple top-level
	// entries including a legitimate "src" directory (like the ros2 archive).
	parent := t.TempDir()
	srcDir := filepath.Join(parent, "src")
	for _, name := range []string{"bin", "cmake", "include", "lib", "share", "src", "ssl", "tools"} {
		if err := os.MkdirAll(filepath.Join(srcDir, name), 0o755); err != nil {
			t.Fatal(err)
		}
	}
	if err := os.WriteFile(filepath.Join(srcDir, "setup.sh"), []byte("x"), 0o644); err != nil {
		t.Fatal(err)
	}

	if got := findNestedFolder(srcDir); got != "" {
		t.Fatalf("expected no nested folder for multi-entry dir, got %q", got)
	}
}

func TestFindNestedFolderSingleWrapper(t *testing.T) {
	// A single wrapping directory named like the parent should still flatten.
	parent := t.TempDir()
	wrapper := filepath.Join(parent, "cmake-3.30.3", "cmake-3.30.3")
	if err := os.MkdirAll(wrapper, 0o755); err != nil {
		t.Fatal(err)
	}
	destDir := filepath.Join(parent, "cmake-3.30.3")
	if got := findNestedFolder(destDir); got != filepath.Join(destDir, "cmake-3.30.3") {
		t.Fatalf("expected nested folder %q, got %q", filepath.Join(destDir, "cmake-3.30.3"), got)
	}
}

func TestFlattenNestedDir(t *testing.T) {
	setup := func(t *testing.T) string {
		t.Helper()
		dir := t.TempDir()
		wrapper := filepath.Join(dir, "ffmpeg-4.4")
		if err := os.MkdirAll(filepath.Join(wrapper, "libavcodec"), 0o755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(wrapper, "README"), []byte("x"), 0o644); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(wrapper, "libavcodec", "utils.c"), []byte("y"), 0o644); err != nil {
			t.Fatal(err)
		}
		return dir
	}

	t.Run("single wrapper is flattened", func(t *testing.T) {
		dir := setup(t)

		if err := FlattenNestedDir(dir); err != nil {
			t.Fatal(err)
		}

		// Wrapper's contents moved up, wrapper itself gone.
		assertFileContent(t, filepath.Join(dir, "README"), "x")
		assertFileContent(t, filepath.Join(dir, "libavcodec", "utils.c"), "y")
		if _, err := os.Lstat(filepath.Join(dir, "ffmpeg-4.4")); !os.IsNotExist(err) {
			t.Fatalf("wrapper should be removed, got err = %v", err)
		}
	})

	t.Run("wrapper mode is kept", func(t *testing.T) {
		dir := setup(t)
		wrapper := filepath.Join(dir, "ffmpeg-4.4")
		if err := os.Chmod(wrapper, 0o700); err != nil {
			t.Fatal(err)
		}

		if err := FlattenNestedDir(dir); err != nil {
			t.Fatal(err)
		}

		// The wrapper's inode takes dir's place, so its mode travels with it.
		info, err := os.Stat(dir)
		if err != nil {
			t.Fatal(err)
		}
		if info.Mode().Perm() != 0o700 {
			t.Fatalf("mode = %o, want 700", info.Mode().Perm())
		}
	})

	t.Run("include wrapper is kept", func(t *testing.T) {
		dir := setup(t)
		if err := os.RemoveAll(filepath.Join(dir, "ffmpeg-4.4")); err != nil {
			t.Fatal(err)
		}
		if err := os.MkdirAll(filepath.Join(dir, "include"), 0o755); err != nil {
			t.Fatal(err)
		}

		if err := FlattenNestedDir(dir); err != nil {
			t.Fatal(err)
		}

		if _, err := os.Lstat(filepath.Join(dir, "include")); err != nil {
			t.Fatalf("include dir should be kept: %v", err)
		}
	})

	t.Run("multi-entry dir is untouched", func(t *testing.T) {
		dir := setup(t)
		if err := os.WriteFile(filepath.Join(dir, "extra.txt"), []byte("z"), 0o644); err != nil {
			t.Fatal(err)
		}

		if err := FlattenNestedDir(dir); err != nil {
			t.Fatal(err)
		}

		// dir now has two entries, nothing should be moved.
		if _, err := os.Lstat(filepath.Join(dir, "ffmpeg-4.4")); err != nil {
			t.Fatalf("wrapper should be kept: %v", err)
		}
	})
}

func TestMoveFile(t *testing.T) {
	t.Run("regular file", func(t *testing.T) {
		dir := t.TempDir()
		src := filepath.Join(dir, "src")
		dst := filepath.Join(dir, "dst")
		if err := os.WriteFile(src, []byte("hello"), 0o755); err != nil {
			t.Fatal(err)
		}

		if err := MoveFile(src, dst); err != nil {
			t.Fatal(err)
		}

		assertFileContent(t, dst, "hello")
		if _, err := os.Lstat(src); !os.IsNotExist(err) {
			t.Fatalf("src should be removed, got err = %v", err)
		}
		info, err := os.Stat(dst)
		if err != nil {
			t.Fatal(err)
		}
		if info.Mode().Perm() != 0o755 {
			t.Fatalf("mode = %o, want 755", info.Mode().Perm())
		}
	})

	t.Run("missing dst parent dirs", func(t *testing.T) {
		dir := t.TempDir()
		src := filepath.Join(dir, "src")
		dst := filepath.Join(dir, "a", "b", "dst")
		if err := os.WriteFile(src, []byte("hello"), 0o644); err != nil {
			t.Fatal(err)
		}

		if err := MoveFile(src, dst); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, dst, "hello")
	})

	t.Run("symlink is recreated and source removed", func(t *testing.T) {
		dir := t.TempDir()
		target := filepath.Join(dir, "target")
		if err := os.WriteFile(target, []byte("hello"), 0o644); err != nil {
			t.Fatal(err)
		}
		src := filepath.Join(dir, "link")
		if err := os.Symlink(target, src); err != nil {
			t.Fatal(err)
		}
		dst := filepath.Join(dir, "moved", "link")

		if err := MoveFile(src, dst); err != nil {
			t.Fatal(err)
		}

		got, err := os.Readlink(dst)
		if err != nil {
			t.Fatal(err)
		}
		if got != target {
			t.Fatalf("link target = %q, want %q", got, target)
		}
		if _, err := os.Lstat(src); !os.IsNotExist(err) {
			t.Fatalf("src symlink should be removed, got err = %v", err)
		}
	})

	t.Run("existing read-only dst is replaced", func(t *testing.T) {
		dir := t.TempDir()
		src := filepath.Join(dir, "src")
		dst := filepath.Join(dir, "dst")
		if err := os.WriteFile(src, []byte("new"), 0o644); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(dst, []byte("old"), 0o444); err != nil {
			t.Fatal(err)
		}

		if err := MoveFile(src, dst); err != nil {
			t.Fatal(err)
		}
		assertFileContent(t, dst, "new")
	})
}

func TestCopyFile(t *testing.T) {
	t.Run("symlink is recreated", func(t *testing.T) {
		dir := t.TempDir()
		target := filepath.Join(dir, "target")
		if err := os.WriteFile(target, []byte("hello"), 0o644); err != nil {
			t.Fatal(err)
		}
		src := filepath.Join(dir, "link")
		if err := os.Symlink(target, src); err != nil {
			t.Fatal(err)
		}
		dst := filepath.Join(dir, "copy")

		if err := CopyFile(src, dst); err != nil {
			t.Fatal(err)
		}

		got, err := os.Readlink(dst)
		if err != nil {
			t.Fatal(err)
		}
		if got != target {
			t.Fatalf("link target = %q, want %q", got, target)
		}
		// Source symlink is kept, unlike MoveFile.
		if _, err := os.Lstat(src); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("regular file keeps mode", func(t *testing.T) {
		dir := t.TempDir()
		src := filepath.Join(dir, "src")
		dst := filepath.Join(dir, "dst")
		if err := os.WriteFile(src, []byte("hello"), 0o755); err != nil {
			t.Fatal(err)
		}

		if err := CopyFile(src, dst); err != nil {
			t.Fatal(err)
		}

		assertFileContent(t, dst, "hello")
		info, err := os.Stat(dst)
		if err != nil {
			t.Fatal(err)
		}
		if info.Mode().Perm() != 0o755 {
			t.Fatalf("mode = %o, want 755", info.Mode().Perm())
		}
	})
}
