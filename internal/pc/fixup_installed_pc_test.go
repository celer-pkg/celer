package pc

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestFixupPkgConfigFile_StandardPrefix(t *testing.T) {
	root := t.TempDir()
	pcPath := filepath.Join(root, "lib", "pkgconfig", "foo.pc")
	mustWritePc(t, pcPath, "prefix="+root+"\nlibdir="+root+"/lib\nincludedir="+root+"/include\n")

	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, pcPath)
	want := "prefix=${pcfiledir}/../..\nlibdir=${prefix}/lib\nincludedir=${prefix}/include\n"
	if got != want {
		t.Fatalf("got:\n%s\nwant:\n%s", got, want)
	}
}

func TestFixupPkgConfigFile_VendorEmbeddedBuildDir(t *testing.T) {
	root := t.TempDir()
	buildDir := filepath.Join(t.TempDir(), "assimp_install")
	pcPath := filepath.Join(root, "opt", "rviz_assimp_vendor", "lib", "pkgconfig", "assimp.pc")
	mustWritePc(t, pcPath, ""+
		"prefix="+buildDir+"\n"+
		"exec_prefix="+buildDir+"\n"+
		"libdir="+buildDir+"/lib\n"+
		"includedir="+buildDir+"/include\n")

	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, pcPath)
	want := "prefix=${pcfiledir}/../..\n" +
		"exec_prefix=${prefix}\n" +
		"libdir=${prefix}/lib\n" +
		"includedir=${prefix}/include\n"
	if got != want {
		t.Fatalf("got:\n%s\nwant:\n%s", got, want)
	}
}

func TestFixupPkgConfigFile_NoPrefixLine(t *testing.T) {
	root := t.TempDir()
	pcPath := filepath.Join(root, "lib", "pkgconfig", "benchmark_main.pc")
	mustWritePc(t, pcPath, ""+
		"libdir="+root+"/lib\n"+
		"\n"+
		"Name: benchmark\n"+
		"Description: Google microbenchmark framework (with main() function)\n"+
		"Version: 1.9.4\n"+
		"Requires: benchmark\n"+
		"Libs: -L${libdir} -lbenchmark_main\n")

	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, pcPath)
	want := "prefix=${pcfiledir}/../..\n" +
		"libdir=${prefix}/lib\n" +
		"\n" +
		"Name: benchmark\n" +
		"Description: Google microbenchmark framework (with main() function)\n" +
		"Version: 1.9.4\n" +
		"Requires: benchmark\n" +
		"Libs: -L${libdir} -lbenchmark_main\n"
	if got != want {
		t.Fatalf("got:\n%s\nwant:\n%s", got, want)
	}
}

func TestFixupPkgConfigFile_NoPrefixLineTwice(t *testing.T) {
	root := t.TempDir()
	pcPath := filepath.Join(root, "lib", "pkgconfig", "benchmark_main.pc")
	mustWritePc(t, pcPath, "libdir="+root+"/lib\n\nName: benchmark\n")

	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}

	first := mustRead(t, pcPath)
	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}
	if second := mustRead(t, pcPath); second != first {
		t.Fatalf("second run changed output:\nfirst:\n%s\nsecond:\n%s", first, second)
	}
}

func TestFixupPkgConfigFile_StagingLibPathFlags(t *testing.T) {
	root := t.TempDir()
	staging := t.TempDir()
	pcPath := filepath.Join(root, "lib", "pkgconfig", "libevent_openssl.pc")
	mustWritePc(t, pcPath, ""+
		"prefix="+root+"\n"+
		"exec_prefix=${prefix}\n"+
		"libdir=${prefix}/lib\n"+
		"\n"+
		"Name: libevent_openssl\n"+
		"Libs: -L${libdir} -levent_openssl\n"+
		"Libs.private:   -L"+staging+"/lib/libssl.so -L"+staging+"/lib/libcrypto.so\n"+
		"Cflags: -I${prefix}/include\n")

	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}

	got := mustRead(t, pcPath)
	if strings.Contains(got, staging) {
		t.Fatalf("staging path leaked:\n%s", got)
	}
	want := "prefix=${pcfiledir}/../..\n" +
		"exec_prefix=${prefix}\n" +
		"libdir=${prefix}/lib\n" +
		"\n" +
		"Name: libevent_openssl\n" +
		"Libs: -L${libdir} -levent_openssl\n" +
		"Libs.private: -lssl -lcrypto\n" +
		"Cflags: -I${prefix}/include\n"
	if got != want {
		t.Fatalf("got:\n%s\nwant:\n%s", got, want)
	}
}

func TestLibNameFromPathFlag(t *testing.T) {
	cases := map[string]string{
		"-L/usr/lib/libssl.so":            "ssl",
		"-L/usr/lib/libfoo.so.1.2":        "foo",
		"-L/usr/lib/libbar.a":             "bar",
		"-L${libdir}":                     "",
		"-L/usr/lib":                      "",
		"-L/usr/lib/lib64":                "",
		"-lssl":                           "",
		"/usr/lib/libssl.so":              "",
		"-L/opt/x/lib/libevent_core.so.2": "event_core",
	}
	for field, want := range cases {
		if got := readLibNameFromPath(field); got != want {
			t.Errorf("libNameFromPathFlag(%q) = %q, want %q", field, got, want)
		}
	}
}

func TestFixupPkgConfigFile_IgnoresNonPkgconfigDir(t *testing.T) {
	root := t.TempDir()
	// A .pc outside a pkgconfig/ dir (unusual) must be left untouched.
	pcPath := filepath.Join(root, "share", "cmake", "stray.pc")
	content := "prefix=" + root + "\n"
	mustWritePc(t, pcPath, content)

	if err := FixupPkgConfigFile(root); err != nil {
		t.Fatal(err)
	}

	if got := mustRead(t, pcPath); got != content {
		t.Fatalf("got:\n%s\nwant unchanged:\n%s", got, content)
	}
}

func TestFixupPkgConfigFile_NonexistentDir(t *testing.T) {
	// A nobuild port has no installed tree; fixup must be a no-op, not an error.
	if err := FixupPkgConfigFile(filepath.Join(t.TempDir(), "does-not-exist")); err != nil {
		t.Fatalf("FixupPkgConfigFile() should skip a nonexistent dir, got: %v", err)
	}
}

func mustWritePc(t *testing.T, path, content string) {
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
	b, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	return string(b)
}
