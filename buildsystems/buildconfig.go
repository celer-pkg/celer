package buildsystems

import (
	"celer/buildtools"
	"celer/context"
	"celer/generator"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

const supportedString = "nobuild, prebuilt, b2, cmake, gyp, makefiles, meson, qmake"

var supportedArray = []string{"nobuild", "prebuilt", "b2", "cmake", "gyp", "makefiles", "meson", "qmake"}

type PortConfig struct {
	LibName         string     // like: `ffmpeg`
	LibVersion      string     // like: `4.4`
	Archive         string     // like: `ffmpeg-4.4.tar.xz`
	Url             string     // like: `https://ffmpeg.org/releases/ffmpeg-4.4.tar.xz`
	IgnoreSubmodule bool       // whether ignore submodule during git clone.
	HostName        string     // like: `x86_64-linux`, `x86_64-windows`
	ProjectName     string     // toml filename in conf/projects.
	Toolchain       *Toolchain // same with `Toolchain` in config/toolchain.go
	SrcDir          string     // for example: ${workspace}/buildtrees/icu@75.1/src/icu4c/source
	RepoDir         string     // for example: ${workspace}/buildtrees/icu@75.1/src
	BuildDir        string     // for example: ${workspace}/buildtrees/ffmpeg/x86_64-linux-20.04-Release
	PackageDir      string     // for example: ${workspace}/packages/ffmpeg-3.4.13-x86_64-linux-20.04-Release
	LibraryFolder   string     // for example: aarch64-linux-gnu-gcc-9.2@project_01_standard@Release
	IncludeDirs     []string   // headers not in standard include path.
	LibDirs         []string   // libs not in standard lib path.
	Jobs            int        // number of jobs to run in parallel
	DevDep          bool       // whether dev dependency
	Native          bool       // whether native build
}

func (p PortConfig) nameVersionDesc() string {
	if p.DevDep {
		return fmt.Sprintf("%s@%s@dev", p.LibName, p.LibVersion)
	} else {
		return fmt.Sprintf("%s@%s", p.LibName, p.LibVersion)
	}
}

type buildSystem interface {
	eventHook

	Name() string
	CheckTools() error

	// Clean repo.
	Clean() error

	// Clone & patch source code
	Clone(repoUrl, repoRef, archive string) error
	Patch() error

	// Configure
	configureOptions() ([]string, error)
	configured() bool
	Configure(options []string) error

	// Build
	buildOptions() ([]string, error)
	Build(options []string) error

	// Install
	installOptions() ([]string, error)
	Install(options []string) error

	setupEnvs()
	rollbackEnvs()

	fillPlaceHolders()
	getLogPath(suffix string) string
}

type libraryType struct {
	enableShared  string
	disableShared string
	enableStatic  string
	disableStatic string
}

type BuildConfig struct {
	Pattern string `toml:"pattern,omitempty"`
	Url     string `toml:"url,omitempty"` // Used to override url in package.

	// Build System
	BuildSystem         string `toml:"build_system"`
	BuildSystem_Windows string `toml:"build_system_windows,omitempty"`
	BuildSystem_Linux   string `toml:"build_system_linux,omitempty"`
	BuildSystem_Darwin  string `toml:"build_system_darwin,omitempty"`

	// CMakeGenerator
	CMakeGenerator         string `toml:"cmake_generator,omitempty"`
	CMakeGenerator_Windows string `toml:"cmake_generator_windows,omitempty"`
	CMakeGenerator_Linux   string `toml:"cmake_generator_linux,omitempty"`
	CMakeGenerator_Darwin  string `toml:"cmake_generator_darwin,omitempty"`

	// Build Tools
	BuildTools         []string `toml:"build_tools,omitempty"`
	BuildTools_Windows []string `toml:"build_tools_windows,omitempty"`
	BuildTools_Linux   []string `toml:"build_tools_linux,omitempty"`
	BuildTools_Darwin  []string `toml:"build_tools_darwin,omitempty"`

	// Library Type
	LibraryType         string `toml:"library_type,omitempty"`
	LibraryType_Windows string `toml:"library_type_windows,omitempty"`
	LibraryType_Linux   string `toml:"library_type_linux,omitempty"`
	LibraryType_Darwin  string `toml:"library_type_darwin,omitempty"`

	// BuildShared
	BuildShared         string `toml:"build_shared,omitempty"`
	BuildShared_Windows string `toml:"build_shared_windows,omitempty"`
	BuildShared_Linux   string `toml:"build_shared_linux,omitempty"`
	BuildShared_Darwin  string `toml:"build_shared_darwin,omitempty"`

	// BuildStatic
	BuildStatic         string `toml:"build_static,omitempty"`
	BuildStatic_Windows string `toml:"build_static_windows,omitempty"`
	BuildStatic_Linux   string `toml:"build_static_linux,omitempty"`
	BuildStatic_Darwin  string `toml:"build_static_windows,omitempty"`

	// C Standard
	CStandard         string `toml:"c_standard,omitempty"`
	CStandard_Windows string `toml:"c_standard_windows,omitempty"`
	CStandard_Linux   string `toml:"c_standard_linux,omitempty"`
	CStandard_Darwin  string `toml:"c_standard_darwin,omitempty"`

	// C++ Standard
	CXXStandard         string `toml:"cxx_standard,omitempty"`
	CXXStandard_Windows string `toml:"cxx_standard_windows,omitempty"`
	CXXStandard_Linux   string `toml:"cxx_standard_linux,omitempty"`
	CXXStandard_Darwin  string `toml:"cxx_standard_darwin,omitempty"`

	// Environment Variables
	Envs         []string `toml:"envs,omitempty"`
	Envs_Windows []string `toml:"envs_windows,omitempty"`
	Envs_Linux   []string `toml:"envs_linux,omitempty"`
	Envs_Darwin  []string `toml:"envs_darwin,omitempty"`

	// Pathces
	Patches         []string `toml:"patches,omitempty"`
	Patches_Windows []string `toml:"patches_windows,omitempty"`
	Patches_Linux   []string `toml:"patches_linux,omitempty"`
	Patches_Darwin  []string `toml:"patches_darwin,omitempty"`

	// BuildInSource
	BuildInSource         bool  `toml:"build_in_source,omitempty"`
	BuildInSource_Windows *bool `toml:"build_in_source_windows,omitempty"`
	BuildInSource_Linux   *bool `toml:"build_in_source_linux,omitempty"`
	BuildInSource_Darwin  *bool `toml:"build_in_source_darwin,omitempty"`

	// Autogen Options
	AutogenOptions         []string `toml:"autogen_options,omitempty"`
	AutogenOptions_Windows []string `toml:"autogen_options_windows,omitempty"`
	AutogenOptions_Linux   []string `toml:"autogen_options_linux,omitempty"`
	AutogenOptions_Darwin  []string `toml:"autogen_options_darwin,omitempty"`

	// Dependencies
	Dependencies         []string `toml:"dependencies,omitempty"`
	Dependencies_Windows []string `toml:"dependencies_windows,omitempty"`
	Dependencies_Linux   []string `toml:"dependencies_linux,omitempty"`
	Dependencies_Darwin  []string `toml:"dependencies_darwin,omitempty"`

	// Dev Dependencies
	DevDependencies         []string `toml:"dev_dependencies,omitempty"`
	DevDependencies_Windows []string `toml:"dev_dependencies_windows,omitempty"`
	DevDependencies_Linux   []string `toml:"dev_dependencies_linux,omitempty"`
	DevDependencies_Darwin  []string `toml:"dev_dependencies_darwin,omitempty"`

	// Event hooks for PreConfigure
	PreConfigure         []string `toml:"pre_configure,omitempty"`
	PreConfigure_Windows []string `toml:"pre_configure_windows,omitempty"`
	PreConfigure_Linux   []string `toml:"pre_configure_linux,omitempty"`
	PreConfigure_Darwin  []string `toml:"pre_configure_darwin,omitempty"`

	// Event hooks for PostConfigure
	PostConfigure         []string `toml:"post_configure,omitempty"`
	PostConfigure_Windows []string `toml:"post_configure_windows,omitempty"`
	PostConfigure_Linux   []string `toml:"post_configure_linux,omitempty"`
	PostConfigure_Darwin  []string `toml:"post_configure_darwin,omitempty"`

	// Event hooks for PreBuild
	PreBuild         []string `toml:"pre_build,omitempty"`
	PreBuild_Windows []string `toml:"pre_build_windows,omitempty"`
	PreBuild_Linux   []string `toml:"pre_build_linux,omitempty"`
	PreBuild_Darwin  []string `toml:"pre_build_darwin,omitempty"`

	// Event hooks for FixBuild
	FixBuild         []string `toml:"fix_build,omitempty"`
	FixBuild_Windows []string `toml:"fix_build_windows,omitempty"`
	FixBuild_Linux   []string `toml:"fix_build_linux,omitempty"`
	FixBuild_Darwin  []string `toml:"fix_build_darwin,omitempty"`

	// Event hooks for PostBuild
	PostBuild         []string `toml:"post_build,omitempty"`
	PostBuild_Windows []string `toml:"post_build_windows,omitempty"`
	PostBuild_Linux   []string `toml:"post_build_linux,omitempty"`
	PostBuild_Darwin  []string `toml:"post_build_darwin,omitempty"`

	// Event hooks for PreInstall
	PreInstall         []string `toml:"pre_install,omitempty"`
	PreInstall_Windows []string `toml:"pre_install_windows,omitempty"`
	PreInstall_Linux   []string `toml:"pre_install_linux,omitempty"`
	PreInstall_Darwin  []string `toml:"pre_install_darwin,omitempty"`

	// Event hooks for PostInstall
	PostInstall         []string `toml:"post_install,omitempty"`
	PostInstall_Windows []string `toml:"post_install_windows,omitempty"`
	PostInstall_Linux   []string `toml:"post_install_linux,omitempty"`
	PostInstall_Darwin  []string `toml:"post_install_darwin,omitempty"`

	// Configure Options
	Options         []string `toml:"options,omitempty"`
	Options_Windows []string `toml:"options_windows,omitempty"`
	Options_Linux   []string `toml:"options_linux,omitempty"`
	Options_Darwin  []string `toml:"options_darwin,omitempty"`

	// Internal fields
	Ctx         context.Context   `toml:"-"`
	DevDep      bool              `toml:"-"`
	Native      bool              `toml:"-"`
	PortConfig  PortConfig        `toml:"-"`
	BuildType   string            `toml:"-"` // It'll be converted to lowercase in use.
	Optimize    *context.Optimize `toml:"-"`
	buildSystem buildSystem
	envBackup   envsBackup
}

func (b BuildConfig) Validate() error {
	if b.BuildSystem == "" {
		return fmt.Errorf("build_tool is empty, it should be one of %s", supportedString)
	}

	if !slices.Contains(supportedArray, b.BuildSystem) {
		return fmt.Errorf("unsupported build tool: %s, it should be one of %s", b.BuildSystem, supportedString)
	}

	return nil
}

func (b BuildConfig) libraryType(defaultEnableShared, defaultEnableStatic string) libraryType {
	var (
		enableShared, disableShared string
		enableStatic, disableStatic string
	)
	splitBuildOption := func(buildOption string) (string, string) {
		parts := strings.Split(buildOption, "|")
		if len(parts) == 2 {
			return parts[0], parts[1]
		}
		return parts[0], ""
	}

	enableShared, disableShared = splitBuildOption(b.BuildShared)
	enableStatic, disableStatic = splitBuildOption(b.BuildStatic)

	if enableShared == "" {
		enableShared = defaultEnableShared
	}
	if enableStatic == "" {
		enableStatic = defaultEnableStatic
	}

	return libraryType{
		enableShared:  enableShared,
		enableStatic:  enableStatic,
		disableShared: disableShared,
		disableStatic: disableStatic,
	}
}

func (b BuildConfig) Clone(repoUrl, repoRef, archive string) error {
	// For git repo, clone it when source dir doesn't exists.
	if strings.HasSuffix(repoUrl, ".git") {
		if !fileio.PathExists(b.PortConfig.RepoDir) {
			// Clone repo.
			title := fmt.Sprintf("[clone %s]", b.PortConfig.nameVersionDesc())
			if err := git.CloneRepo(title, repoUrl, repoRef,
				b.PortConfig.IgnoreSubmodule,
				b.PortConfig.RepoDir); err != nil {
				return err
			}
		}
	} else {
		// Check and repair resource.
		destDir := expr.If(b.buildSystem.Name() == "prebuilt", b.PortConfig.PackageDir, b.PortConfig.RepoDir)
		archive = expr.If(archive == "", filepath.Base(repoUrl), archive)
		repair := fileio.NewRepair(repoUrl, archive, ".", destDir)
		if err := repair.CheckAndRepair(b.Ctx); err != nil {
			return err
		}

		// Move extracted files to repo dir.
		entities, err := os.ReadDir(destDir)
		if err != nil || len(entities) == 0 {
			return fmt.Errorf("failed to find extracted files under repo dir")
		}
		if len(entities) == 1 {
			srcDir := filepath.Join(destDir, entities[0].Name())
			if err := fileio.RenameDir(srcDir, destDir); err != nil {
				return err
			}
		}

		// Init as git repo for tracking file change.
		if b.buildSystem.Name() != "prebuilt" {
			if err := git.InitRepo(destDir, "init for tracking file change"); err != nil {
				return err
			}
		}
	}

	return nil
}

func (b BuildConfig) Clean() error {
	return b.buildSystem.Clean()
}

func (b BuildConfig) Patch() error {
	if len(b.Patches) > 0 {
		// In windows, msys2 is required to apply patch .
		if runtime.GOOS == "windows" {
			if err := buildtools.CheckTools(b.Ctx, "msys2"); err != nil {
				return err
			}
		}

		// Apply all patches.
		for _, patch := range b.Patches {
			patch = strings.TrimSpace(patch)
			if patch == "" {
				continue
			}

			// Find patch file to apply.
			defaultPatchPath := filepath.Join(dirs.PortsDir, b.PortConfig.LibName, b.PortConfig.LibVersion, patch)
			preferedPatchPath := filepath.Join(dirs.ConfProjectsDir, b.PortConfig.ProjectName, b.PortConfig.LibName, b.PortConfig.LibVersion, patch)

			var patchPath string
			if fileio.PathExists(preferedPatchPath) {
				patchPath = preferedPatchPath
			} else if fileio.PathExists(defaultPatchPath) {
				patchPath = defaultPatchPath
			} else {
				return fmt.Errorf("patch %s not found", patch)
			}

			// Apply patch (linux patch or git patch).
			if err := git.ApplyPatch(b.PortConfig.RepoDir, b.PortConfig.RepoDir, patchPath); err != nil {
				return err
			}
		}
	}

	// Copy files under port dir if they are exist.
	overrideFiles := func(portDir string) error {
		// If port dir not exists, skip it.
		if !fileio.PathExists(portDir) {
			return nil
		}

		entities, err := os.ReadDir(portDir)
		if err != nil {
			return fmt.Errorf("failed to read port dir: %s", portDir)
		}
		for _, entity := range entities {
			if entity.Name() != "port.toml" &&
				entity.Name() != "cmake_config.toml" &&
				entity.Name() != "README.md" &&
				!strings.Contains(entity.Name(), ".patch") {
				srcFile := filepath.Join(portDir, entity.Name())
				destFile := filepath.Join(b.PortConfig.SrcDir, entity.Name())
				if !fileio.PathExists(destFile) {
					if err := fileio.CopyFile(srcFile, destFile); err != nil {
						return fmt.Errorf("failed to patch files.\n %w", err)
					}
				}
			}
		}
		return nil
	}
	portDir := filepath.Join(dirs.PortsDir, b.PortConfig.LibName, b.PortConfig.LibVersion)
	if err := overrideFiles(portDir); err != nil {
		return fmt.Errorf("failed to override files from port dir.\n %w", err)
	}
	projectPortDir := filepath.Join(dirs.ConfProjectsDir, b.PortConfig.ProjectName, b.PortConfig.LibName, b.PortConfig.LibVersion)
	if err := overrideFiles(projectPortDir); err != nil {
		return fmt.Errorf("failed to override files from project port dir.\n %w", err)
	}

	return nil
}

// Can be override by different buildsystems.
func (b BuildConfig) configureOptions() ([]string, error) {
	return slices.Clone(b.Options), nil
}

// Can be override by different buildsystems.
func (b BuildConfig) buildOptions() ([]string, error) {
	return nil, nil
}

// Can be override by different buildsystems.
func (b BuildConfig) installOptions() ([]string, error) {
	return nil, nil
}

func (b BuildConfig) Install(url, ref, archive string) error {
	if err := b.buildSystem.CheckTools(); err != nil {
		return fmt.Errorf("failed to check tools for %s.\n %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Replace placeholders with real value, like ${HOST}, ${SYSROOT} etc.
	b.fillPlaceHolders()

	// nobuild config do not have crosstool.
	if b.PortConfig.Toolchain != nil {
		b.setupEnvs()
		defer b.rollbackEnvs()

		// Create a symlink in the sysroot that points to the installed directory,
		// then the pc file would be found by other libraries.
		if b.PortConfig.Toolchain.RootFS != "" {
			// This symblink is used to find library via toolchain_file.cmake
			if err := b.checkSymlink(dirs.InstalledDir,
				filepath.Join(b.PortConfig.Toolchain.RootFS, "installed"),
			); err != nil {
				return err
			}

			// Create tmp dir in rootfs if not exist.
			rootfsTmp := filepath.Join(b.PortConfig.Toolchain.RootFS, "tmp")
			if err := os.MkdirAll(rootfsTmp, os.ModePerm); err != nil {
				return err
			}

			// This symblink is used to find library during build.
			if err := b.checkSymlink(dirs.TmpDepsDir,
				filepath.Join(b.PortConfig.Toolchain.RootFS, "tmp", "deps")); err != nil {
				return err
			}
		}
	}

	// Clone repo, the repo maybe already cloned during computer hash.
	if !fileio.PathExists(b.PortConfig.RepoDir) {
		if err := b.buildSystem.Clone(url, ref, archive); err != nil {
			message := expr.If(strings.HasSuffix(url, ".git"), "clone error", "download error")
			return fmt.Errorf("%s %s: %w", message, b.PortConfig.nameVersionDesc(), err)
		}
	}

	// Apply patches.
	if err := b.buildSystem.Patch(); err != nil {
		return fmt.Errorf("patch %s: %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Configure related steps.
	if !b.buildSystem.configured() {
		if err := b.buildSystem.preConfigure(); err != nil {
			return fmt.Errorf("failed to pre configure %s.\n %w", b.PortConfig.nameVersionDesc(), err)
		}
		configureOptions, err := b.buildSystem.configureOptions()
		if err != nil {
			return fmt.Errorf("configure %s -> %w", b.PortConfig.nameVersionDesc(), err)
		}
		if err := b.buildSystem.Configure(configureOptions); err != nil {
			return fmt.Errorf("configure %s\n %w", b.PortConfig.nameVersionDesc(), err)
		}
		if err := b.buildSystem.postConfigure(); err != nil {
			return fmt.Errorf("post configure %s\n %w", b.PortConfig.nameVersionDesc(), err)
		}
	}

	// Build related steps.
	if err := b.buildSystem.preBuild(); err != nil {
		return fmt.Errorf("pre build %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}
	buildOptions, err := b.buildSystem.buildOptions()
	if err != nil {
		return fmt.Errorf("get build options %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.Build(buildOptions); err != nil {
		// Some third-party need extra steps to fix build. For example: nspr.
		if len(b.FixBuild) > 0 {
			if err := b.buildSystem.fixBuild(); err != nil {
				return fmt.Errorf("fix build %s\n %w", b.PortConfig.nameVersionDesc(), err)
			}
			if err := b.buildSystem.Build(buildOptions); err != nil {
				return fmt.Errorf("build %s again\n %w", b.PortConfig.nameVersionDesc(), err)
			}
		} else {
			return fmt.Errorf("build %s\n %w", b.PortConfig.nameVersionDesc(), err)
		}
	}
	if err := b.buildSystem.postBuild(); err != nil {
		return fmt.Errorf("post build %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Install related steps.
	if err := b.buildSystem.preInstall(); err != nil {
		return fmt.Errorf("pre install %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}
	installOptions, err := b.buildSystem.installOptions()
	if err != nil {
		return fmt.Errorf("get install options %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.Install(installOptions); err != nil {
		return fmt.Errorf("install %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.postInstall(); err != nil {
		return fmt.Errorf("post install %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Fixup pkg config files.
	var prefix = expr.If(b.PortConfig.Toolchain.RootFS == "" || b.DevDep,
		filepath.Join(string(os.PathSeparator), "installed", b.PortConfig.HostName+"-dev"),
		filepath.Join(string(os.PathSeparator), "installed", b.PortConfig.LibraryFolder),
	)
	if err := fileio.FixupPkgConfig(b.PortConfig.PackageDir, prefix); err != nil {
		return fmt.Errorf("fixup pkg-config\n %w", err)
	}

	// Generate cmake configs.
	portDir := filepath.Join(dirs.PortsDir, b.PortConfig.LibName, b.PortConfig.LibVersion)
	preferedPortDir := filepath.Join(dirs.ConfProjectsDir, b.PortConfig.ProjectName, b.PortConfig.LibName, b.PortConfig.LibVersion)
	cmakeConfig, err := generator.FindMatchedConfig(portDir, preferedPortDir, b.PortConfig.Toolchain.SystemName, b.LibraryType)
	if err != nil {
		return fmt.Errorf("find matched config %s\n %w", b.PortConfig.nameVersionDesc(), err)
	}
	if cmakeConfig != nil {
		cmakeConfig.Version = b.PortConfig.LibVersion
		cmakeConfig.SystemName = b.PortConfig.Toolchain.SystemName
		cmakeConfig.Libname = b.PortConfig.LibName
		cmakeConfig.BuildType = b.BuildType
		if err := cmakeConfig.Generate(b.PortConfig.PackageDir); err != nil {
			return err
		}
	}

	return nil
}

func (b *BuildConfig) InitBuildSystem(optimize *context.Optimize) error {
	if b.BuildSystem == "" {
		return fmt.Errorf("build_system is empty")
	}

	switch b.BuildSystem {
	case "nobuild":
		b.buildSystem = NewNoBuild(b, optimize)
	case "gyp":
		b.buildSystem = NewGyp(b, optimize)
	case "cmake":
		b.buildSystem = NewCMake(b, optimize)
	case "makefiles":
		b.buildSystem = NewMakefiles(b, optimize)
	case "meson":
		b.buildSystem = NewMeson(b, optimize)
	case "b2":
		b.buildSystem = NewB2(b, optimize)
	case "bazel":
		b.buildSystem = NewBazel(b, optimize)
	case "qmake":
		b.buildSystem = NewQMake(b, optimize)
	case "prebuilt":
		b.buildSystem = NewPrebuilt(b, optimize)
	default:
		return fmt.Errorf("unsupported build system: %s", b.BuildSystem)
	}

	// Merges the platform-specific fields into the BuildConfig struct.
	b.mergePlatform()
	return nil
}

// checkSymlink create a symlink in the sysroot.
func (b BuildConfig) checkSymlink(src, dest string) error {
	// Convenient function to create a relative symlink.
	createSymlink := func(src, dest string) error {
		relPath, err := filepath.Rel(filepath.Dir(dest), src)
		if err != nil {
			return fmt.Errorf("compute relative path\n %w", err)
		}
		if err := os.Symlink(relPath, dest); err != nil {
			return fmt.Errorf("create symlink\n %w", err)
		}
		return nil
	}

	// Check if the symlink exists.
	info, err := os.Lstat(dest)
	if err != nil {
		if os.IsNotExist(err) {
			return createSymlink(src, dest)
		}
		return fmt.Errorf("checking symlink: %v", err)
	}

	// Check the symlink target.
	if info.Mode()&os.ModeSymlink != 0 {
		// Read the target of the symlink.
		realTarget, err := os.Readlink(dest)
		if err != nil {
			return fmt.Errorf("read symlink target: %v", err)
		}

		// If symlink is broken or points to the wrong target, remove it and recreate.
		if realTarget != src {
			if err := os.Remove(dest); err != nil {
				return fmt.Errorf("remove broken symlink: %v", err)
			}
			return createSymlink(src, dest)
		}

		return nil
	}

	// Remove if it's not a symlink.
	if err = os.Remove(dest); err != nil {
		return fmt.Errorf("remove non-symlink: %v", err)
	}
	return createSymlink(src, dest)
}

// fillPlaceHolders Replace placeholders with real paths and values.
func (b *BuildConfig) fillPlaceHolders() {
	for index, argument := range b.Options {
		if strings.Contains(argument, "${HOST}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${HOST}", b.PortConfig.Toolchain.Host)
			}
		}

		if strings.Contains(argument, "${SYSTEM_NAME}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${SYSTEM_NAME}", strings.ToLower(b.PortConfig.Toolchain.SystemName))
			}
		}

		if strings.Contains(argument, "${SYSTEM_PROCESSOR}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${SYSTEM_PROCESSOR}", b.PortConfig.Toolchain.SystemProcessor)
			}
		}

		if strings.Contains(argument, "${SYSROOT}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${SYSROOT}", b.PortConfig.Toolchain.RootFS)
			}
		}

		if strings.Contains(argument, "${CROSSTOOL_PREFIX}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${CROSSTOOL_PREFIX}", b.PortConfig.Toolchain.CrosstoolPrefix)
			}
		}

		if strings.Contains(argument, "${DEPS_DIR}") {
			b.Options[index] = strings.ReplaceAll(argument, "${DEPS_DIR}",
				filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder))
		}

		if strings.Contains(argument, "${SRC_DIR}") {
			b.Options[index] = strings.ReplaceAll(argument, "${SRC_DIR}", b.PortConfig.SrcDir)
		}
	}
}

func (b BuildConfig) replaceSource(archive, url string) error {
	var replaceFailed bool

	// Backup repo
	if err := os.Rename(b.PortConfig.RepoDir, b.PortConfig.RepoDir+"_bak"); err != nil {
		return err
	}

	defer func() {
		// Rollback repo if repalce source failed.
		if replaceFailed {
			os.Rename(b.PortConfig.RepoDir+"_bak", b.PortConfig.RepoDir)
		} else {
			os.RemoveAll(b.PortConfig.RepoDir + "_bak")
		}
	}()

	// Check and repair resource.
	archive = expr.If(archive == "", filepath.Base(url), archive)
	repair := fileio.NewRepair(url, archive, ".", b.PortConfig.RepoDir)
	if err := repair.CheckAndRepair(b.Ctx); err != nil {
		replaceFailed = true
		return err
	}

	// Move extracted files to source dir.
	entities, err := os.ReadDir(b.PortConfig.RepoDir)
	if err != nil || len(entities) == 0 {
		replaceFailed = true
		return fmt.Errorf("failed to find extracted files under tmp dir")
	}
	if len(entities) == 1 {
		srcDir := filepath.Join(b.PortConfig.RepoDir, entities[0].Name())
		if err := fileio.RenameDir(srcDir, b.PortConfig.RepoDir); err != nil {
			replaceFailed = true
			return err
		}
	}

	// Init as git repo for tracking file change.
	if err := git.InitRepo(b.PortConfig.RepoDir, "init for tracking file change"); err != nil {
		return err
	}

	return nil
}

func (b BuildConfig) replaceHolders(content string) string {
	content = strings.ReplaceAll(content, "${SYSTEM_NAME}", b.PortConfig.Toolchain.SystemName)
	content = strings.ReplaceAll(content, "${HOST}", b.PortConfig.Toolchain.Host)
	content = strings.ReplaceAll(content, "${SYSTEM_PROCESSOR}", b.PortConfig.Toolchain.SystemProcessor)
	content = strings.ReplaceAll(content, "${SYSROOT}", b.PortConfig.Toolchain.RootFS)
	content = strings.ReplaceAll(content, "${CROSSTOOL_PREFIX}", b.PortConfig.Toolchain.CrosstoolPrefix)
	content = strings.ReplaceAll(content, "${BUILD_DIR}", b.PortConfig.BuildDir)
	content = strings.ReplaceAll(content, "${PACKAGE_DIR}", b.PortConfig.PackageDir)
	content = strings.ReplaceAll(content, "${DEPS_DEV_DIR}", filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev"))
	content = strings.ReplaceAll(content, "${BUILDTREES_DIR}", dirs.BuildtreesDir)

	// Replace ${SRC_DIR} with repoDir.
	content = strings.ReplaceAll(content, "${REPO_DIR}", b.PortConfig.RepoDir)

	if b.DevDep {
		content = strings.ReplaceAll(content, "${DEPS_DIR}", filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev"))
	} else {
		content = strings.ReplaceAll(content, "${DEPS_DIR}", filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder))
	}

	if buildtools.Python3 != nil {
		content = strings.ReplaceAll(content, "${PYTHON3_PATH}", buildtools.Python3.Path)
	}

	return content
}

func (b BuildConfig) getLogPath(suffix string) string {
	parentDir := filepath.Dir(b.PortConfig.BuildDir)
	fileName := filepath.Base(b.PortConfig.BuildDir) + fmt.Sprintf("-%s.log", suffix)
	return filepath.Join(parentDir, fileName)
}

// msvcEnvs provider the MSVC environment variables required by msys2.
func (b BuildConfig) msvcEnvs() (string, error) {
	// Append envs if exist.
	var envs []string
	var appendEnv = func(envKey, envValue string) {
		if envValue != "" {
			envs = append(envs, fmt.Sprintf(`%s="%s"`, envKey, envValue))
		}
	}

	var cflags, cxxflags, ldflags []string

	// Set optimization flags with build_type.
	if b.Optimize != nil {
		if b.DevDep {
			if b.Optimize.Release != "" {
				cflags = append(cflags, b.Optimize.Release)
				cxxflags = append(cxxflags, b.Optimize.Release)
			}
		} else {
			switch b.BuildType {
			case "release":
				if b.Optimize.Release != "" {
					cflags = append(cflags, b.Optimize.Release)
					cxxflags = append(cxxflags, b.Optimize.Release)
				}
			case "debug":
				if b.Optimize.Debug != "" {
					cflags = append(cflags, b.Optimize.Debug)
					cxxflags = append(cxxflags, b.Optimize.Debug)
				}
			case "relwithdebinfo":
				if b.Optimize.RelWithDebInfo != "" {
					cflags = append(cflags, b.Optimize.RelWithDebInfo)
					cxxflags = append(cxxflags, b.Optimize.RelWithDebInfo)
				}
			case "minsizerel":
				if b.Optimize.MinSizeRel != "" {
					cflags = append(cflags, b.Optimize.MinSizeRel)
					cxxflags = append(cxxflags, b.Optimize.MinSizeRel)
				}
			}
		}
	}

	// Set CFLAGS/CXXFLAGS/LDFLAGS.
	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)
	var appendIncludeDir = func(includeDir string) {
		includeDir = fileio.ToCygpath(includeDir)
		includeFlag := "-isystem " + includeDir
		cflags = append(cflags, includeFlag)
		cxxflags = append(cxxflags, includeFlag)
	}
	var appendLibDir = func(libdir string) {
		libdir = fileio.ToCygpath(libdir)
		lFlag := "-L" + libdir
		rFlag := "-Wl,-rpath-link," + libdir

		// Add -L/rpath-link flag.
		if !slices.Contains(ldflags, lFlag) {
			ldflags = append(ldflags, lFlag)
		}
		if !slices.Contains(ldflags, rFlag) {
			ldflags = append(ldflags, rFlag)
		}
	}

	// sysroot and tmp dir.
	if b.DevDep {
		// Append CFLAGS/CXXFLAGS/LDFLAGS
		appendIncludeDir(filepath.Join(tmpDepsDir, "include"))
		appendLibDir(filepath.Join(tmpDepsDir, "lib"))
	} else if b.PortConfig.Toolchain.RootFS != "" {
		// Update CFLAGS/CXXFLAGS
		appendIncludeDir(filepath.Join(tmpDepsDir, "include"))
		for _, dir := range b.PortConfig.Toolchain.IncludeDirs {
			appendIncludeDir(filepath.Join(b.PortConfig.Toolchain.RootFS, dir))
		}

		// Append LDFLAGS
		appendLibDir(filepath.Join(tmpDepsDir, "lib"))
		for _, dir := range b.PortConfig.Toolchain.LibDirs {
			appendLibDir(filepath.Join(b.PortConfig.Toolchain.RootFS, dir))
		}
	}
	appendEnv("CFLAGS", strings.Join(cflags, " "))
	appendEnv("CXXFLAGS", strings.Join(cxxflags, " "))
	appendEnv("LDFLAGS", strings.Join(ldflags, " "))

	// pkg-config
	var (
		configPaths   []string
		configLibDirs []string
		pathDivider   string
		sysrootDir    string
	)
	configPaths = []string{
		fileio.ToCygpath(filepath.Join(tmpDepsDir, "lib", "pkgconfig")),
		fileio.ToCygpath(filepath.Join(tmpDepsDir, "share", "pkgconfig")),
	}
	sysrootDir = fileio.ToCygpath(tmpDepsDir)
	pathDivider = ":"

	// Set merged pkgconfig envs.
	appendEnv("PKG_CONFIG_PATH", strings.Join(configPaths, pathDivider))
	appendEnv("PKG_CONFIG_LIBDIR", strings.Join(configLibDirs, pathDivider))
	appendEnv("PKG_CONFIG_SYSROOT_DIR", sysrootDir)

	// Load MSVC environment variables.
	msvcEnvs, err := b.readMSVCEnvs()
	if err != nil {
		return "", err
	}

	// Append MSVC related envs.
	parts := strings.Split(os.Getenv("PATH"), ";")
	msvcPaths := strings.Split(msvcEnvs["PATH"], ";")
	parts = append(parts, msvcPaths...)
	for index, path := range parts {
		parts[index] = fileio.ToCygpath(path)
	}
	appendEnv("PATH", fmt.Sprintf("%s:${PATH}", strings.Join(parts, ":")))
	appendEnv("INCLUDE", msvcEnvs["INCLUDE"])
	appendEnv("LIB", msvcEnvs["LIB"])

	return strings.Join(envs, " "), nil
}

func (b BuildConfig) readMSVCEnvs() (map[string]string, error) {
	// Read MSVC environment variables.
	command := fmt.Sprintf(`call "%s" x64 && set`, b.PortConfig.Toolchain.MSVC.VCVars)
	executor := cmd.NewExecutor("read msvc envs", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return nil, err
	}

	// Parse environment variables from output.
	var msvcEnvs = make(map[string]string)
	lines := strings.Split(output, "\n")
	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line != "" && strings.Contains(line, "=") {
			parts := strings.Split(line, "=")
			msvcEnvs[parts[0]] = parts[1]
		}
	}

	return msvcEnvs, nil
}
