package pc

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestPkgConfigApply_RewritesPrefixAndToolVars(t *testing.T) {
	packageDir := t.TempDir()
	pkgConfigDir := filepath.Join(packageDir, "lib", "pkgconfig")
	if err := os.MkdirAll(pkgConfigDir, os.ModePerm); err != nil {
		t.Fatalf("mkdir pkgconfig dir: %v", err)
	}

	pkgPath := filepath.Join(pkgConfigDir, "sample.pc")
	content := strings.Join([]string{
		"prefix=/old/prefix",
		"exec_prefix=/old/prefix",
		"libdir=/old/prefix/lib",
		"includedir=/old/prefix/include",
		"g_ir_scanner=/old/tools/g-ir-scanner",
		"g_ir_compiler=/old/tools/g-ir-compiler",
		"Name: sample",
		"Libs: -L/old/prefix/lib -lsample",
	}, "\n") + "\n"
	if err := os.WriteFile(pkgPath, []byte(content), 0o644); err != nil {
		t.Fatalf("write pkg-config: %v", err)
	}

	sysrootDir := "/opt/sysroot"
	toolBinDir := "/tmp/deps/x86_64-linux-dev/bin"
	relativeScanner, err := filepath.Rel(sysrootDir, filepath.Join(toolBinDir, "g-ir-scanner"))
	if err != nil {
		t.Fatalf("rel scanner path: %v", err)
	}
	relativeCompiler, err := filepath.Rel(sysrootDir, filepath.Join(toolBinDir, "g-ir-compiler"))
	if err != nil {
		t.Fatalf("rel compiler path: %v", err)
	}

	pkgConfig := PkgConfig{
		ToolBinDir:     toolBinDir,
		SysrootDir:     sysrootDir,
		PkgConfigTools: []string{"g_ir_scanner", "g_ir_compiler"},
	}
	if err = pkgConfig.Apply(packageDir, "/tmp/deps/aarch64-linux"); err != nil {
		t.Fatalf("apply pkg-config: %v", err)
	}

	data, err := os.ReadFile(pkgPath)
	if err != nil {
		t.Fatalf("read pkg-config: %v", err)
	}

	text := string(data)
	for _, want := range []string{
		"prefix=/tmp/deps/aarch64-linux",
		"exec_prefix=${prefix}",
		"libdir=${prefix}/lib",
		"includedir=${prefix}/include",
		"g_ir_scanner=${pc_sysrootdir}/" + filepath.ToSlash(relativeScanner),
		"g_ir_compiler=${pc_sysrootdir}/" + filepath.ToSlash(relativeCompiler),
		"Libs: -L${libdir} -lsample",
	} {
		if !strings.Contains(text, want) {
			t.Fatalf("expected %q in pkg-config, got:\n%s", want, text)
		}
	}
}

func TestPkgConfigApply_RequiresToolBinDirForToolRewrite(t *testing.T) {
	pkgConfig := PkgConfig{
		PkgConfigTools: []string{"g_ir_scanner"},
	}
	if err := pkgConfig.Apply(t.TempDir(), "/tmp/deps/aarch64-linux"); err == nil {
		t.Fatal("expected missing ToolBinDir to fail")
	}
}
