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
	"strconv"
	"strings"
	"time"

	"github.com/celer-pkg/celer/internal/cmd"
	"github.com/celer-pkg/celer/internal/fileio"
	"github.com/celer-pkg/celer/internal/logger"
)

func NewBazel(config *BuildConfig) *bazel {
	return &bazel{BuildConfig: config}
}

type bazel struct {
	*BuildConfig
}

const (
	bazelrcFileName        = "celer.bazelrc"
	minBazelVersionForMSVC = "6.4.0"
)

func (bazel) Name() string {
	return "bazel"
}

func (b bazel) CheckTools() []string {
	tools := b.defaultBuildTools()
	tools = append(tools, "bazel")
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

	// On Windows, tell Bazel where the VC++ tools are. USE_BAZEL_VERSION is
	// set separately and only when the project's own pin is missing or too
	// old (see ensureBazelVersionForMSVC).
	if msvc := toolchain.GetMSVC(); msvc != nil && msvc.VCVars != "" {
		// VCVars is e.g. ".../VC/Auxiliary/Build/vcvarsall.bat";
		// BAZEL_VC expects the VC directory (3 levels up).
		vcDir := filepath.Dir(filepath.Dir(filepath.Dir(msvc.VCVars)))
		b.envBackup.setenv("BAZEL_VC", vcDir)
		b.ensureBazelVersionForMSVC()
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

	// Other binutils for link actions (local spawn strategy respects these process env vars).
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

func (b bazel) bazelrcPath() string {
	return filepath.Join(b.PortConfig.BuildDir, bazelrcFileName)
}

// startupArgs passes celer's generated bazelrc and suppresses the project's
// workspace .bazelrc.
func (b bazel) startupArgs(args ...string) []string {
	return append([]string{
		"--noworkspace_rc",
		"--bazelrc=" + filepath.ToSlash(b.bazelrcPath()),
	}, args...)
}

func (b bazel) ensureBazelVersionForMSVC() {
	pinned := b.projectBazelVersion()
	if bazelVersionAtLeast(pinned, minBazelVersionForMSVC) {
		return
	}

	if pinned == "" {
		logger.PrintInfo("[bazel %s] no .bazelversion; using Bazel %s (USE_BAZEL_VERSION) for MSVC",
			b.PortConfig.nameVersion(), minBazelVersionForMSVC)
	} else {
		logger.PrintWarning("[bazel %s] .bazelversion %s is too old for the installed VC++ tools; using Bazel %s",
			b.PortConfig.nameVersion(), pinned, minBazelVersionForMSVC)
	}
	b.envBackup.setenv("USE_BAZEL_VERSION", minBazelVersionForMSVC)
}

func (b bazel) projectBazelVersion() string {
	data, err := os.ReadFile(filepath.Join(b.PortConfig.SrcDir, ".bazelversion"))
	if err != nil {
		return ""
	}
	for _, line := range strings.Split(string(data), "\n") {
		if line = strings.TrimSpace(line); line != "" {
			return line
		}
	}
	return ""
}

func bazelVersionAtLeast(pinned, min string) bool {
	p, ok := parseBazelVersion(pinned)
	if !ok {
		return pinned != ""
	}
	m, _ := parseBazelVersion(min)
	return slices.Compare(p[:], m[:]) >= 0
}

func parseBazelVersion(v string) (nums [3]int, ok bool) {
	fields := strings.Split(strings.TrimSpace(v), ".")
	for i := 0; i < len(fields) && i < len(nums); i++ {
		digits := 0
		for digits < len(fields[i]) && fields[i][digits] >= '0' && fields[i][digits] <= '9' {
			digits++
		}
		if digits == 0 {
			return [3]int{}, false
		}
		n, err := strconv.Atoi(fields[i][:digits])
		if err != nil {
			return [3]int{}, false
		}
		nums[i] = n
	}
	return nums, true
}

func (b bazel) Configure(options []string) error {
	if err := b.setupToolchainEnvs(); err != nil {
		return err
	}

	if err := os.MkdirAll(b.PortConfig.BuildDir, os.ModePerm); err != nil {
		return fmt.Errorf("create build dir for bazel -> %w", err)
	}

	// Generate a cc_toolchain (crosstool) for cross-compilation, replacing
	// @local_config_cc. Returns "" when not applicable (host build / no rootfs /
	// non-gcc/clang).
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
	targets, flags, exclusions := splitBazelOptions(options)
	if len(targets) == 0 {
		targets = []string{"//..."}
	}

	args := []string{"build"}
	args = append(args, targets...)
	args = append(args, flags...)
	args = append(args, exclusions...)
	args = append(args, fmt.Sprintf("--jobs=%d", b.PortConfig.Jobs))

	title := fmt.Sprintf("[build %s]", b.PortConfig.nameVersion())
	executor := cmd.NewExecutor(title, "bazel", b.startupArgs(args...)...)
	executor.SetLogPath(b.getLogPath("build"))
	executor.SetWorkDir(b.PortConfig.SrcDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (b bazel) shutdownServer() {
	title := fmt.Sprintf("[bazel shutdown %s]", b.PortConfig.nameVersion())
	shutdown := cmd.NewExecutor(title, "bazel", b.startupArgs("shutdown")...)
	shutdown.SetWorkDir(b.PortConfig.SrcDir)
	shutdown.Execute()

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

	if err := os.MkdirAll(b.PortConfig.PackageDir, os.ModePerm); err != nil {
		return err
	}
	outputs, err := b.declaredOutputs()
	if err != nil {
		return err
	}
	for _, out := range outputs {
		if err := b.installArtifact(out.src, out.rel); err != nil {
			return fmt.Errorf("failed to install %s -> %w", out.src, err)
		}
	}
	if len(outputs) == 0 {
		logger.PrintWarning("[bazel %s] cquery reported no declared outputs; installing headers only",
			b.PortConfig.nameVersion())
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

	b.shutdownServer()
	return nil
}

func splitBazelOptions(options []string) (targets, flags, exclusions []string) {
	for _, opt := range options {
		switch {
		case strings.HasPrefix(opt, "//"), strings.HasPrefix(opt, "@"), strings.HasPrefix(opt, ":"):
			targets = append(targets, opt)
		case strings.HasPrefix(opt, "-//"), strings.HasPrefix(opt, "-@"):
			exclusions = append(exclusions, opt)
		default:
			flags = append(flags, opt)
		}
	}
	return targets, flags, exclusions
}

// bazelOutput is one file declared by the built targets: src is its absolute
// path, rel the location it keeps inside the package.
type bazelOutput struct {
	src string
	rel string
}

// declaredOutputs asks Bazel which files the requested targets produce,
// resolved against this port's execroot.
func (b bazel) declaredOutputs() ([]bazelOutput, error) {
	options, err := b.buildOptions()
	if err != nil {
		return nil, err
	}
	targets, flags, exclusions := splitBazelOptions(options)
	if len(targets) == 0 {
		targets = []string{"//..."}
	}

	root := b.executionRoot()
	if root == "" {
		return nil, fmt.Errorf("bazel execroot not found in %s", b.PortConfig.BuildDir)
	}

	args := []string{"cquery", "--output=files"}
	args = append(args, flags...)
	args = append(args, targets...)
	args = append(args, exclusions...)

	title := fmt.Sprintf("[cquery %s]", b.PortConfig.nameVersion())
	executor := cmd.NewExecutor(title, "bazel", b.startupArgs(args...)...)
	executor.SetWorkDir(b.PortConfig.SrcDir)
	out, err := executor.ExecuteOutput()
	if err != nil {
		return nil, fmt.Errorf("cquery declared outputs -> %w", err)
	}

	var outputs []bazelOutput
	seen := make(map[string]struct{})
	for _, line := range strings.Split(out, "\n") {
		out, ok := b.resolveOutput(root, line)
		if !ok {
			continue
		}
		if _, dup := seen[out.src]; dup {
			continue
		}
		seen[out.src] = struct{}{}
		outputs = append(outputs, out)
	}
	return outputs, nil
}

// executionRoot locates the execroot under this port's private output_base.
// The output_base is dedicated to one workspace, so execroot has one child.
func (b bazel) executionRoot() string {
	dir := filepath.Join(b.PortConfig.BuildDir, "output_base", "execroot")
	entries, err := os.ReadDir(dir)
	if err != nil {
		return ""
	}
	for _, entry := range entries {
		if entry.IsDir() {
			return filepath.Join(dir, entry.Name())
		}
	}
	return ""
}

// resolveOutput turns a cquery path into a file to install. Generated files are
// printed as bazel-out/<config>/bin/<rel>; other files are relative to the
// workspace or, for external repos, to the execroot.
func (b bazel) resolveOutput(execRoot, path string) (bazelOutput, bool) {
	path = strings.TrimPrefix(strings.TrimSpace(path), "./")
	if path == "" {
		return bazelOutput{}, false
	}

	if parts := strings.SplitN(path, "/", 4); len(parts) == 4 && parts[0] == "bazel-out" && parts[2] == "bin" {
		src := filepath.Join(execRoot, filepath.FromSlash(path))
		if b.isFile(src) {
			return bazelOutput{src: src, rel: filepath.FromSlash(parts[3])}, true
		}
		return bazelOutput{}, false
	}

	for _, base := range []string{b.PortConfig.SrcDir, execRoot} {
		if src := filepath.Join(base, filepath.FromSlash(path)); b.isFile(src) {
			return bazelOutput{src: src, rel: filepath.FromSlash(path)}, true
		}
	}
	return bazelOutput{}, false
}

// installArtifact copies one build output into PACKAGE_DIR: libraries go to
// lib/, executables and Windows runtime files to bin/, headers to include/,
// everything else keeps its package-relative path.
func (b bazel) installArtifact(src, relPath string) error {
	var dest string
	switch kind := artifactKind(src, relPath); kind {
	case "lib", "bin":
		dest = filepath.Join(b.PortConfig.PackageDir, kind, filepath.Base(relPath))
		if filepath.Dir(relPath) != "." && fileio.PathExists(dest) {
			dest = filepath.Join(b.PortConfig.PackageDir, kind, relPath)
		}
	case "include":
		dest = filepath.Join(b.PortConfig.PackageDir, "include", relPath)
	default:
		dest = filepath.Join(b.PortConfig.PackageDir, relPath)
	}
	if err := os.MkdirAll(filepath.Dir(dest), os.ModePerm); err != nil {
		return err
	}
	return fileio.CopyFile(src, dest)
}

// artifactKind classifies a Bazel output: "lib", "bin", "include", or "" for
// files that keep their package-relative path.
func artifactKind(src, relPath string) string {
	name := strings.ToLower(filepath.Base(relPath))
	switch filepath.Ext(name) {
	case ".a", ".lib", ".dylib", ".tbd", ".exp", ".so":
		return "lib"
	case ".h", ".hh", ".hpp", ".hxx", ".inc", ".inl", ".ipp":
		return "include"
	case ".exe", ".dll", ".pdb", ".com":
		return "bin"
	}
	if strings.Contains(name, ".so.") {
		return "lib"
	}
	if fileio.IsELFFile(src) || fileio.IsPEFile(src) {
		return "bin"
	}
	return ""
}

// generateBazelrc writes the bazelrc used by this port into BuildDir and
// returns its path. It is passed via --bazelrc (see startupArgs) so that
// SrcDir, shared by all platforms of the port, is never modified.
func (b bazel) generateBazelrc(ccToolchainDir string) (string, error) {
	toolchain := b.Ctx.Platform().GetToolchain()
	rootfs := b.Ctx.Platform().GetRootFS()

	var buffer bytes.Buffer
	fmt.Fprintf(&buffer, "# Generated by celer for %s\n", b.PortConfig.nameVersion())

	// Remove .bazelrc generated by last build.
	b.removeLegacyBazelrc()

	// Keep all Bazel state and convenience symlinks in BuildDir: ~/.cache/bazel
	// is shared with unrelated builds, and SrcDir is shared by all platforms.
	outputBase := filepath.ToSlash(filepath.Join(b.PortConfig.BuildDir, "output_base"))
	fmt.Fprintf(&buffer, "startup --output_base=%s\n", outputBase)
	symlinkPrefix := filepath.ToSlash(filepath.Join(b.PortConfig.BuildDir, "bazel-"))
	fmt.Fprintf(&buffer, "build --symlink_prefix=%s\n", symlinkPrefix)

	// Map celer's build type onto Bazel's compilation mode; otherwise every
	// "release" build silently uses Bazel's default fastbuild (-O0).
	if mode, flags := b.compilationMode(); mode != "" {
		fmt.Fprintf(&buffer, "build --compilation_mode=%s\n", mode)
		for _, flag := range flags {
			writeFlags(&buffer, flag, "copt", "cxxopt")
		}
	}

	// Cross-compilation only; dev dependencies build for the host.
	if !b.DevDep && !b.HostDev {
		fmt.Fprintf(&buffer, "build --spawn_strategy=local\n")
		// Bazel 6.x dropped @bazel_tools//platforms; keep old projects working.
		fmt.Fprintf(&buffer, "build --incompatible_use_platforms_repo_for_constraints=false\n")

		// Register celer's crosstool instead of @local_config_cc, which injects
		// host binutils and misses the sysroot includes. Without one (no rootfs,
		// non-gcc/clang) Bazel keeps its default host toolchain detection.
		if ccToolchainDir != "" {
			fmt.Fprintf(&buffer, "build --incompatible_enable_cc_toolchain_resolution\n")
			fmt.Fprintf(&buffer, "build --action_env=BAZEL_DO_NOT_DETECT_CPP_TOOLCHAIN=1\n")
			fmt.Fprintf(&buffer, "build --override_repository=cc_toolchain=%s\n", ccToolchainDir)
			fmt.Fprintf(&buffer, "build --extra_toolchains=@cc_toolchain//:cc_toolchain_impl\n")
			fmt.Fprintf(&buffer, "build --platforms=@cc_toolchain//:target_platform\n")
		}

		// Clang runtime flags (--gcc-toolchain, --rtlib) apply to compile and
		// link actions alike.
		for _, flag := range toolchain.RuntimeFlags() {
			writeFlags(&buffer, flag, "copt", "cxxopt", "linkopt")
		}

		// Sysroot headers and --sysroot come from the crosstool; only its lib
		// dirs are needed here so the linker can resolve target libraries.
		if rootfs != nil {
			sysrootDir := filepath.ToSlash(rootfs.GetAbsDir())
			for _, dir := range rootfs.GetLibDirs() {
				writeLibDir(&buffer, filepath.ToSlash(filepath.Join(sysrootDir, dir)))
			}
		}

		// Dependency prefix (staging dir) ahead of the sysroot.
		depDir := filepath.Join(b.PortConfig.StagingDir, b.PortConfig.LibraryDir)
		depInclude := filepath.ToSlash(filepath.Join(depDir, "include"))
		depLib := filepath.ToSlash(filepath.Join(depDir, "lib"))
		if fileio.PathExists(depInclude) {
			writeIncludeDir(&buffer, depInclude)
		}
		if fileio.PathExists(depLib) {
			writeLibDir(&buffer, depLib)
		}

		// Runtime rpath, matching celer's <prefix>/bin + <prefix>/lib layout.
		fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath=\\$$ORIGIN/../lib\n")
		fmt.Fprintf(&buffer, "build --linkopt=-Wl,-rpath=\\$$ORIGIN/../lib64\n")
	}

	// Port options: include/lib dirs, language standards, CFLAGS/CXXFLAGS/LDFLAGS.
	for _, dir := range b.IncludeDirs {
		writeIncludeDir(&buffer, filepath.ToSlash(b.ExprVars.Expand(dir)))
	}
	for _, dir := range b.LibDirs {
		writeLibDir(&buffer, filepath.ToSlash(b.ExprVars.Expand(dir)))
	}
	if b.CStandard != "" {
		writeFlags(&buffer, "-std="+b.CStandard, "copt")
	}
	if b.CXXStandard != "" {
		writeFlags(&buffer, "-std="+b.CXXStandard, "cxxopt")
	}
	for _, env := range b.Envs {
		name, value, ok := strings.Cut(env, "=")
		if !ok {
			continue
		}
		option, known := envFlagOptions[strings.TrimSpace(name)]
		if !known {
			continue
		}
		for part := range strings.FieldsSeq(b.expandVariables(value)) {
			writeFlags(&buffer, part, option)
		}
	}

	if err := os.MkdirAll(b.PortConfig.BuildDir, os.ModePerm); err != nil {
		return "", err
	}
	path := b.bazelrcPath()
	return path, os.WriteFile(path, buffer.Bytes(), os.ModePerm)
}

// envFlagOptions maps port envs onto the Bazel option that carries the same flag.
var envFlagOptions = map[string]string{
	"CFLAGS":   "copt",
	"CXXFLAGS": "cxxopt",
	"LDFLAGS":  "linkopt",
}

// writeFlags writes one "build --<option>=<value>" line per option.
func writeFlags(buffer *bytes.Buffer, value string, options ...string) {
	for _, option := range options {
		fmt.Fprintf(buffer, "build --%s=%s\n", option, value)
	}
}

// writeIncludeDir and writeLibDir emit the flag pairs celer always writes together.
func writeIncludeDir(buffer *bytes.Buffer, dir string) {
	writeFlags(buffer, "-I"+dir, "copt", "cxxopt")
}

func writeLibDir(buffer *bytes.Buffer, dir string) {
	writeFlags(buffer, "-L"+dir, "linkopt")
	writeFlags(buffer, "-Wl,-rpath-link,"+dir, "linkopt")
}

func (b bazel) removeLegacyBazelrc() {
	path := filepath.Join(b.PortConfig.SrcDir, ".bazelrc")
	data, err := os.ReadFile(path)
	if err != nil {
		return
	}
	if bytes.HasPrefix(data, []byte("# Generated by celer for ")) {
		_ = os.Remove(path)
	}
}

func (b bazel) compilationMode() (string, []string) {
	switch strings.ToLower(b.BuildType) {
	case "debug":
		return "dbg", nil
	case "relwithdebinfo":
		return "opt", []string{"-g"}
	case "minsizerel":
		return "opt", []string{"-Os"}
	default:
		// "release" (celer's default) and anything unexpected.
		return "opt", nil
	}
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
        # Bazel enables the feature whose name matches --compilation_mode.
        # The generated toolchain has to provide them; without these, -c opt
        # would not add any optimization flags.
        feature(
            name = "opt",
            flag_sets = [flag_set(
                actions = _COMPILE_ACTIONS,
                flag_groups = [flag_group(flags = ["-O2", "-DNDEBUG"])],
            )],
        ),
        feature(
            name = "dbg",
            flag_sets = [flag_set(
                actions = _COMPILE_ACTIONS,
                flag_groups = [flag_group(flags = ["-g"])],
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
