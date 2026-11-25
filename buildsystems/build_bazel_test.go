package buildsystems

import (
	"bytes"
	"os"
	"path/filepath"
	"slices"
	"testing"
)

func TestBazelVersionAtLeast(t *testing.T) {
	cases := []struct {
		pinned string
		want   bool
	}{
		{"6.4.0", true},
		{"6.4", true},
		{"6.4.0rc1", true},
		{"6.5.0", true},
		{"7.1.0", true},
		{"5.4.1", false},
		{"6.3.2", false},
		{"6.3", false},
		{"latest", true},
		{"last_green", true},
		{"", false},
	}
	for _, tc := range cases {
		if got := bazelVersionAtLeast(tc.pinned, minBazelVersionForMSVC); got != tc.want {
			t.Errorf("bazelVersionAtLeast(%q, %q) = %v, want %v", tc.pinned, minBazelVersionForMSVC, got, tc.want)
		}
	}
}

func TestEnsureBazelVersionForMSVC(t *testing.T) {
	cases := []struct {
		name       string
		bazelVer   string
		wantEnv    string
		wantSetEnv bool
	}{
		{"missing_pin", "", "6.4.0", true},
		{"old_pin", "5.4.1", "6.4.0", true},
		{"old_pin_6_3", "6.3.2", "6.4.0", true},
		{"new_pin_kept", "6.4.0", "", false},
		{"newer_pin_kept", "7.1.0", "", false},
		{"non_numeric_pin_kept", "latest", "", false},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Setenv("USE_BAZEL_VERSION", "")

			srcDir := t.TempDir()
			if tc.bazelVer != "" {
				if err := os.WriteFile(filepath.Join(srcDir, ".bazelversion"), []byte(tc.bazelVer+"\n"), 0o644); err != nil {
					t.Fatal(err)
				}
			}

			b := bazel{BuildConfig: &BuildConfig{
				envBackup:  envsBackup{modifiedEnvs: make(map[string]bool)},
				PortConfig: PortConfig{SrcDir: srcDir},
			}}
			b.ensureBazelVersionForMSVC()

			got := os.Getenv("USE_BAZEL_VERSION")
			if got != tc.wantEnv {
				t.Errorf("USE_BAZEL_VERSION = %q, want %q", got, tc.wantEnv)
			}
			if _, ok := b.envBackup.modifiedEnvs["USE_BAZEL_VERSION"]; ok != tc.wantSetEnv {
				t.Errorf("modifiedEnvs[USE_BAZEL_VERSION] = %v, want %v", ok, tc.wantSetEnv)
			}
		})
	}
}

func TestProjectBazelVersion(t *testing.T) {
	srcDir := t.TempDir()
	b := bazel{BuildConfig: &BuildConfig{PortConfig: PortConfig{SrcDir: srcDir}}}

	if got := b.projectBazelVersion(); got != "" {
		t.Errorf("projectBazelVersion without file = %q, want empty", got)
	}

	content := "\n  6.4.0  \n8f9c0f0e0d1c2b3a4f5e6d7c8b9a0f1e2d3c4b5a6f7e8d9c0b1a2f3e4d5c6b7a\n"
	if err := os.WriteFile(filepath.Join(srcDir, ".bazelversion"), []byte(content), 0o644); err != nil {
		t.Fatal(err)
	}
	if got := b.projectBazelVersion(); got != "6.4.0" {
		t.Errorf("projectBazelVersion = %q, want %q", got, "6.4.0")
	}
}

func TestSplitBazelOptions(t *testing.T) {
	targets, flags, exclusions := splitBazelOptions([]string{
		"//:foo",
		"@repo//:bar",
		":local",
		"--compilation_mode=opt",
		"-//test/...",
		"-@legacy//...",
		"--copt=-DFOO",
	})

	wantTargets := []string{"//:foo", "@repo//:bar", ":local"}
	wantFlags := []string{"--compilation_mode=opt", "--copt=-DFOO"}
	wantExclusions := []string{"-//test/...", "-@legacy//..."}

	if !slices.Equal(targets, wantTargets) {
		t.Errorf("targets = %v, want %v", targets, wantTargets)
	}
	if !slices.Equal(flags, wantFlags) {
		t.Errorf("flags = %v, want %v", flags, wantFlags)
	}
	if !slices.Equal(exclusions, wantExclusions) {
		t.Errorf("exclusions = %v, want %v", exclusions, wantExclusions)
	}
}

