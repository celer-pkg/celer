package buildsystems

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func NewPip(config *BuildConfig) *pip {
	return &pip{BuildConfig: config}
}

// pip installs a declarative list of Python packages. Target ports install
// into the target site-packages through the target Python (optionally under
// qemu); For dev/host ports then install into the host venv.
type pip struct {
	*BuildConfig
}

func (p pip) Name() string {
	return "pip"
}

func (p pip) configured() bool {
	return false
}

func (p pip) CheckTools() []string {
	toolchain := p.Ctx.Platform().GetToolchain()
	tools := slices.Clone(p.BuildConfig.BuildTools)
	configMatch := p.BuildConfig.SystemProcessor == "aarch64" && p.BuildConfig.SystemName == "linux"
	detectMatch := toolchain.GetSystemProcessor() == "aarch64" && toolchain.GetSystemName() == "linux"
	if configMatch || detectMatch {
		tools = append(tools, "qemu-aarch64")
	}
	return tools
}

func (p pip) Configure(options []string) error {
	return nil
}

func (p pip) Build(options []string) error {
	if len(p.PythonPackages) == 0 {
		return fmt.Errorf("build_system 'pip' requires python_packages for %s", p.PortConfig.nameVersion())
	}

	if err := os.MkdirAll(p.PortConfig.BuildDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create build dir %s -> %w", p.PortConfig.BuildDir, err)
	}

	targetDir, err := p.targetSitePackages()
	if err != nil {
		return err
	}
	if err := os.MkdirAll(targetDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create python target dir %s -> %w", targetDir, err)
	}

	requirementsFile, err := p.writeRequirements()
	if err != nil {
		return err
	}
	pythonExe, err := p.pythonExecutable()
	if err != nil {
		return err
	}
	wheelhouse, err := p.wheelhouseDir()
	if err != nil {
		return err
	}
	if err := os.MkdirAll(wheelhouse, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create python wheelhouse %s -> %w", wheelhouse, err)
	}

	// Prefer an existing wheelhouse. If it is incomplete, refresh it online.
	if wheelhouseHasFiles(wheelhouse) {
		if err := p.installFromWheelhouse(pythonExe, wheelhouse, targetDir, requirementsFile); err == nil {
			return nil
		} else if p.Ctx.Offline() {
			return fmt.Errorf("offline mode: failed to install %s from wheelhouse %s -> %w",
				p.PortConfig.nameVersion(), wheelhouse, err)
		}

		// The cached wheel set was incomplete. Clear the partial install before
		// refreshing the wheelhouse and retrying.
		if err := os.RemoveAll(targetDir); err != nil {
			return fmt.Errorf("failed to clean incomplete python target dir %s -> %w", targetDir, err)
		}
		if err := os.MkdirAll(targetDir, os.ModePerm); err != nil {
			return fmt.Errorf("failed to recreate python target dir %s -> %w", targetDir, err)
		}
	} else if p.Ctx.Offline() {
		return fmt.Errorf("offline mode: no python packages available in %s for %s",
			wheelhouse, p.PortConfig.nameVersion())
	}

	if err := p.downloadWheels(pythonExe, wheelhouse, requirementsFile); err != nil {
		return err
	}
	return p.installFromWheelhouse(pythonExe, wheelhouse, targetDir, requirementsFile)
}

func (p pip) Install(options []string) error {
	return nil
}

func (p pip) pythonEnvRequired() bool {
	return (p.DevDep || p.HostDev) && len(p.PythonPackages) > 0
}

func (p pip) writeRequirements() (string, error) {
	requirementsFile := filepath.Join(p.PortConfig.BuildDir, "requirements.txt")
	var requirements strings.Builder
	for _, packageSpec := range p.PythonPackages {
		normalized, err := formatPythonPackage(p.expandVariables(packageSpec))
		if err != nil {
			return "", err
		}
		requirements.WriteString(normalized)
		requirements.WriteString("\n")
	}
	if err := os.WriteFile(requirementsFile, []byte(requirements.String()), 0o644); err != nil {
		return "", fmt.Errorf("failed to write %s -> %w", requirementsFile, err)
	}
	return requirementsFile, nil
}

