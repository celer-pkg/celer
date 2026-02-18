package configs

import (
	"celer/buildtools"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/encrypt"
	"celer/pkgs/errors"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

// Install install a port and tell me where it was installed from.
func (p *Port) Install(options InstallOptions) (string, error) {
	installedDir := expr.If(p.DevDep || p.Native,
		filepath.Join(dirs.InstalledDir, p.ctx.Platform().GetHostName()+"-dev"),
		filepath.Join(dirs.InstalledDir,
			p.ctx.Platform().GetName()+"@"+p.ctx.Project().GetName()+"@"+p.ctx.BuildType()),
	)

	// Check if installed already.
	installed, err := p.Installed()
	if err != nil {
		return "", err
	}

	// If already installed and not with "--force/-f", report and return.
	if installed && !options.Force {
		if p.IsHostSupported() {
			color.Printf(color.List, "\n[✔] -- package: %s\n", p.NameVersion())
			color.Printf(color.Hint, "Location: %s\n", installedDir)
		}
		return "", nil
	}

	// Check all tools at the beginning (only for top-level port)
	if p.Parent == "" {
		if err := p.checkAllTools(); err != nil {
			return "", err
		}
	}

	if options.Force {
		// Remove installed port with its build cache, logs.
		remoteOptions := RemoveOptions{
			Purge:      true,
			Recursive:  options.Recursive,
			BuildCache: true,
		}
		if err := p.Remove(remoteOptions); err != nil {
			return "", fmt.Errorf("failed to remove installed package -> %w", err)
		}

		// Clean source repo.
		if err := p.MatchedConfig.Clean(); err != nil {
			return "", fmt.Errorf("failed to clean repo before install -> %w", err)
		}
	}

	// Clear the tmp/deps dir, then copy library files of dependencies into it.
	// This ensures the folder contains exactly the libraries required by the current port.
	if p.Parent == "" {
		color.Printf(color.Title, "\n[clean tmps for %s]: %s\n", p.NameVersion(), dirs.TmpDepsDir)
		if err := os.RemoveAll(dirs.TmpDepsDir); err != nil {
			return "", err
		}
		color.Printf(color.Hint, "✔ rm -rf %s\n", dirs.TmpDepsDir)

		if err := fileio.MkdirAll(dirs.TmpDepsDir, os.ModePerm); err != nil {
			return "", err
		}
		color.Printf(color.Hint, "✔ mkdir -p %s\n", dirs.TmpDepsDir)
	}

	// No config or explicit prebuilt-with-url -> treat as nobuild or prebuilt.
	if len(p.BuildConfigs) == 0 ||
		(p.MatchedConfig.BuildSystem == "prebuilt" && p.MatchedConfig.Url != "") {
		if err := p.InstallFromSource(options); err != nil {
			return "", err
		}
		if len(p.BuildConfigs) == 0 {
			return "nobuild", nil
		}
		return "prebuilt", nil
	}

	// 1. Try to install from package.
	if installed, err := p.InstallFromPackage(options); err != nil {
		return "", err
	} else if installed {
		return "package", nil
	}

	// 2. Try to install from cache (only when not storing cache and not forcing).
	if !options.StoreCache && !options.Force {
		if installed, err := p.InstallFromPackageCache(options); err != nil {
			return "", err
		} else if installed {
			return "package cache", nil
		}
	}

	// 3. Fallback: install from source.
	if err := p.InstallFromSource(options); err != nil {
		return "", err
	}
	return "source", nil
}

func (p Port) Clone() error {
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		var port = Port{DevDep: true}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}
		if err := port.MatchedConfig.Clone(port.Package.Url, port.Package.Ref, port.Package.Archive, port.Package.Depth); err != nil {
			return err
		}
	}

	for _, nameVersion := range p.MatchedConfig.Dependencies {
		var port = Port{DevDep: false, Native: p.DevDep || p.Native}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}
		if err := port.MatchedConfig.Clone(port.Package.Url, port.Package.Ref, port.Package.Archive, port.Package.Depth); err != nil {
			return err
		}
	}

	// url with "_" means virtual port, no need to clone.
	if p.Package.Url != "_" {
		if err := p.MatchedConfig.Clone(p.Package.Url, p.Package.Ref, p.Package.Archive, p.Package.Depth); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) doInstallFromPackageCache(options InstallOptions) (bool, error) {
	// No cache dir configured, skip it.
	packageCache := p.ctx.PackageCache()
	if packageCache == nil {
		return false, nil
	}

	// Try to install dependencies first.
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		var port Port
		port.DevDep = false
		port.Native = p.DevDep || p.Native
		port.Parent = p.NameVersion()
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return false, err
		}
		if _, err := port.Install(options); err != nil {
			return false, err
		}
	}

	var port Port
	if err := port.Init(p.ctx, p.NameVersion()); err != nil {
		return false, err
	}

	// Calculate buildhash.
	buildhash, err := p.buildhash(p.Package.Commit)
	if err != nil {
		return false, fmt.Errorf("failed to calculate buildhash -> %w", err)
	}

	// Read cache file and extract them to package dir.
	if ok, err := packageCache.Read(p.NameVersion(), buildhash+".tar.gz", p.MatchedConfig.PortConfig.PackageDir); err != nil {
		return false, fmt.Errorf("read cache with buildhash: %s", err)
	} else if ok {
		return true, nil
	}

	return false, nil
}