func TestResolveOutput(t *testing.T) {
	buildDir := t.TempDir()
	srcDir := t.TempDir()
	execRoot := filepath.Join(buildDir, "output_base", "execroot", "ws")

	generated := filepath.Join(execRoot, "bazel-out", "k8-fastbuild", "bin", "sub", "libfoo.a")
	source := filepath.Join(srcDir, "include", "api.h")
	for _, p := range []string{generated, source} {
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(p, []byte("x"), 0o644); err != nil {
			t.Fatal(err)
		}
	}

	b := bazel{BuildConfig: &BuildConfig{PortConfig: PortConfig{SrcDir: srcDir, BuildDir: buildDir}}}
	cases := []struct {
		path    string
		wantSrc string
		wantRel string
		wantOK  bool
	}{
		{"bazel-out/k8-fastbuild/bin/sub/libfoo.a", generated, filepath.Join("sub", "libfoo.a"), true},
		{"include/api.h", source, filepath.FromSlash("include/api.h"), true},
		{"include/missing.h", "", "", false},
		{"bazel-out/k8-fastbuild/bin/missing.a", "", "", false},
		{"", "", "", false},
	}
	for _, tc := range cases {
		got, ok := b.resolveOutput(execRoot, tc.path)
		if ok != tc.wantOK || got.src != tc.wantSrc || got.rel != tc.wantRel {
			t.Errorf("resolveOutput(%q) = %+v, %v; want src=%q rel=%q ok=%v",
				tc.path, got, ok, tc.wantSrc, tc.wantRel, tc.wantOK)
		}
	}
}

func TestInstallArtifactClassifies(t *testing.T) {
	packageDir := t.TempDir()
	srcDir := t.TempDir()
	binDir := t.TempDir()

	b := bazel{BuildConfig: &BuildConfig{PortConfig: PortConfig{
		SrcDir:     srcDir,
		PackageDir: packageDir,
	}}}

	write := func(dir, name string, data []byte) string {
		t.Helper()
		p := filepath.Join(dir, name)
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(p, data, 0o755); err != nil {
			t.Fatal(err)
		}
		return p
	}

	cases := []struct {
		name string
		src  string
		rel  string
		want string
	}{
		{"static_lib", write(binDir, "libfoo.a", []byte("!<arch>\n")), "libfoo.a", filepath.Join("lib", "libfoo.a")},
		{"shared_lib", write(binDir, "libbar.so.1", []byte("\x7fELFrest")), "libbar.so.1", filepath.Join("lib", "libbar.so.1")},
		{"elf_exec", write(binDir, "myapp", []byte("\x7fELFrest")), "myapp", filepath.Join("bin", "myapp")},
		{"windows_dll", write(binDir, "lib.dll", []byte("MZ")), "lib.dll", filepath.Join("bin", "lib.dll")},
		{"import_lib", write(binDir, "foo.lib", []byte("!<arch>\n")), "foo.lib", filepath.Join("lib", "foo.lib")},
		{"header", write(srcDir, "api.h", []byte("#pragma once")), "api.h", filepath.Join("include", "api.h")},
		{"data", write(binDir, "notes.txt", []byte("x")), filepath.Join("pkg", "notes.txt"), filepath.Join("pkg", "notes.txt")},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if err := b.installArtifact(tc.src, tc.rel); err != nil {
				t.Fatalf("installArtifact: %v", err)
			}
			if _, err := os.Stat(filepath.Join(packageDir, tc.want)); err != nil {
				t.Fatalf("expected %s in package dir: %v", tc.want, err)
			}
		})
	}
}

func TestWriteFlagHelpers(t *testing.T) {
	var buffer bytes.Buffer
	writeFlags(&buffer, "-O2", "copt", "cxxopt")
	writeIncludeDir(&buffer, "/deps/include")
	writeLibDir(&buffer, "/deps/lib")

	want := "build --copt=-O2\n" +
		"build --cxxopt=-O2\n" +
		"build --copt=-I/deps/include\n" +
		"build --cxxopt=-I/deps/include\n" +
		"build --linkopt=-L/deps/lib\n" +
		"build --linkopt=-Wl,-rpath-link,/deps/lib\n"
	if got := buffer.String(); got != want {
		t.Errorf("generated rc:\n%swant:\n%s", got, want)
	}
}
