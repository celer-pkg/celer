package buildsystems

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// T1: Absolute prefix is rewritten to ${pcfiledir}/../..
func TestApply_RewritesAbsolutePrefix(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/some/absolute/installation/path\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "prefix=${pcfiledir}/../..") {
		t.Fatalf("expected prefix=${pcfiledir}/../.., got:\n%s", result)
	}
}

// T2: "prefix =" with space is normalized and rewritten.
func TestApply_NormalizesPrefixSpacing(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix =/some/path\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "prefix=${pcfiledir}/../..") {
		t.Fatalf("expected prefix=${pcfiledir}/../.., got:\n%s", result)
	}
}

// T3: exec_prefix, libdir, includedir using ${prefix} pass through untouched.
func TestApply_PreservesPrefixChainedVariables(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := `prefix=/old/path
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
sharedlibdir=${prefix}/lib
`
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "exec_prefix=${prefix}") {
		t.Fatalf("exec_prefix should pass through, got:\n%s", result)
	}
	if !strings.Contains(result, "libdir=${exec_prefix}/lib") {
		t.Fatalf("libdir should pass through, got:\n%s", result)
	}
	if !strings.Contains(result, "includedir=${prefix}/include") {
		t.Fatalf("includedir should pass through, got:\n%s", result)
	}
	if !strings.Contains(result, "sharedlibdir=${prefix}/lib") {
		t.Fatalf("sharedlibdir should pass through, got:\n%s", result)
	}
}

// T4: pkgdatadir with ${pc_sysrootdir} is cleaned.
func TestApply_StripsPcSysrootdir(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/usr\npkgdatadir=${pc_sysrootdir}/usr/share/somepkg\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if strings.Contains(result, "${pc_sysrootdir}") {
		t.Fatalf("pc_sysrootdir should be stripped, got:\n%s", result)
	}
}

// T5: Libs with absolute -L path is normalized to -L${libdir}.
func TestApply_NormalizesLibsAbsoluteLPath(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/usr\nLibs: -L/usr/lib/x86_64-linux-gnu -lfoo\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "-L${libdir}") {
		t.Fatalf("expected -L${libdir}, got:\n%s", result)
	}
	if strings.Contains(result, "/usr/lib/x86_64-linux-gnu") {
		t.Fatalf("absolute -L path should be removed, got:\n%s", result)
	}
}

// T6: Libs with already-correct -L${libdir} is preserved.
func TestApply_PreservesAlreadyCorrectLibs(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/usr\nLibs: -L${libdir} -lfoo\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "Libs: -L${libdir} -lfoo") {
		t.Fatalf("correct Libs should be preserved, got:\n%s", result)
	}
}

// T7: Libs.private with absolute -L path is normalized.
func TestApply_NormalizesLibsPrivateAbsoluteLPath(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/usr\nLibs.private: -L/usr/lib/x86_64-linux-gnu -lbar\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "-L${libdir}") {
		t.Fatalf("expected -L${libdir} in Libs.private, got:\n%s", result)
	}
}

// T8: All three directories (lib, share, lib64) are processed.
func TestApply_ProcessesAllPkgconfigDirs(t *testing.T) {
	dir := t.TempDir()
	for _, sub := range []string{"lib/pkgconfig", "share/pkgconfig", "lib64/pkgconfig"} {
		pkgDir := filepath.Join(dir, sub)
		if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
			t.Fatal(err)
		}
		content := "prefix=/original/" + sub + "\n"
		if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
			t.Fatal(err)
		}
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	for _, sub := range []string{"lib/pkgconfig", "share/pkgconfig", "lib64/pkgconfig"} {
		result := readFile(t, filepath.Join(dir, sub, "test.pc"))
		if !strings.Contains(result, "prefix=${pcfiledir}/../..") {
			t.Fatalf("[%s] expected prefix=${pcfiledir}/../.., got:\n%s", sub, result)
		}
	}
}

// T9: Non-existent pkgconfig directories are handled gracefully.
func TestApply_HandlesMissingDirsGracefully(t *testing.T) {
	dir := t.TempDir()
	// No pkgconfig dirs created — should not error.

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatalf("Apply() should not error on missing dirs: %v", err)
	}
}

// T10: Read-only file is made writable and rewritten.
func TestApply_HandlesReadOnlyFile(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/foo\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), 0400); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "prefix=${pcfiledir}/../..") {
		t.Fatalf("expected prefix=${pcfiledir}/../.. for read-only file, got:\n%s", result)
	}
}

// T11: Non-.pc files in pkgconfig directory are ignored.
func TestApply_IgnoresNonPcFiles(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	pcContent := "prefix=/original\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "lib.pc"), []byte(pcContent), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	txtContent := "this is not a pc file\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "README.txt"), []byte(txtContent), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	pcResult := readFile(t, filepath.Join(pkgDir, "lib.pc"))
	if !strings.Contains(pcResult, "prefix=${pcfiledir}/../..") {
		t.Fatalf("pc file should be rewritten, got:\n%s", pcResult)
	}

	txtResult := readFile(t, filepath.Join(pkgDir, "README.txt"))
	if txtResult != txtContent {
		t.Fatalf("non-pc file should not be modified, got:\n%s", txtResult)
	}
}

// T12: Double-space normalization in Libs.
func TestApply_NormalizesLibsDoubleSpaces(t *testing.T) {
	dir := t.TempDir()
	pkgDir := filepath.Join(dir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	content := "prefix=/usr\nLibs:  -L/abs/path  -lfoo\n"
	if err := os.WriteFile(filepath.Join(pkgDir, "test.pc"), []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if err := FixupPkgConfigFile(dir); err != nil {
		t.Fatal(err)
	}

	result := readFile(t, filepath.Join(pkgDir, "test.pc"))
	if !strings.Contains(result, "-L${libdir}") {
		t.Fatalf("double-spaced -L should be normalized to -L${libdir}, got:\n%s", result)
	}
}

func readFile(t *testing.T, path string) string {
	t.Helper()
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	return string(data)
}