func (p Port) doInstallFromSource(options InstallOptions) error {
	var installFailed bool
	defer func() {
		// Remove package dir if install failed.
		if installFailed {
			if err := os.RemoveAll(p.PackageDir); err != nil {
				fmt.Printf("remove broken package dir %s: %s\n", p.PackageDir, err)
			}
		}
	}()

	// Validate cache dir before building to avoid wasting build time.
	// Note: only store cache for non-devdep and non-native builds.
	var writeCacheAfterInstall bool
	if options.StoreCache && !p.MatchedConfig.DevDep && !p.MatchedConfig.Native {
		packageCache := p.ctx.PackageCache()
		if packageCache == nil {
			return errors.ErrPackageCacheDirNotConfigured
		}
		if packageCache.GetDir() == "" {
			return errors.ErrPackageCacheDirNotConfigured
		}

		if !fileio.PathExists(filepath.Join(packageCache.GetDir(), ".token")) {
			return errors.ErrPackageCacheTokenNotConfigured
		}

		if options.CacheToken == "" {
			return errors.ErrPackageCacheTokenNotSpecified
		}

		if !encrypt.CheckToken(packageCache.GetDir(), options.CacheToken) {
			return errors.ErrPackageCacheTokenNotMatch
		}

		writeCacheAfterInstall = true
	}

	if err := p.MatchedConfig.Install(p.Package.Url, p.Package.Ref, p.Package.Archive); err != nil {
		installFailed = true
		return err
	}

	// Generate meta file and store cache.
	buildSystem := p.MatchedConfig.BuildSystem
	if buildSystem != "nobuild" {
		// Skip meta file and cache for ports with url="_".
		// port with url="_" means no source repo and just in development.
		if p.Package.Url == "_" {
			color.Printf(color.Warning, "\n[!] ======== virtual project, skipping meta file generation and cache storing. ========\n")
			return nil
		}

		metaData, err := p.buildMeta(p.Package.Commit)
		if err != nil {
			installFailed = true
			return err
		}
		metaFile := filepath.Join(p.PackageDir, p.meta2hash(metaData)) + ".meta"
		if err := fileio.MkdirAll(filepath.Dir(metaFile), os.ModePerm); err != nil {
			installFailed = true
			return err
		}
		if err := os.WriteFile(metaFile, []byte(metaData), os.ModePerm); err != nil {
			installFailed = true
			return err
		}

		// Store cache after installation.
		if writeCacheAfterInstall {
			if p.ctx.PackageCache() == nil {
				return errors.ErrPackageCacheDirNotConfigured
			}
			packageCache := p.ctx.PackageCache()
			if packageCache.GetDir() == "" {
				return errors.ErrPackageCacheDirNotConfigured
			}
			if err := packageCache.Write(p.MatchedConfig.PortConfig.PackageDir, metaData); err != nil {
				return err
			}
		}
	}

	return nil
}

