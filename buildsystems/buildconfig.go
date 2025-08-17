package buildsystems

import (
	"celer/buildtools"
	"celer/generator"
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

const supportedString = "nobuild, prebuilt, b2, cmake, gyp, makefiles, meson, ninja, qmake"

var supportedArray = []string{"nobuild", "prebuilt", "b2", "cmake", "gyp", "makefiles", "meson", "ninja", "qmake"}

type PortConfig struct {
	LibName       string      // like: `ffmpeg`
	LibVersion    string      // like: `4.4`
	Archive       string      // like: `ffmpeg-4.4.tar.xz`
	Url           string      // like: `https://ffmpeg.org/releases/ffmpeg-4.4.tar.xz`
	HostName      string      // like: `x86_64-linux`, `x86_64-windows`
	ProjectName   string      // toml filename in conf/projects.
	CrossTools    *CrossTools // cross tools like CC, CXX, FC, RANLIB, AR, LD, NM, OBJDUMP, STRIP
	SrcDir        string      // for example: ${workspace}/buildtrees/icu@75.1/src/icu4c/source
	RepoDir       string      // for example: ${workspace}/buildtrees/icu@75.1/src
	BuildDir      string      // for example: ${workspace}/buildtrees/ffmpeg/x86_64-linux-20.04-Release
	PackageDir    string      // for example: ${workspace}/packages/ffmpeg-3.4.13-x86_64-linux-20.04-Release
	LibraryFolder string      // for example: aarch64-linux-gnu-gcc-9.2@project_01_standard@Release
	IncludeDirs   []string    // headers not in standard include path.
	LibDirs       []string    // libs not in standard lib path.
	JobNum        int         // number of jobs to run in parallel
	DevDep        bool        // whether dev dependency
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

	// CleanRepo repo.
	CleanRepo() error

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
	setBuildType(buildType string)
	getLogPath(suffix string) string
}

type libraryType struct {
	enableShared  string
	disableShared string
	enableStatic  string
	disableStatic string
}

type BuildConfig struct {
	Pattern string `toml:"pattern"`
	Url     string `toml:"url,omitempty"` // Used to override url in package.

	// Build System
	BuildSystem         string `toml:"build_system"`
	BuildSystem_Windows string `toml:"build_system_windows,omitempty"`
	BuildSystem_Linux   string `toml:"build_system_linux,omitempty"`
	BuildSystem_Darwin  string `toml:"build_system_darwin,omitempty"`

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
	DevDep      bool       `toml:"-"`
	PortConfig  PortConfig `toml:"-"`
	BuildType   string     `toml:"-"`
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
		if !fileio.PathExists(b.PortConfig.SrcDir) {
			// Clone repo.
			title := fmt.Sprintf("[clone %s]", b.PortConfig.nameVersionDesc())
			if err := git.CloneRepo(title, repoUrl, repoRef, b.PortConfig.SrcDir); err != nil {
				return err
			}
		}
	} else {
		// For archive repo, download it and extract to src dir event src dir not empty.
		archive = expr.If(archive == "", filepath.Base(repoUrl), archive)
		if !fileio.PathExists(filepath.Join(dirs.DownloadedDir, archive)) {
			// Create clean temp directory.
			if err := dirs.CleanTmpFilesDir(); err != nil {
				return fmt.Errorf("create clean tmp dir error: %w", err)
			}

			// Remove repor dir.
			if err := os.RemoveAll(b.PortConfig.RepoDir); err != nil {
				return fmt.Errorf("remove repo dir error: %w", err)
			}

			// Check and repair resource.
			repair := fileio.NewRepair(repoUrl, archive, ".", dirs.TmpFilesDir)
			if err := repair.CheckAndRepair(); err != nil {
				return err
			}

			// Move extracted files to source dir.
			entities, err := os.ReadDir(dirs.TmpFilesDir)
			if err != nil || len(entities) == 0 {
				return fmt.Errorf("cannot find extracted files under tmp dir")
			}
			if len(entities) == 1 {
				srcDir := filepath.Join(dirs.TmpFilesDir, entities[0].Name())
				if err := fileio.RenameDir(srcDir, b.PortConfig.RepoDir); err != nil {
					return err
				}
			} else if len(entities) > 1 {
				if err := fileio.RenameDir(dirs.TmpFilesDir, b.PortConfig.RepoDir); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func (b BuildConfig) CleanRepo() error {
	return b.buildSystem.CleanRepo()
}

func (b BuildConfig) Patch() error {
	if len(b.Patches) > 0 {
		// In windows, msys2 is required to apply patch .
		if runtime.GOOS == "windows" {
			if err := buildtools.CheckTools("msys2"); err != nil {
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
			return fmt.Errorf("cannot read port dir: %s", portDir)
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
						return fmt.Errorf("patch files error: %w", err)
					}
				}
			}
		}
		return nil
	}
	portDir := filepath.Join(dirs.PortsDir, b.PortConfig.LibName, b.PortConfig.LibVersion)
	if err := overrideFiles(portDir); err != nil {
		return fmt.Errorf("override files from port dir error: %w", err)
	}
	projectPortDir := filepath.Join(dirs.ConfProjectsDir, b.PortConfig.ProjectName, b.PortConfig.LibName, b.PortConfig.LibVersion)
	if err := overrideFiles(projectPortDir); err != nil {
		return fmt.Errorf("override files from project port dir error: %w", err)
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
		return fmt.Errorf("check tools for %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Replace placeholders with real value, like ${HOST}, ${SYSROOT} etc.
	b.fillPlaceHolders()

	// nobuild config do not have crosstool.
	if b.PortConfig.CrossTools != nil {
		b.setupEnvs()
		defer b.rollbackEnvs()

		// Create a symlink in the sysroot that points to the installed directory,
		// then the pc file would be found by other libraries.
		if b.PortConfig.CrossTools.RootFS != "" {
			// This symblink is used to find library via toolchain_file.cmake
			if err := b.checkSymlink(dirs.InstalledDir,
				filepath.Join(b.PortConfig.CrossTools.RootFS, "installed"),
			); err != nil {
				return err
			}

			// Create tmp dir in rootfs if not exist.
			rootfsTmp := filepath.Join(b.PortConfig.CrossTools.RootFS, "tmp")
			if err := os.MkdirAll(rootfsTmp, os.ModePerm); err != nil {
				return err
			}

			// This symblink is used to find library during build.
			if err := b.checkSymlink(dirs.TmpDepsDir,
				filepath.Join(b.PortConfig.CrossTools.RootFS, "tmp", "deps")); err != nil {
				return err
			}
		}
	}

	// Clone and patch related steps.
	if err := b.buildSystem.Clone(url, ref, archive); err != nil {
		operating := expr.If(strings.HasSuffix(url, ".git"), "clone", "download")
		return fmt.Errorf("%s %s: %w", operating, b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.Patch(); err != nil {
		return fmt.Errorf("patch %s: %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Configure related steps.
	if !b.buildSystem.configured() {
		if err := b.buildSystem.preConfigure(); err != nil {
			return fmt.Errorf("pre configure %s error: %w", b.PortConfig.nameVersionDesc(), err)
		}
		configureOptions, err := b.buildSystem.configureOptions()
		if err != nil {
			return fmt.Errorf("configure %s error: %w", b.PortConfig.nameVersionDesc(), err)
		}
		if err := b.buildSystem.Configure(configureOptions); err != nil {
			return fmt.Errorf("configure %s error: %w", b.PortConfig.nameVersionDesc(), err)
		}
		if err := b.buildSystem.postConfigure(); err != nil {
			return fmt.Errorf("post configure %s error: %w", b.PortConfig.nameVersionDesc(), err)
		}
	}

	// Build related steps.
	if err := b.buildSystem.preBuild(); err != nil {
		return fmt.Errorf("pre build %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}
	buildOptions, err := b.buildSystem.buildOptions()
	if err != nil {
		return fmt.Errorf("get build options %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.Build(buildOptions); err != nil {
		// Some third-party need extra steps to fix build. For example: nspr.
		if len(b.FixBuild) > 0 {
			if err := b.buildSystem.fixBuild(); err != nil {
				return fmt.Errorf("fix build %s error: %w", b.PortConfig.nameVersionDesc(), err)
			}
			if err := b.buildSystem.Build(buildOptions); err != nil {
				return fmt.Errorf("build %s again error: %w", b.PortConfig.nameVersionDesc(), err)
			}
		} else {
			return fmt.Errorf("build %s error: %w", b.PortConfig.nameVersionDesc(), err)
		}
	}
	if err := b.buildSystem.postBuild(); err != nil {
		return fmt.Errorf("post build %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Install related steps.
	if err := b.buildSystem.preInstall(); err != nil {
		return fmt.Errorf("pre install %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}
	installOptions, err := b.buildSystem.installOptions()
	if err != nil {
		return fmt.Errorf("get install options %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.Install(installOptions); err != nil {
		return fmt.Errorf("install %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}
	if err := b.buildSystem.postInstall(); err != nil {
		return fmt.Errorf("post install %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}

	// Fixup pkg config files.
	var prefix = expr.If(b.PortConfig.CrossTools.RootFS == "" || b.DevDep,
		filepath.Join(string(os.PathSeparator), "installed", b.PortConfig.HostName+"-dev"),
		filepath.Join(string(os.PathSeparator), "installed", b.PortConfig.LibraryFolder),
	)
	if err := fileio.FixupPkgConfig(b.PortConfig.PackageDir, prefix); err != nil {
		return fmt.Errorf("fixup pkg-config error: %w", err)
	}

	// Generate cmake configs.
	portDir := filepath.Join(dirs.PortsDir, b.PortConfig.LibName, b.PortConfig.LibVersion)
	preferedPortDir := filepath.Join(dirs.ConfProjectsDir, b.PortConfig.ProjectName, b.PortConfig.LibName, b.PortConfig.LibVersion)
	cmakeConfig, err := generator.FindMatchedConfig(portDir, preferedPortDir, b.PortConfig.CrossTools.SystemName, b.LibraryType)
	if err != nil {
		return fmt.Errorf("find matched config %s error: %w", b.PortConfig.nameVersionDesc(), err)
	}
	if cmakeConfig != nil {
		cmakeConfig.Version = b.PortConfig.LibVersion
		cmakeConfig.SystemName = b.PortConfig.CrossTools.SystemName
		cmakeConfig.Libname = b.PortConfig.LibName
		cmakeConfig.BuildType = b.BuildType
		if err := cmakeConfig.Generate(b.PortConfig.PackageDir); err != nil {
			return err
		}
	}

	return nil
}

func (b *BuildConfig) InitBuildSystem() error {
	if b.BuildSystem == "" {
		return fmt.Errorf("build_system is empty")
	}

	switch b.BuildSystem {
	case "nobuild":
		b.buildSystem = NewNoBuild(b)
	case "gyp":
		b.buildSystem = NewGyp(b)
	case "cmake":
		b.buildSystem = NewCMake(b, "")
	case "ninja":
		b.buildSystem = NewNinja(b)
	case "makefiles":
		b.buildSystem = NewMakefiles(b)
	case "meson":
		b.buildSystem = NewMeson(b)
	case "b2":
		b.buildSystem = NewB2(b)
	case "bazel":
		b.buildSystem = NewBazel(b)
	case "qmake":
		b.buildSystem = NewQMake(b)
	case "prebuilt":
		b.buildSystem = NewPrebuilt(b)
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
			return fmt.Errorf("compute relative path error: %w", err)
		}
		if err := os.Symlink(relPath, dest); err != nil {
			return fmt.Errorf("create symlink error: %w", err)
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
				b.Options[index] = strings.ReplaceAll(argument, "${HOST}", b.PortConfig.CrossTools.Host)
			}
		}

		if strings.Contains(argument, "${SYSTEM_NAME}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${SYSTEM_NAME}", strings.ToLower(b.PortConfig.CrossTools.SystemName))
			}
		}

		if strings.Contains(argument, "${SYSTEM_PROCESSOR}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${SYSTEM_PROCESSOR}", b.PortConfig.CrossTools.SystemProcessor)
			}
		}

		if strings.Contains(argument, "${SYSROOT}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${SYSROOT}", b.PortConfig.CrossTools.RootFS)
			}
		}

		if strings.Contains(argument, "${CROSSTOOL_PREFIX}") {
			if b.DevDep {
				b.Options = slices.Delete(b.Options, index, index+1)
			} else {
				b.Options[index] = strings.ReplaceAll(argument, "${CROSSTOOL_PREFIX}", b.PortConfig.CrossTools.CrosstoolPrefix)
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

func (b BuildConfig) setBuildType(buildType string) {
	isRelease := strings.ToLower(buildType) == "release"

	if b.PortConfig.CrossTools.Name == "msvc" {
		cl := strings.Split(os.Getenv("CL"), " ")
		cl = slices.DeleteFunc(cl, func(element string) bool {
			return strings.Contains(element, "/O")
		})

		if b.DevDep {
			cl = append(cl, "/O2")
			os.Setenv("CL", strings.Join(cl, " "))
		} else {
			flags := expr.If(isRelease, "/O2", "/Od")

			cl = append(cl, flags)
			os.Setenv("CL", strings.Join(cl, " "))
		}
	} else {
		cflags := strings.Split(os.Getenv("CFLAGS"), " ")
		cflags = slices.DeleteFunc(cflags, func(element string) bool {
			element = strings.TrimSpace(element)
			return element == "-g" || element == "-O"
		})

		cxxflags := strings.Split(os.Getenv("CXXFLAGS"), " ")
		cxxflags = slices.DeleteFunc(cxxflags, func(element string) bool {
			element = strings.TrimSpace(element)
			return element == "-g" || element == "-O"
		})

		if b.DevDep {
			cflags = append(cflags, "-O3")
			cxxflags = append(cxxflags, "-O3")
			os.Setenv("CFLAGS", strings.Join(cflags, " "))
			os.Setenv("CXXFLAGS", strings.Join(cxxflags, " "))
		} else {
			flags := expr.If(isRelease, "-O3", "-g")

			cflags = append(cflags, flags)
			cxxflags = append(cxxflags, flags)
			os.Setenv("CFLAGS", strings.Join(cflags, " "))
			os.Setenv("CXXFLAGS", strings.Join(cxxflags, " "))
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

	// Clean tmp directory.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		replaceFailed = true
		return fmt.Errorf("create clean tmp dir error: %w", err)
	}

	// Check and repair resource.
	archive = expr.If(archive == "", filepath.Base(url), archive)
	repair := fileio.NewRepair(url, archive, ".", dirs.TmpFilesDir)
	if err := repair.CheckAndRepair(); err != nil {
		replaceFailed = true
		return err
	}

	// Move extracted files to source dir.
	entities, err := os.ReadDir(dirs.TmpFilesDir)
	if err != nil || len(entities) == 0 {
		replaceFailed = true
		return fmt.Errorf("cannot find extracted files under tmp dir")
	}
	if len(entities) == 1 {
		srcDir := filepath.Join(dirs.TmpFilesDir, entities[0].Name())
		if err := fileio.RenameDir(srcDir, b.PortConfig.RepoDir); err != nil {
			replaceFailed = true
			return err
		}
	} else if len(entities) > 1 {
		if err := fileio.RenameDir(dirs.TmpFilesDir, b.PortConfig.RepoDir); err != nil {
			replaceFailed = true
			return err
		}
	}

	return nil
}

func (b BuildConfig) replaceHolders(content string) string {
	content = strings.ReplaceAll(content, "${SYSTEM_NAME}", b.PortConfig.CrossTools.SystemName)
	content = strings.ReplaceAll(content, "${HOST}", b.PortConfig.CrossTools.Host)
	content = strings.ReplaceAll(content, "${SYSTEM_PROCESSOR}", b.PortConfig.CrossTools.SystemProcessor)
	content = strings.ReplaceAll(content, "${SYSROOT}", b.PortConfig.CrossTools.RootFS)
	content = strings.ReplaceAll(content, "${CROSSTOOL_PREFIX}", b.PortConfig.CrossTools.CrosstoolPrefix)
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
func (b BuildConfig) msvcEnvs() string {
	var envs []string

	// Convert windows PATH to linux PATH.
	parts := strings.Split(os.Getenv("PATH"), ";")
	for index, path := range parts {
		parts[index] = fileio.ToCygpath(path)
	}
	envs = append(envs, fmt.Sprintf(`export PATH="%s:${PATH}"`, strings.Join(parts, ":")))

	// Provider envs if exist.
	var appendEnv = func(envKey string) {
		envValue := strings.TrimSpace(os.Getenv(envKey))
		if envValue != "" {
			envs = append(envs, fmt.Sprintf(`export %s="%s"`, envKey, envValue))
		}
	}
	appendEnv("PKG_CONFIG_PATH")
	appendEnv("PKG_CONFIG_SYSROOT_DIR")
	appendEnv("ACLOCAL_PATH")
	appendEnv("CFLAGS")
	appendEnv("CXXFLAGS")
	appendEnv("CPPFLAGS")
	appendEnv("LDFLAGS")
	appendEnv("INCLUDE")
	appendEnv("LIB")

	// Provider CC, CXX, LD, AR but no NM and others.
	envs = append(envs, fmt.Sprintf(`export CC="%s"`, b.PortConfig.CrossTools.CC))
	envs = append(envs, fmt.Sprintf(`export CXX="%s"`, b.PortConfig.CrossTools.CXX))
	envs = append(envs, fmt.Sprintf(`export LD="%s"`, b.PortConfig.CrossTools.LD))
	envs = append(envs, fmt.Sprintf(`export AR="%s"`, "ar-lib "+b.PortConfig.CrossTools.AR))

	return strings.Join(envs, " && ")
}
