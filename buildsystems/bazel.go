package buildsystems

import (
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func NewBazel(config *BuildConfig) *bazel {
	return &bazel{BuildConfig: config}
}

type bazel struct {
	*BuildConfig
}

func (bazel) Name() string {
	return "bazel"
}

func (b bazel) CheckTools() []string {
	tools := slices.Clone(b.BuildConfig.BuildTools)
	tools = append(tools, "cmake", "bazel")
	return tools
}

func (b bazel) configured() bool {
	return false
}

func (b bazel) configureOptions() ([]string, error) {
	return []string{}, nil
}

// setupToolchainEnvs wires the cross-compiler into the process environment so
// that Bazel's @local_config_cc probes it (instead of the host gcc).
func (b bazel) setupToolchainEnvs() {
	toolchain := b.Ctx.Platform().GetToolchain()

	if b.DevDep || b.HostDev {
		// Host-side dev builds use the native compiler.
		if name := toolchain.GetName(); name != "msvc" && name != "clang-cl" {
			toolchain.ClearEnvs()
		}
		return
	}

	// Bare cross-compiler for @local_config_cc probing.
	if cc := toolchain.GetCC(); cc != "" {
		b.envBackup.setenv("CC", cc)
	}
	if cxx := toolchain.GetCXX(); cxx != "" {
		b.envBackup.setenv("CXX", cxx)
	}

	// Bazel's archiver must support @response files. The gcc-ar LTO wrapper
	// (e.g., aarch64-none-linux-gnu-gcc-ar) does not, so substitute plain ar
	// when the configured ar is gcc- prefixed.
	ar := toolchain.GetAR()
	if strings.Contains(ar, "gcc-") {
		if prefix := toolchain.GetCrosstoolPrefix(); prefix != "" {
			ar = prefix + "ar"
		}
	}
	if ar != "" {
		b.envBackup.setenv("AR", ar)
	}

	// Other binutils for link actions (local spawn strategy respects these
	// process env vars).
	for _, e := range [][2]string{
		{"LD", toolchain.GetLD()},
		{"NM", toolchain.GetNM()},
		{"STRIP", toolchain.GetSTRIP()},
	} {
		if e[1] != "" {
			b.envBackup.setenv(e[0], e[1])
		}
	}
}

func (b bazel) Configure(options []string) error {
	b.setupToolchainEnvs()

	// Ensure BuildDir exists: Bazel's output_base lives under it (see
	// generateBazelrc) and Bazel needs its parent directory to exist.
	if err := os.MkdirAll(b.PortConfig.BuildDir, os.ModePerm); err != nil {
		return fmt.Errorf("create build dir for bazel -> %w", err)
	}

	// Generate .bazelrc with cross-compilation flags.
	if _, err := b.generateBazelrc(); err != nil {
		return fmt.Errorf("generate .bazelrc for bazel -> %w", err)
	}

	// Execute custom configure commands if defined in port.toml.
	if len(b.CustomConfigure) > 0 {
		scripts := strings.Join(b.CustomConfigure, " && ")
		scripts = b.expandVariables(scripts)
		title := fmt.Sprintf("[configure %s]", b.PortConfig.nameVersion())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(b.getLogPath("configure"))
		executor.SetWorkDir(b.PortConfig.SrcDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b bazel) buildOptions() ([]string, error) {
	// Pass port.toml options through as bazel flags.
	var options []string
	for _, opt := range b.Options {
		opt = b.expandVariables(opt)
		options = append(options, opt)
	}
	return options, nil
}

func (b bazel) Build(options []string) error {
	b.setupToolchainEnvs()

	// Execute custom build commands if port.toml has them.
	if len(b.CustomBuild) > 0 {
		scripts := strings.Join(b.CustomBuild, " && ")
		scripts = b.expandVariables(scripts)
		title := fmt.Sprintf("[build %s]", b.PortConfig.nameVersion())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(b.getLogPath("build"))
		executor.SetWorkDir(b.PortConfig.SrcDir)
		if err := executor.Execute(); err != nil {
			return err
		}
		return nil
	}

	// Default: bazel build <targets> <flags>.
	// Options that don't start with "-" are treated as build targets
	// (e.g. "//:skcms_public"); the rest are flags. With no targets, build //...
	var targets, flags []string
	for _, opt := range options {
		if strings.HasPrefix(opt, "-") {
			flags = append(flags, opt)
		} else {
			targets = append(targets, opt)
		}
	}
	if len(targets) == 0 {
		targets = []string{"//..."}
	}
	command := fmt.Sprintf("bazel build %s %s --jobs=%d",
		strings.Join(targets, " "), strings.Join(flags, " "), b.PortConfig.Jobs)

	title := fmt.Sprintf("[build %s]", b.PortConfig.nameVersion())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(b.getLogPath("build"))
	executor.SetWorkDir(b.PortConfig.SrcDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (b bazel) installOptions() ([]string, error) {
	return []string{}, nil
}

func (b bazel) Install(options []string) error {
	b.setupToolchainEnvs()

	// Custom install commands in port.toml take full control.
	if len(b.CustomInstall) > 0 {
		scripts := strings.Join(b.CustomInstall, " && ")
		scripts = b.expandVariables(scripts)
		title := fmt.Sprintf("[install %s]", b.PortConfig.nameVersion())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(b.getLogPath("install"))
		executor.SetWorkDir(b.PortConfig.SrcDir)
		if err := executor.Execute(); err != nil {
			return err
		}
		return nil
	}

	// Default install: copy outputs from bazel-bin/ to PACKAGE_DIR.
	if err := os.MkdirAll(b.PortConfig.PackageDir, os.ModePerm); err != nil {
		return err
	}

	// bazel-bin/ is a symlink to bazel-out/<config>/bin/. Resolve it so the
	// walk reads real files regardless of which compilation config was used.
	bazelBin := filepath.Join(b.PortConfig.SrcDir, "bazel-bin")
	if resolved, err := filepath.EvalSymlinks(bazelBin); err == nil && resolved != "" {
		bazelBin = resolved
	}
	if !fileio.PathExists(bazelBin) {
		// Fall back to scanning bazel-out for any config's bin/.
		bazelOutDir := filepath.Join(b.PortConfig.SrcDir, "bazel-out")
		if entries, err := os.ReadDir(bazelOutDir); err == nil {
			for _, entry := range entries {
				if !entry.IsDir() {
					continue
				}
				candidate := filepath.Join(bazelOutDir, entry.Name(), "bin")
				if fileio.PathExists(candidate) {
					bazelBin = candidate
					break
				}
			}
		}
	}

	if !fileio.PathExists(bazelBin) {
		return fmt.Errorf("bazel-bin not found in %s", b.PortConfig.SrcDir)
	}

	// Walk bazel-bin/ and copy relevant outputs.
	err := filepath.WalkDir(bazelBin, func(path string, d os.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if path == bazelBin {
			return nil
		}

		baseName := filepath.Base(path)

		// Skip Bazel internal artifacts.
		if d.IsDir() {
			if baseName == "_objs" || strings.HasPrefix(baseName, "_solib") || strings.HasPrefix(baseName, "_") {
				return filepath.SkipDir
			}
			if strings.HasSuffix(baseName, ".runfiles") {
				return filepath.SkipDir
			}
		}

		relPath, err := filepath.Rel(bazelBin, path)
		if err != nil {
			return err
		}
		destPath := filepath.Join(b.PortConfig.PackageDir, relPath)

		if d.IsDir() {
			return os.MkdirAll(destPath, os.ModePerm)
		}

		// Skip generated build files (not actual artifacts).
		ext := filepath.Ext(baseName)
		if ext == ".params" || ext == ".cmd" || ext == ".scan" || ext == ".d" || ext == ".o" {
			return nil
		}

		return fileio.CopyFile(path, destPath)
	})
	if err != nil {
		return fmt.Errorf("failed to copy bazel outputs -> %w", err)
	}

	// Bazel has no install step -- headers stay in the source tree.
	// Copy public headers (.h files) from SrcDir to PACKAGE_DIR/include/.
	includeDest := filepath.Join(b.PortConfig.PackageDir, "include")
	if !fileio.PathExists(includeDest) {
		if err := os.MkdirAll(includeDest, os.ModePerm); err != nil {
			return err
		}
	}
	err = filepath.WalkDir(b.PortConfig.SrcDir, func(path string, d os.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}

		if path == b.PortConfig.SrcDir {
			return nil
		}

		// Skip Bazel convenience symlinks, VCS dirs, and internal source dirs.
		base := filepath.Base(path)
		if d.IsDir() {
			if base == ".git" ||
				base == "bazel" ||
				strings.HasPrefix(base, "bazel-") ||
				base == "src" ||
				base == "fuzz" ||
				base == "infra" {
				return filepath.SkipDir
			}
			return nil
		}

		if filepath.Ext(base) != ".h" {
			return nil
		}

		relPath, err := filepath.Rel(b.PortConfig.SrcDir, path)
		if err != nil {
			return err
		}

		destPath := filepath.Join(includeDest, relPath)
		if err := os.MkdirAll(filepath.Dir(destPath), os.ModePerm); err != nil {
			return err
		}
		return fileio.CopyFile(path, destPath)
	})
	if err != nil {
		return fmt.Errorf("failed to copy headers -> %w", err)
	}

	// Organize outputs: libraries into lib/, executables into bin/.
	libDir := filepath.Join(b.PortConfig.PackageDir, "lib")
	binDir := filepath.Join(b.PortConfig.PackageDir, "bin")
	if entries, err := os.ReadDir(b.PortConfig.PackageDir); err == nil {
		for _, entry := range entries {
			if entry.IsDir() {
				continue
			}

			name := entry.Name()
			ext := filepath.Ext(name)
			src := filepath.Join(b.PortConfig.PackageDir, name)
			if ext == ".a" || ext == ".so" || strings.Contains(name, ".so.") {
				if err := os.MkdirAll(libDir, os.ModePerm); err != nil {
					return err
				}
				if err := os.Rename(src, filepath.Join(libDir, name)); err != nil {
					return err
				}
			} else if fileio.IsELFFile(src) {
				if err := os.MkdirAll(binDir, os.ModePerm); err != nil {
					return err
				}
				if err := os.Rename(src, filepath.Join(binDir, name)); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

// generateBazelrc generates a .bazelrc file in the source directory with
// cross-compilation flags for Bazel.
func (b bazel) generateBazelrc() (string, error) {
	toolchain := b.Ctx.Platform().GetToolchain()
	rootfs := b.Ctx.Platform().GetRootFS()

	var buffer bytes.Buffer
	fmt.Fprintf(&buffer, "# Generated by celer for %s\n", b.PortConfig.nameVersion())

	// Isolate ALL Bazel state (execroot, external deps, build outputs) under
	// the celer BuildDir instead of the global ~/.cache/bazel, so everything
	// build-generated stays inside the workspace.
	outputBase := filepath.ToSlash(filepath.Join(b.PortConfig.BuildDir, "output_base"))
	fmt.Fprintf(&buffer, "startup --output_base=%s\n", outputBase)

	// Cross-compilation mode (not for dev dependencies).
	if !b.DevDep && !b.HostDev {
		fmt.Fprintf(&buffer, "build --spawn_strategy=local\n")

		if prefix := toolchain.GetCrosstoolPrefix(); prefix != "" {
			// Use the platform configured ar for @local_config_cc probing.
			// For gcc platforms this is gcc-ar (LTO wrapper), which Bazel
			// cannot use -- substitute the plain ar from the same toolchain.
			ar := toolchain.GetAR()
			if strings.Contains(ar, "gcc-") {
				ar = prefix + "ar"
			}
			fmt.Fprintf(&buffer, "build --repo_env=AR=%s\n", ar)
		}

		// Runtime flags (e.g., clang --gcc-toolchain, --rtlib). CC already
		// carries them for probing/compiling; mirror onto linkopts too.
		for _, flag := range toolchain.RuntimeFlags() {
			fmt.Fprintf(&buffer, "build --copt=%s\n", flag)
			fmt.Fprintf(&buffer, "build --cxxopt=%s\n", flag)
			fmt.Fprintf(&buffer, "build --linkopt=%s\n", flag)
		}

		// Sysroot: the compiler gets --sysroot via the CC env var (set by
		// toolchain.SetEnvs), so its builtin includes resolve to the rootfs.
		// Only the linker needs --sysroot repeated here.
		if rootfs != nil {
			sysrootDir := filepath.ToSlash(rootfs.GetAbsDir())
			fmt.Fprintf(&buffer, "build --linkopt=--sysroot=%s\n", sysrootDir)

			// Sysroot lib dirs.
			for _, libDir := range rootfs.GetLibDirs() {
				libPath := filepath.ToSlash(filepath.Join(sysrootDir, libDir))
				fmt.Fprintf(&buffer, "build --linkopt=-L%s\n", libPath)
				fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath-link,%s\n", libPath)
			}
		}

		// Dependency include/lib dirs (from tmp/deps, prepended to sysroot).
		depDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryDir)
		depInclude := filepath.ToSlash(filepath.Join(depDir, "include"))
		depLib := filepath.ToSlash(filepath.Join(depDir, "lib"))

		if fileio.PathExists(depInclude) {
			fmt.Fprintf(&buffer, "build --copt=-I%s\n", depInclude)
			fmt.Fprintf(&buffer, "build --cxxopt=-I%s\n", depInclude)
		}
		if fileio.PathExists(depLib) {
			fmt.Fprintf(&buffer, "build --linkopt=-L%s\n", depLib)
			fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath-link,%s\n", depLib)
		}

		// RPATH for runtime library resolution.
		fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath=\\$$ORIGIN/../lib\n")
		fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath=\\$$ORIGIN/../lib64\n")
	}

	// Include dirs from port.toml.
	for _, incDir := range b.IncludeDirs {
		incDir = b.ExprVars.Expand(incDir)
		incDir = filepath.ToSlash(incDir)
		fmt.Fprintf(&buffer, "build --copt=-I%s\n", incDir)
		fmt.Fprintf(&buffer, "build --cxxopt=-I%s\n", incDir)
	}

	// Lib dirs from port.toml.
	for _, libDir := range b.LibDirs {
		libDir = b.ExprVars.Expand(libDir)
		libDir = filepath.ToSlash(libDir)
		fmt.Fprintf(&buffer, "build --linkopt=-L%s\n", libDir)
		fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath-link,%s\n", libDir)
	}

	// C/C++ standard from port.toml.
	if cstd := b.CStandard; cstd != "" {
		fmt.Fprintf(&buffer, "build --copt=-std=%s\n", cstd)
	}
	if cxxstd := b.CXXStandard; cxxstd != "" {
		fmt.Fprintf(&buffer, "build --cxxopt=-std=%s\n", cxxstd)
	}

	// CFLAGS/CXXFLAGS/LDFLAGS from port.toml envs - convert to bazel flags.
	for _, env := range b.Envs {
		before, after, ok := strings.Cut(env, "=")
		if !ok {
			continue
		}

		flagStr := strings.TrimSpace(b.expandVariables(after))
		switch strings.TrimSpace(before) {
		case "CFLAGS":
			for part := range strings.FieldsSeq(flagStr) {
				fmt.Fprintf(&buffer, "build --copt=%s\n", part)
			}
		case "CXXFLAGS":
			for part := range strings.FieldsSeq(flagStr) {
				fmt.Fprintf(&buffer, "build --cxxopt=%s\n", part)
			}
		case "LDFLAGS":
			for part := range strings.FieldsSeq(flagStr) {
				fmt.Fprintf(&buffer, "build --linkopt=%s\n", part)
			}
		}
	}

	// Write .bazelrc to source directory (Bazel reads .bazelrc from workspace root).
	bazelrcPath := filepath.Join(b.PortConfig.SrcDir, ".bazelrc")
	if err := os.WriteFile(bazelrcPath, buffer.Bytes(), os.ModePerm); err != nil {
		return "", err
	}

	return bazelrcPath, nil
}