func (p pip) installFromWheelhouse(pythonExe, wheelhouse, targetDir, requirementsFile string) error {
	args := []string{
		"-s", "-m", "pip", "install", "--upgrade",
		"--no-index", "--find-links", wheelhouse,
		"--target", targetDir,
		"-r", requirementsFile,
	}
	return p.runPip("build", pythonExe, args)
}

func (p pip) downloadWheels(pythonExe, wheelhouse, requirementsFile string) error {
	args := []string{
		"-s", "-m", "pip", "download",
		"-d", wheelhouse,
		"-r", requirementsFile,
	}
	args = append(args, p.indexArgs()...)
	return p.runPip("download", pythonExe, args)
}

func (p pip) runPip(logSuffix, pythonExe string, args []string) error {
	command, commandArgs := pythonExe, args
	if emulator, emulatorArgs := p.emulatorCommand(); emulator != "" {
		command = emulator
		commandArgs = append([]string{}, emulatorArgs...)
		commandArgs = append(commandArgs, pythonExe)
		commandArgs = append(commandArgs, args...)
	}

	executor := cmd.NewExecutor(fmt.Sprintf("[pip %s %s]", p.PortConfig.nameVersion(), logSuffix), command, commandArgs...)
	executor.SetWorkDir(p.PortConfig.BuildDir)
	executor.SetLogPath(p.getLogPath(logSuffix))
	return executor.Execute()
}

func (p pip) indexArgs() []string {
	var args []string
	pythonConfig := p.Ctx.Project().GetPythonConfig()
	if pythonConfig == nil {
		return nil
	}
	if indexURL := pythonConfig.GetIndexUrl(); indexURL != "" {
		args = append(args, "-i", indexURL)
	}
	for _, extraIndexURL := range pythonConfig.GetExtraIndexUrls() {
		args = append(args, "--extra-index-url", extraIndexURL)
	}
	for _, trustedHost := range pythonConfig.GetTrustedHosts() {
		args = append(args, "--trusted-host", trustedHost)
	}
	return args
}

func (p pip) pythonExecutable() (string, error) {
	if p.DevDep || p.HostDev {
		if buildtools.PythonTool == nil || buildtools.PythonTool.Path == "" {
			return "", fmt.Errorf("python tool is not available for %s", p.PortConfig.nameVersion())
		}
		return buildtools.PythonTool.Path, nil
	}

	pythonVersion, err := p.pythonVersion()
	if err != nil {
		return "", err
	}

	var candidates []string
	if strings.EqualFold(p.Ctx.Platform().GetToolchain().GetSystemName(), "windows") {
		candidates = []string{"python.exe"}
	} else if strings.HasPrefix(pythonVersion, "2") {
		candidates = []string{"python2", "python"}
	} else {
		candidates = []string{"python3", "python"}
	}

	for _, candidate := range candidates {
		pythonPath := filepath.Join(p.PortConfig.StagingDir, p.PortConfig.LibraryDir, "bin", candidate)
		if fileio.PathExists(pythonPath) {
			return pythonPath, nil
		}
	}
	return "", fmt.Errorf("target python is not available under %s/bin (tried %s), add cpython@<version> to dependencies of %s",
		filepath.Join(p.PortConfig.StagingDir, p.PortConfig.LibraryDir), strings.Join(candidates, ", "), p.PortConfig.nameVersion())
}

func (p pip) pythonVersion() (string, error) {
	if p.DevDep || p.HostDev {
		if buildtools.PythonTool != nil && buildtools.PythonTool.Version != "" {
			return buildtools.PythonTool.Version, nil
		}
		return "", fmt.Errorf("python tool is not available for %s", p.PortConfig.nameVersion())
	}

	pythonVersion := cpythonMinorVersion(p.Dependencies)
	if pythonVersion == "" {
		return "", fmt.Errorf("cannot determine target Python version for %s, add cpython@<version> to dependencies",
			p.PortConfig.nameVersion())
	}
	return pythonVersion, nil
}

