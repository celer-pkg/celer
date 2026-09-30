package pc

import (
	"os"
	"path/filepath"
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

// TestFixupPkgConfigFile_VendorEmbeddedBuildDir repros the OGRE/assimp layout:
// the .pc was generated with CMAKE_INSTALL_PREFIX pointing at an ExternalProject
// build dir, then the whole tree was copied to opt/<vendor> without rewriting
// the .pc. Fixup must rewrite the baked build-dir absolute paths to ${prefix},
// not the relocated path.
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