func (p Port) doInstallFromPackage(destDir string) error {
	// Check and repair current port.
	packageFiles, err := p.PackageFiles(
		p.PackageDir,
		p.ctx.Platform().GetName(),
		p.ctx.Project().GetName(),
	)
	if err != nil {
		return err
	}

	// Copy files from package to installed dir.
	platformProject := fmt.Sprintf("%s@%s@%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), p.ctx.BuildType())
	for _, file := range packageFiles {
		if p.DevDep || p.Native {
			file = strings.TrimPrefix(file, p.ctx.Platform().GetHostName()+"-dev"+string(os.PathSeparator))
		} else {
			file = strings.TrimPrefix(file, filepath.Join(platformProject, string(os.PathSeparator)))
		}

		src := filepath.Join(p.PackageDir, file)
		dest := filepath.Join(destDir, file)

		// Rename meta file as new name in meta folder.
		if strings.HasSuffix(file, ".meta") {
			dest = p.metaFile
		}

		// Ensure dest dir exists.
		if err := fileio.MkdirAll(filepath.Dir(dest), os.ModePerm); err != nil {
			return err
		}

		if err := fileio.CopyFile(src, dest); err != nil {
			return fmt.Errorf("failed to copy file.\n%w", err)
		}
	}

	return nil
}

func (p *Port) InstallFromPackage(options InstallOptions) (bool, error) {
	// No package no install.
	if !fileio.PathExists(p.MatchedConfig.PortConfig.PackageDir) {
		return false, nil
	}

	// Install dependencies/dev_dependencies.
	if err := p.installAllDeps(options); err != nil {
		return false, err
	}

	// Check if have meta file in package, no meta file means the package is invalid.
	var metaFile string
	entities, err := os.ReadDir(p.MatchedConfig.PortConfig.PackageDir)
	if err != nil {
		return false, fmt.Errorf("failed to read package dir -> %w", err)
	}
	for _, entity := range entities {
		if strings.HasSuffix(entity.Name(), ".meta") {
			metaFile = filepath.Join(p.MatchedConfig.PortConfig.PackageDir, entity.Name())
			break
		}
	}
	if metaFile == "" {
		suffix := expr.If(p.DevDep, " [dev]", "")
		return false, fmt.Errorf("invalid package %s, since meta file is not found for %s", p.PackageDir, p.NameVersion()+suffix)
	}

	// Install from package if meta matches.
	metaBytes, err := os.ReadFile(metaFile)
	if err != nil {
		return false, fmt.Errorf("failed to read package meta of %s -> %w", p.NameVersion(), err)
	}
	newMeta, err := p.buildMeta(p.Package.Commit)
	if err != nil {
		return false, fmt.Errorf("failed to calculate meta of %s -> %w", p.NameVersion(), err)
	}

	// Remove outdated package.
	var metaFileBackup string
	localMeta := string(metaBytes)
	if localMeta != newMeta {
		color.Printf(color.Warning, "\n================ The outdated package of %s will be removed now. ================\n", p.NameVersion())

		// Backup meta file to outdated dir.
		metaFileBackup = filepath.Join(dirs.InstalledDir, "celer", "meta", "outdated", filepath.Base(p.metaFile))
		if err := fileio.MkdirAll(filepath.Dir(metaFileBackup), os.ModePerm); err != nil {
			return false, fmt.Errorf("failed to mkdir %s", filepath.Dir(metaFileBackup))
		}
		if err := fileio.CopyFile(p.metaFile, metaFileBackup); err != nil {
			return false, fmt.Errorf("failed to backup meta file -> %w", err)
		}

		// Remove outdated package and install from source again.
		remoteOptions := RemoveOptions{
			Purge:      true,
			Recursive:  false,
			BuildCache: true,
		}
		if err := p.Remove(remoteOptions); err != nil {
			return false, fmt.Errorf("failed to remove outdated package -> %w", err)
		}
		if err := p.doInstallFromSource(options); err != nil {
			return false, fmt.Errorf("failed to install from source -> %w", err)
		}
	}

	if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
		return false, fmt.Errorf("failed to install from package -> %w", err)
	}

	if err := p.writeTraceFile("package"); err != nil {
		return false, err
	}

	return true, nil
}

