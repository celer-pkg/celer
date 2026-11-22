package script

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestFixupInstalledScripts_StripsTransientTokens(t *testing.T) {
	root := t.TempDir()
	staging := filepath.Join(t.TempDir(), "staging-foo@1.0-123-456")
	pcPath := filepath.Join(root, "lib", "pkgconfig", "foo.pc")
	mustWrite(t, pcPath, ""+
		"prefix=${pcfiledir}/../..\n"+
		"Libs.private: -L"+staging+"/lib -R"+staging+"/lib -lbar -lbaz\n")

	if err := FixupInstalledScripts(root, staging); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, pcPath)
	if strings.Contains(got, "staging-") {
		t.Fatalf("transient path leaked:\n%s", got)
	}
	want := "prefix=${pcfiledir}/../..\nLibs.private:   -lbar -lbaz\n"
	if got != want {
		t.Fatalf("got:\n%q\nwant:\n%q", got, want)
	}
}

func TestFixupInstalledScripts_StripsTransientTokensFromScripts(t *testing.T) {
	root := t.TempDir()
	staging := filepath.Join(t.TempDir(), "staging-foo@1.0-123-456")
	configPath := filepath.Join(root, "bin", "foo-config")
	mustWrite(t, configPath, ""+
		"#!/bin/sh\n"+
		"includedir=-I"+staging+"/include/foo\n")

	if err := FixupInstalledScripts(root, staging); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, configPath)
	if strings.Contains(got, "staging-") {
		t.Fatalf("transient path leaked:\n%s", got)
	}
}

func TestFixupInstalledScripts_SelfLocatesConfigPrefix(t *testing.T) {
	root := t.TempDir()
	configPath := filepath.Join(root, "bin", "foo-config")
	mustWrite(t, configPath, ""+
		"#!/bin/sh\n"+
		"prefix=\""+root+"\"\n"+
		"includedir=\"${prefix}/include\"\n")

	if err := FixupInstalledScripts(root, ""); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, configPath)
	want := "" +
		"#!/bin/sh\n" +
		"prefix=$(dirname $(dirname \"$0\"))\n" +
		"includedir=\"${prefix}/include\"\n"
	if got != want {
		t.Fatalf("got:\n%s\nwant:\n%s", got, want)
	}
}

func TestFixupInstalledScripts_SelfLocatesNestedPrefix(t *testing.T) {
	root := t.TempDir()
	configPath := filepath.Join(root, "bin", "foo-config")
	mustWrite(t, configPath, "#!/bin/sh\nprefix="+filepath.Join(root, "opt", "foo")+"\n")

	if err := FixupInstalledScripts(root, ""); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, configPath)
	want := "#!/bin/sh\nprefix=$(dirname $(dirname \"$0\"))/opt/foo\n"
	if got != want {
		t.Fatalf("got:\n%s\nwant:\n%s", got, want)
	}
}

func TestFixupInstalledScripts_LeavesExternalPrefix(t *testing.T) {
	root := t.TempDir()
	configPath := filepath.Join(root, "bin", "foo-config")
	content := "#!/bin/sh\nprefix=/usr\n"
	mustWrite(t, configPath, content)

	if err := FixupInstalledScripts(root, ""); err != nil {
		t.Fatal(err)
	}

	if got := mustRead(t, configPath); got != content {
		t.Fatalf("got:\n%s\nwant unchanged:\n%s", got, content)
	}
}

func TestFixupInstalledScripts_LeavesNonShellScripts(t *testing.T) {
	root := t.TempDir()
	binPath := filepath.Join(root, "bin", "tool")
	content := "prefix=" + root + "\n"
	mustWrite(t, binPath, content)

	if err := FixupInstalledScripts(root, ""); err != nil {
		t.Fatal(err)
	}

	if got := mustRead(t, binPath); got != content {
		t.Fatalf("got:\n%s\nwant unchanged:\n%s", got, content)
	}
}

func TestFixupInstalledScripts_IgnoresNonPkgconfigPC(t *testing.T) {
	root := t.TempDir()
	staging := filepath.Join(t.TempDir(), "staging-foo@1.0-1-2")
	pcPath := filepath.Join(root, "share", "cmake", "stray.pc")
	content := "prefix=" + staging + "\n"
	mustWrite(t, pcPath, content)

	if err := FixupInstalledScripts(root, staging); err != nil {
		t.Fatal(err)
	}

	if got := mustRead(t, pcPath); got != content {
		t.Fatalf("got:\n%s\nwant unchanged:\n%s", got, content)
	}
}

func TestFixupInstalledScripts_SkipsBinaries(t *testing.T) {
	root := t.TempDir()
	staging := filepath.Join(t.TempDir(), "staging-foo@1.0-1-2")
	binPath := filepath.Join(root, "lib", "libfoo.so")
	content := append([]byte{0x7f, 'E', 'L', 'F', 0x00}, []byte("-L"+staging+"/lib")...)
	if err := os.MkdirAll(filepath.Dir(binPath), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(binPath, content, 0o755); err != nil {
		t.Fatal(err)
	}

	if err := FixupInstalledScripts(root, staging); err != nil {
		t.Fatal(err)
	}

	if got := mustReadBytes(t, binPath); string(got) != string(content) {
		t.Fatalf("binary file was modified")
	}
}

func TestFixupInstalledScripts_NonexistentDir(t *testing.T) {
	if err := FixupInstalledScripts(filepath.Join(t.TempDir(), "does-not-exist"), ""); err != nil {
		t.Fatalf("FixupInstalledPaths() should skip a nonexistent dir, got: %v", err)
	}
}

func mustWrite(t *testing.T, path, content string) {
	t.Helper()
	if err := os.MkdirAll(filepath.Dir(path), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(path, []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}
}

func mustRead(t *testing.T, path string) string {
	t.Helper()
	return string(mustReadBytes(t, path))
}

func mustReadBytes(t *testing.T, path string) []byte {
	t.Helper()
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	return data
}