func (p pip) targetSitePackages() (string, error) {
	if p.DevDep || p.HostDev {
		if buildtools.PythonTool == nil {
			return "", fmt.Errorf("python tool is not available for %s", p.PortConfig.nameVersion())
		}
		return filepath.Join(p.PortConfig.PackageDir, buildtools.PythonTool.SitePackagesDir()), nil
	}

	pythonVersion, err := p.pythonVersion()
	if err != nil {
		return "", err
	}

	if strings.EqualFold(p.Ctx.Platform().GetToolchain().GetSystemName(), "windows") {
		return filepath.Join(p.PortConfig.PackageDir, "Lib", "site-packages"), nil
	}
	return filepath.Join(p.PortConfig.PackageDir, "lib", "python"+minorVersion(pythonVersion), "site-packages"), nil
}

func (p pip) wheelhouseDir() (string, error) {
	pythonVersion, err := p.pythonVersion()
	if err != nil {
		return "", err
	}

	scope := expr.FormatProcessor(p.Ctx.Platform().GetToolchain().GetSystemProcessor()) + "-" +
		strings.ToLower(strings.TrimSpace(p.Ctx.Platform().GetToolchain().GetSystemName()))
	if p.DevDep || p.HostDev {
		scope = expr.FormatProcessor(runtime.GOARCH) + "-" + runtime.GOOS
	}
	return filepath.Join(p.Ctx.Downloads(), "python-wheels", "py"+minorVersion(pythonVersion)+"-"+scope), nil
}

func (p pip) emulatorCommand() (string, []string) {
	if p.DevDep || p.HostDev {
		return "", nil
	}

	processor := expr.FormatProcessor(p.Ctx.Platform().GetToolchain().GetSystemProcessor())
	if processor == "" || processor == expr.FormatProcessor(runtime.GOARCH) {
		return "", nil
	}

	for _, tool := range p.CheckTools() {
		name, _, _ := strings.Cut(tool, "@")
		name = strings.TrimSpace(name)

		// "qemu" is generic: derive the qemu binary from the target processor.
		if name == "qemu" {
			return "qemu-" + processor + "-static", p.emulatorArgs()
		}

		// "qemu-aarch64" (optionally "-static") names the architecture explicitly.
		base := strings.TrimSuffix(name, "-static")
		arch, ok := strings.CutPrefix(base, "qemu-")
		if !ok || expr.FormatProcessor(arch) != processor {
			continue
		}
		return "qemu-" + processor + "-static", p.emulatorArgs()
	}
	return "", nil
}

func (p pip) emulatorArgs() []string {
	rootfs := p.Ctx.Platform().GetRootFS()
	if rootfs == nil {
		return nil
	}
	return []string{"-L", rootfs.GetAbsDir()}
}

func formatPythonPackage(spec string) (string, error) {
	name, version, ok := strings.Cut(strings.TrimSpace(spec), "@")
	name, version = strings.TrimSpace(name), strings.TrimSpace(version)

	if !ok || name == "" || version == "" {
		return "", fmt.Errorf("invalid python package spec %q, it should be like name@version", spec)
	}
	return name + "==" + version, nil
}

func wheelhouseHasFiles(dir string) bool {
	entries, err := os.ReadDir(dir)
	if err != nil {
		return false
	}
	for _, entry := range entries {
		if !entry.IsDir() {
			return true
		}
	}
	return false
}

func cpythonMinorVersion(dependencies []string) string {
	for _, dependency := range dependencies {
		name, version, ok := strings.Cut(dependency, "@")
		if !ok || (name != "cpython" && name != "cpython2") {
			continue
		}
		return minorVersion(version)
	}
	return ""
}

func minorVersion(version string) string {
	parts := strings.Split(version, ".")
	if len(parts) >= 2 {
		return parts[0] + "." + parts[1]
	}
	return version
}