func (p *Port) InstallFromPackageCache(options InstallOptions) (bool, error) {
	installed, err := p.doInstallFromPackageCache(options)
	if err != nil {
		// Repo not exist is not error.
		if errors.Is(err, errors.ErrRepoNotExit) {
			return false, nil
		}
		return false, fmt.Errorf("failed to install from package cache -> %w", err)
	}

	if installed {
		// Install dependencies/dev_dependencies also.
		if err := p.installAllDeps(options); err != nil {
			return false, err
		}

		if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
			return false, err
		}

		packageCache := p.ctx.PackageCache()
		if packageCache == nil {
			return false, errors.ErrPackageCacheDirNotConfigured
		}
		if packageCache.GetDir() == "" {
			return false, errors.ErrPackageCacheDirNotConfigured
		}
		fromDir := packageCache.GetDir()
		return true, p.writeTraceFile(fmt.Sprintf("package cache, dir: %q", fromDir))
	} else if p.Package.Commit != "" {
		return false, fmt.Errorf("%w: %s", errors.ErrCacheNotFoundWithCommit, p.Package.Commit)
	}

	return false, nil
}

func (p *Port) InstallFromSource(options InstallOptions) error {
	// Clone or download source of all ports.
	if err := p.cloneAllRepos(); err != nil {
		return err
	}

	// Check tools for all ports.
	if err := p.checkAllTools(); err != nil {
		return err
	}

	// Setup platform.
	if err := p.ctx.Platform().Setup(); err != nil {
		return err
	}

	// Install all dependencies for current port.
	if err := p.installAllDeps(options); err != nil {
		return err
	}

	// Prepare dependencies.
	if len(p.MatchedConfig.Dependencies) > 0 || len(p.MatchedConfig.DevDependencies) > 0 {
		color.Printf(color.Title, "\n[prepare deps for %s]:\n", p.NameVersion())
		preparedTmpDeps = []string{}
		if err := p.prepareTmpDeps(); err != nil {
			return err
		}
	}

	// Firstly, install to package dir.
	if err := p.doInstallFromSource(options); err != nil {
		return err
	}

	// Secondly, copy to installed dir.
	if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
		return err
	}

	return p.writeTraceFile("source")
}

func (p Port) cloneAllRepos() error {
	buildConfig := p.MatchedConfig
	for _, nameVersion := range buildConfig.DevDependencies {
		port := Port{DevDep: true}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}
		if err := port.Clone(); err != nil {
			return err
		}
	}
	for _, nameVersion := range buildConfig.Dependencies {
		port := Port{DevDep: false, Native: p.DevDep || p.Native}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}
		if err := port.Clone(); err != nil {
			return err
		}
	}
	if err := p.Clone(); err != nil {
		return err
	}

	return nil
}

func (p *Port) checkAllTools() error {
	var allTools []string

	buildConfig := p.MatchedConfig
	for _, nameVersion := range buildConfig.DevDependencies {
		port := Port{DevDep: true}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		allTools = append(allTools, port.MatchedConfig.CheckTools()...)
	}
	for _, nameVersion := range buildConfig.Dependencies {
		port := Port{DevDep: false, Native: p.DevDep || p.Native}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}
		allTools = append(allTools, port.MatchedConfig.CheckTools()...)
	}

	if p.ctx.CCacheEnabled() {
		allTools = append(allTools, "ccache")
	}

	allTools = append(allTools, p.MatchedConfig.CheckTools()...)

	// Validate tools exist and ensure tool paths are in PATH.
	if err := buildtools.CheckTools(p.ctx, allTools...); err != nil {
		return err
	}

	return nil
}

func (p Port) installAllDeps(options InstallOptions) error {
	// Check and repair dev_dependencies.
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (p.DevDep || p.Native) && p.NameVersion() == nameVersion {
			continue
		}

		// Init port.
		var port = Port{
			DevDep: true,
			Native: true,
			Parent: p.NameVersion(),
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Then install the dependency itself if needed.
		installed, err := port.Installed()
		if err != nil {
			return err
		}
		if !installed || (options.Force && options.Recursive) {
			// Always ensure sub-dependencies are installed first, even if the dependency itself is already installed.
			// This ensures transitive dependencies are always available before installing the dependency.
			if err := port.installAllDeps(options); err != nil {
				return err
			}

			if _, err := port.Install(options); err != nil {
				return err
			}
		}
	}

	// Check and repair dependencies.
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		name := strings.Split(nameVersion, "@")[0]
		if name == p.Name {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), name)
		}

		// Init port.
		var port = Port{
			DevDep: p.DevDep,
			Parent: p.NameVersion(),
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Then install the dependency itself if needed.
		installed, err := port.Installed()
		if err != nil {
			return err
		}
		if !installed || (options.Force && options.Recursive) {
			// Always ensure sub-dependencies are installed first.
			// This ensures transitive dependencies are always available before installing the dependency.
			if err := port.installAllDeps(options); err != nil {
				return err
			}

			if _, err := port.Install(options); err != nil {
				return err
			}
		}
	}

	return nil
}

