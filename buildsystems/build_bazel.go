package buildsystems

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
	"time"

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
func (b bazel) setupToolchainEnvs() error {
	toolchain := b.Ctx.Platform().GetToolchain()

	if b.DevDep || b.HostDev {
		// Host-side dev builds use the native compiler.
		if name := toolchain.GetName(); name != "msvc" && name != "clang-cl" {
			toolchain.ClearEnvs()
		}
		return nil
	}

	// On Windows, tell Bazel where the VC++ tools are and override the
	// Bazel version. Bazel's @local_config_cc may fail to detect tools on
	// VS releases newer than the project's pinned .bazelversion (e.g.
	// Bazel 5.x cannot find VC++ in VS 2026). USE_BAZEL_VERSION forces
	// bazelisk to download a version that understands the installed VS.
	if msvc := toolchain.GetMSVC(); msvc != nil && msvc.VCVars != "" {
		// VCVars is e.g. ".../VC/Auxiliary/Build/vcvarsall.bat";
		// BAZEL_VC expects the VC directory (3 levels up).
		vcDir := filepath.Dir(filepath.Dir(filepath.Dir(msvc.VCVars)))
		b.envBackup.setenv("BAZEL_VC", vcDir)

		// Use a Bazel version that supports this VS release.
		b.envBackup.setenv("USE_BAZEL_VERSION", "6.4.0")
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

	return nil
}

func (b bazel) Configure(options []string) error {
	if err := b.setupToolchainEnvs(); err != nil {
		return err
	}

	// Ensure BuildDir exists: Bazel's output_base lives under it (see
	// generateBazelrc) and Bazel needs its parent directory to exist.
	if err := os.MkdirAll(b.PortConfig.BuildDir, os.ModePerm); err != nil {
		return fmt.Errorf("create build dir for bazel -> %w", err)
	}

	// Generate a cc_toolchain (crosstool) for cross-compilation, replacing
	// @local_config_cc. Returns "" when not applicable (host build / no rootfs /
	// non-gcc/clang), in which case Bazel falls back to its default detection.
	ccToolchainDir, err := b.generateCCToolchain()
	if err != nil {
		return fmt.Errorf("generate cc_toolchain for bazel -> %w", err)
	}

	// Generate .bazelrc with cross-compilation flags.
	if _, err := b.generateBazelrc(ccToolchainDir); err != nil {
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
	if err := b.setupToolchainEnvs(); err != nil {
		return err
	}

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

func (b bazel) shutdownServer() {
	title := fmt.Sprintf("[bazel shutdown %s]", b.PortConfig.nameVersion())
	shutdown := cmd.NewExecutor(title, "bazel", "shutdown")
	shutdown.SetWorkDir(b.PortConfig.SrcDir)
	_ = shutdown.Execute()

	// Give the server time to flush and release file handles.
	time.Sleep(2 * time.Second)
}

func (b bazel) installOptions() ([]string, error) {
	return []string{}, nil
}

func (b bazel) Install(options []string) error {
	if err := b.setupToolchainEnvs(); err != nil {
		return err
	}

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
		b.shutdownServer()
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

	b.shutdownServer()
	return nil
}

// generateBazelrc generates a .bazelrc file in the source directory with
// cross-compilation flags for Bazel.
func (b bazel) generateBazelrc(ccToolchainDir string) (string, error) {
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

		// Bazel 6.x removed @bazel_tools//platforms in favor of
		// @platforms. Some projects (e.g. skcms via rules_docker)
		// still reference the old location. When we override the
		// Bazel version via USE_BAZEL_VERSION, keep compatibility.
		fmt.Fprintf(&buffer, "build --incompatible_use_platforms_repo_for_constraints=false\n")

		// Register celer's cc_toolchain (crosstool) for cross-compilation,
		// replacing @local_config_cc (which is host-oriented: it injects the
		// host binutils and misses the sysroot includes). Disabled when no
		// crosstool was generated (no rootfs / non-gcc/clang), in which case
		// Bazel falls back to its default host toolchain detection.
		if ccToolchainDir != "" {
			fmt.Fprintf(&buffer, "build --incompatible_enable_cc_toolchain_resolution\n")
			fmt.Fprintf(&buffer, "build --action_env=BAZEL_DO_NOT_DETECT_CPP_TOOLCHAIN=1\n")
			fmt.Fprintf(&buffer, "build --override_repository=cc_toolchain=%s\n", ccToolchainDir)
			fmt.Fprintf(&buffer, "build --extra_toolchains=@cc_toolchain//:cc_toolchain_impl\n")
			fmt.Fprintf(&buffer, "build --platforms=@cc_toolchain//:target_platform\n")
		}

		// Runtime flags (e.g., clang --gcc-toolchain, --rtlib) applied to
		// compile and link actions.
		for _, flag := range toolchain.RuntimeFlags() {
			fmt.Fprintf(&buffer, "build --copt=%s\n", flag)
			fmt.Fprintf(&buffer, "build --cxxopt=%s\n", flag)
			fmt.Fprintf(&buffer, "build --linkopt=%s\n", flag)
		}

		// Sysroot lib dirs (link paths). The sysroot itself (headers + the
		// --sysroot flag) is declared in the crosstool via builtin_sysroot; here
		// we only add the lib search paths for the linker.
		if rootfs != nil {
			sysrootDir := filepath.ToSlash(rootfs.GetAbsDir())
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

// generateCCToolchain generates a minimal Bazel cc_toolchain (crosstool) that
// cross-compiles with celer's toolchain + rootfs, replacing @local_config_cc
// (which is host-oriented and injects the host binutils / misses the sysroot
// includes).
func (b bazel) generateCCToolchain() (string, error) {
	toolchain := b.Ctx.Platform().GetToolchain()
	rootfs := b.Ctx.Platform().GetRootFS()
	if rootfs == nil {
		return "", nil
	}
	switch toolchain.GetName() {
	case "gcc", "clang":
	default:
		return "", nil
	}

	binDir := filepath.ToSlash(toolchain.GetAbsDir())
	sysrootDir := filepath.ToSlash(rootfs.GetAbsDir())
	ccPath := filepath.ToSlash(filepath.Join(binDir, toolchain.GetCC()))
	includes := b.probeGccIncludes(ccPath, sysrootDir, toolchain.RuntimeFlags())
	if len(includes) == 0 {
		return "", fmt.Errorf("probe cross-gcc builtin includes for bazel crosstool failed: %s", ccPath)
	}

	cpu := toolchain.GetSystemProcessor()
	targetSystem := strings.ToLower(toolchain.GetSystemName())
	execCpu := b.bazelCpu(runtime.GOARCH)

	// Plain ar (gcc-ar LTO wrapper cannot handle @response files).
	ar := toolchain.GetAR()
	if strings.Contains(ar, "gcc-") {
		if prefix := toolchain.GetCrosstoolPrefix(); prefix != "" {
			ar = prefix + "ar"
		}
	}

	// Resolve binutils. Cross toolchains (e.g. aarch64) ship their own; some
	// toolchains that target the host arch (e.g. x86_64-linux-gnu on an x86_64
	// host) omit them and rely on the host /usr/bin binutils. Fall back to the
	// host tool when the cross one is absent (correct because target==host arch
	// in that case); /bin/false if neither exists.
	ldPath := b.resolveBinutil(binDir, toolchain.GetLD(), "ld")
	arPath := b.resolveBinutil(binDir, ar, "ar")
	nmPath := b.resolveBinutil(binDir, toolchain.GetNM(), "nm")
	stripPath := b.resolveBinutil(binDir, toolchain.GetSTRIP(), "strip")
	objdumpPath := b.resolveBinutil(binDir, toolchain.GetOBJDUMP(), "objdump")

	// cpp / gcov / dwp: fall back to the C compiler (gcc can preprocess / drive
	// gcov); these are rarely invoked directly.
	cppPath := ccPath
	if cpp := toolchain.GetCPP(); cpp != "" {
		if p := filepath.Join(binDir, cpp); b.isFile(p) {
			cppPath = filepath.ToSlash(p)
		}
	}
	gcovPath := ccPath
	if gcov := toolchain.GetGCOV(); gcov != "" {
		if p := filepath.Join(binDir, gcov); b.isFile(p) {
			gcovPath = filepath.ToSlash(p)
		}
	}
	dwpPath := ccPath

	var includesBuf strings.Builder
	for _, inc := range includes {
		fmt.Fprintf(&includesBuf, "    %q,\n", inc)
	}

	configBzl := fmt.Sprintf(ccToolchainConfigBzlTemplate,
		sysrootDir, cpu, targetSystem, includesBuf.String(),
		ccPath,
		filepath.ToSlash(filepath.Join(binDir, toolchain.GetCXX())),
		cppPath, gcovPath,
		ldPath, ldPath,
		arPath,
		nmPath, stripPath,
		objdumpPath,
		dwpPath,
	)

	buildBazel := fmt.Sprintf(ccToolchainBuildTemplate, execCpu, cpu, cpu)

	repoDir := filepath.ToSlash(filepath.Join(b.PortConfig.BuildDir, "cc_toolchain"))
	if err := os.MkdirAll(repoDir, os.ModePerm); err != nil {
		return "", fmt.Errorf("create cc_toolchain repo dir for bazel -> %w", err)
	}
	if err := os.WriteFile(filepath.Join(repoDir, "WORKSPACE.bazel"), []byte("# cc_toolchain repository\n"), os.ModePerm); err != nil {
		return "", err
	}
	if err := os.WriteFile(filepath.Join(repoDir, "BUILD.bazel"), []byte(buildBazel), os.ModePerm); err != nil {
		return "", err
	}
	if err := os.WriteFile(filepath.Join(repoDir, "cc_toolchain_config.bzl"), []byte(configBzl), os.ModePerm); err != nil {
		return "", err
	}
	return repoDir, nil
}

// bazelCpu maps a Go runtime arch to a @platforms//cpu value.
func (b bazel) bazelCpu(goarch string) string {
	switch goarch {
	case "amd64":
		return "x86_64"
	case "arm64":
		return "aarch64"
	case "386":
		return "x86_32"
	default:
		return goarch
	}
}

// isFile reports whether path exists and is not a directory.
func (b bazel) isFile(path string) bool {
	info, err := os.Stat(path)
	return err == nil && !info.IsDir()
}

// resolveBinutil resolves a binutils tool path: prefer the cross-toolchain's
// own binary under binDir, fall back to the host's (correct when the target
// arch equals the host arch, which is exactly when cross toolchains tend to
// omit these tools), else "/bin/false".
func (b bazel) resolveBinutil(binDir, tomlName, base string) string {
	if tomlName != "" {
		if p := filepath.Join(binDir, tomlName); b.isFile(p) {
			return filepath.ToSlash(p)
		}
	}
	if host, err := exec.LookPath(base); err == nil {
		return filepath.ToSlash(host)
	}
	return "/bin/false"
}

// probeGccIncludes gets the compiler's builtin include dirs via `-E -v`.
// runtimeFlags must match the real build so the probed dirs stay in sync.
func (b bazel) probeGccIncludes(gccPath, sysroot string, runtimeFlags []string) []string {
	args := []string{"--sysroot=" + sysroot}
	args = append(args, runtimeFlags...)
	args = append(args, "-E", "-v", "-xc++", "-")
	cmd := exec.Command(gccPath, args...)
	cmd.Stdin = strings.NewReader("")
	var stderr bytes.Buffer
	cmd.Stderr = &stderr
	cmd.Stdout = io.Discard
	if err := cmd.Run(); err != nil {
		return nil
	}
	return b.parseGccIncludeDirs(stderr.String())
}

func (b bazel) parseGccIncludeDirs(output string) []string {
	var dirs []string
	inBlock := false
	for line := range strings.SplitSeq(output, "\n") {
		switch {
		case strings.Contains(line, "search starts here"):
			inBlock = true
		case strings.Contains(line, "End of search list"):
			inBlock = false
		case inBlock:
			trimmed := strings.TrimSpace(line)
			if trimmed == "" || strings.HasPrefix(trimmed, "(") {
				continue
			}

			// Strip trailing "(framework directory)" etc.
			if idx := strings.Index(trimmed, " ("); idx > 0 {
				trimmed = trimmed[:idx]
			}

			// Normalize (e.g. collapse the ../../../../ that clang emits for
			// --gcc-toolchain paths) so the declared dir matches the form Bazel
			// reports in "undeclared inclusion(s)" errors.
			if cleaned := filepath.Clean(trimmed); cleaned != "" {
				trimmed = filepath.ToSlash(cleaned)
			}
			dirs = append(dirs, trimmed)
		}
	}
	return dirs
}

const ccToolchainConfigBzlTemplate = `load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl", "feature", "flag_set", "flag_group", "tool_path")
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

SYSROOT = %q
CPU = %q
TARGET_SYSTEM = %q

BUILTIN_INCLUDES = [
%s]

TOOL_PATHS = {
    "gcc": %q,
    "g++": %q,
    "cpp": %q,
    "gcov": %q,
    "ld": %q,
    "compatibility_ld": %q,
    "ar": %q,
    "nm": %q,
    "strip": %q,
    "objdump": %q,
    "dwp": %q,
}

_COMPILE_ACTIONS = [
    ACTION_NAMES.assemble,
    ACTION_NAMES.preprocess_assemble,
    ACTION_NAMES.c_compile,
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.linkstamp_compile,
]

_LINK_ACTIONS = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

def _impl(ctx):
    features = [
        feature(
            name = "default_compile_flags",
            enabled = True,
            flag_sets = [flag_set(
                actions = _COMPILE_ACTIONS,
                flag_groups = [flag_group(flags = ["--sysroot=" + SYSROOT])],
            )],
        ),
        feature(
            name = "default_link_flags",
            enabled = True,
            flag_sets = [flag_set(
                actions = _LINK_ACTIONS,
                flag_groups = [flag_group(flags = ["--sysroot=" + SYSROOT])],
            )],
        ),
        feature(
            name = "dependency_file",
            enabled = True,
            flag_sets = [flag_set(
                actions = _COMPILE_ACTIONS,
                flag_groups = [flag_group(flags = ["-MD", "-MF", "%%{dependency_file}"])],
            )],
        ),
        feature(
            name = "random_seed",
            enabled = True,
            flag_sets = [flag_set(
                actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
                flag_groups = [flag_group(flags = ["-frandom-seed=%%{output_file}"])],
            )],
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = CPU,
        host_system_name = "local",
        target_system_name = TARGET_SYSTEM,
        target_cpu = CPU,
        target_libc = "glibc",
        compiler = "gcc",
        abi_version = "local",
        abi_libc_version = "local",
        tool_paths = [tool_path(name = k, path = v) for k, v in TOOL_PATHS.items()],
        cxx_builtin_include_directories = BUILTIN_INCLUDES,
        builtin_sysroot = SYSROOT,
        features = features,
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
`

const ccToolchainBuildTemplate = `load(":cc_toolchain_config.bzl", "cc_toolchain_config")

package(default_visibility = ["//visibility:public"])

filegroup(name = "empty")

cc_toolchain_config(name = "cc_toolchain_config")

cc_toolchain(
    name = "cc_toolchain",
    toolchain_config = ":cc_toolchain_config",
    all_files = ":empty",
    ar_files = ":empty",
    as_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 0,
)

toolchain(
    name = "cc_toolchain_impl",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:%s",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:%s",
    ],
    toolchain = ":cc_toolchain",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

platform(
    name = "target_platform",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:%s",
    ],
)
`