func (p Port) prepareTmpDeps() error {
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (p.DevDep || p.Native) && p.NameVersion() == nameVersion {
			continue
		}

		// Ignore duplicated.
		if slices.Contains(preparedTmpDeps, nameVersion+" [dev]") {
			continue
		}

		// Init port.
		var port Port
		port.Parent = p.NameVersion()
		port.DevDep = true
		port.Native = true
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Copy package files to tmp/deps.
		if err := port.doInstallFromPackage(port.tmpDepsDir); err != nil {
			return err
		}

		// Fixup pkg config files.
		// Use absolute path for dev dependencies since native_file wrapper unsets PKG_CONFIG_SYSROOT_DIR
		// and this can also make sure system pc file can work right.
		var pkgConfigPrefix = filepath.Join(dirs.TmpDepsDir, port.ctx.Platform().GetHostName()+"-dev")
		if err := fileio.FixupPkgConfig(port.tmpDepsDir, pkgConfigPrefix); err != nil {
			return fmt.Errorf("failed to fixup pkg-config -> %w", err)
		}

		// Provider tmp deps recursively.
		preparedTmpDeps = append(preparedTmpDeps, nameVersion+" [dev]")
		if err := port.prepareTmpDeps(); err != nil {
			return err
		}

		color.Printf(color.Hint, "✔ %-15s -- [dev]\n", port.NameVersion())
	}

	for _, nameVersion := range p.MatchedConfig.Dependencies {
		name := strings.Split(nameVersion, "@")[0]
		if name == p.Name {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), nameVersion)
		}

		// Ignore duplicated.
		if slices.Contains(preparedTmpDeps, nameVersion+expr.If(p.DevDep || p.Native, " [dev]", "")) {
			continue
		}

		// Init port.
		var port Port
		port.DevDep = p.DevDep
		port.Native = p.DevDep || p.Native
		port.Parent = p.NameVersion()
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Copy package files to tmp/deps.
		if err := port.doInstallFromPackage(port.tmpDepsDir); err != nil {
			return err
		}

		// Fixup pkg config files.
		// Use absolute path for dev dependencies since native_file wrapper unsets PKG_CONFIG_SYSROOT_DIR
		// and this can also make sure system pc file can work right.
		pkgConfigPrefix := expr.If(port.DevDep || port.Native,
			port.tmpDepsDir,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", port.MatchedConfig.PortConfig.LibraryFolder),
		)
		if err := fileio.FixupPkgConfig(port.tmpDepsDir, pkgConfigPrefix); err != nil {
			return fmt.Errorf("failed to fixup pkg-config -> %w", err)
		}

		// Provider tmp deps recursively.
		preparedTmpDeps = append(preparedTmpDeps, nameVersion+expr.If(p.DevDep || p.Native, " [dev]", ""))
		if err := port.prepareTmpDeps(); err != nil {
			return err
		}

		content := expr.If(port.DevDep || port.Native, "✔ %-15s -- [dev]\n", "✔ %s\n")
		color.Printf(color.Hint, content, port.NameVersion())
	}

	return nil
}

func (p Port) writeTraceFile(installedFrom string) error {
	// Write installed files trace into its installation trace list.
	if err := fileio.MkdirAll(filepath.Dir(p.traceFile), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create trace dir -> %w", err)
	}
	packageFiles, err := p.PackageFiles(p.PackageDir, p.ctx.Platform().GetName(), p.ctx.Project().GetName())
	if err != nil {
		return fmt.Errorf("failed to get package files -> %w", err)
	}
	if err := os.WriteFile(p.traceFile, []byte(strings.Join(packageFiles, "\n")), os.ModePerm); err != nil {
		return fmt.Errorf("failed to write trace file -> %w", err)
	}

	// Print install trace.
	color.Printf(color.List, "\n[✔] -- package: %s is installed from %s\n", p.NameVersion(), installedFrom)
	color.Printf(color.Hint, "Location: %s\n", p.InstalledDir)
	return nil
}
