package configs

import (
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

	if options.Force {
		// Remove installed port with its build cache, logs.
		remoteOptions := RemoveOptions{
			Purge:      true,
			Recursive:  options.Recursive,
			BuildCache: true,
		}
		if err := p.Remove(remoteOptions); err != nil {
			return "", err
		}

		// Clean source.
		if err := p.MatchedConfig.Clean(); err != nil {
			return "", err
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

		if err := os.MkdirAll(dirs.TmpDepsDir, os.ModePerm); err != nil {
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
		if installed, err := p.InstallFromBinaryCache(options); err != nil {
			return "", err
		} else if installed {
			return "binary cache", nil
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

func (p Port) doInstallFromBinaryCache(options InstallOptions) (bool, error) {
	// No cache dir configured, skip it.
	binaryCache := p.ctx.BinaryCache()
	if binaryCache == nil {
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
		return false, fmt.Errorf("failed to calculate buildhash.\n %w", err)
	}

	// Read cache file and extract them to package dir.
	if ok, err := binaryCache.Read(p.NameVersion(), buildhash+".tar.gz", p.MatchedConfig.PortConfig.PackageDir); err != nil {
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
		binaryCache := p.ctx.BinaryCache()
		if binaryCache == nil {
			return errors.ErrBinaryCacheDirNotConfigured
		}
		if binaryCache.GetDir() == "" {
			return errors.ErrBinaryCacheDirNotConfigured
		}

		if !fileio.PathExists(filepath.Join(binaryCache.GetDir(), ".token")) {
			return errors.ErrBinaryCacheTokenNotConfigured
		}

		if options.CacheToken == "" {
			return errors.ErrBinaryCacheTokenNotSpecified
		}

		if !encrypt.CheckToken(binaryCache.GetDir(), options.CacheToken) {
			return errors.ErrBinaryCacheTokenNotMatch
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
		if err := os.MkdirAll(filepath.Dir(metaFile), os.ModePerm); err != nil {
			installFailed = true
			return err
		}
		if err := os.WriteFile(metaFile, []byte(metaData), os.ModePerm); err != nil {
			installFailed = true
			return err
		}

		// Store cache after installation.
		if writeCacheAfterInstall {
			if p.ctx.BinaryCache() == nil {
				return errors.ErrBinaryCacheDirNotConfigured
			}
			binaryCache := p.ctx.BinaryCache()
			if binaryCache.GetDir() == "" {
				return errors.ErrBinaryCacheDirNotConfigured
			}
			if err := binaryCache.Write(p.MatchedConfig.PortConfig.PackageDir, metaData); err != nil {
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

		if err := os.MkdirAll(filepath.Dir(dest), os.ModePerm); err != nil {
			return err
		}

		if err := fileio.CopyFile(src, dest); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) InstallFromPackage(options InstallOptions) (bool, error) {
	// No package no install.
	if !fileio.PathExists(p.MatchedConfig.PortConfig.PackageDir) {
		return false, nil
	}

	// Install dependencies/dev_dependencies.
	if err := p.installDependencies(options); err != nil {
		return false, err
	}

	// Check if have meta file in package, no meta file means the package is invalid.
	var metaFile string
	entities, err := os.ReadDir(p.MatchedConfig.PortConfig.PackageDir)
	if err != nil {
		return false, fmt.Errorf("failed to read package dir.\n %w", err)
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
		return false, fmt.Errorf("failed to read package meta of %s.\n %w", p.NameVersion(), err)
	}
	newMeta, err := p.buildMeta(p.Package.Commit)
	if err != nil {
		return false, fmt.Errorf("failed to calculate meta of %s.\n %w", p.NameVersion(), err)
	}

	// Remove outdated package.
	var metaFileBackup string
	localMeta := string(metaBytes)
	if localMeta != newMeta {
		color.Printf(color.Warning, "\n================ The outdated package of %s will be removed now. ================\n", p.NameVersion())

		// Backup installed meta file to tmp dir.
		metaFileBackup = filepath.Join(dirs.TmpDir, filepath.Base(p.metaFile)+".old")

		// Ensure cleanup of backup if anything fails before it's moved.
		defer func() {
			if metaFileBackup != "" {
				os.Remove(metaFileBackup)
			}
		}()

		if err := os.MkdirAll(filepath.Dir(metaFileBackup), os.ModePerm); err != nil {
			return false, fmt.Errorf("failed to mkdir %s", filepath.Dir(metaFileBackup))
		}

		if err := fileio.CopyFile(p.metaFile, metaFileBackup); err != nil {
			return false, fmt.Errorf("failed to backup meta file.\n %w", err)
		}

		// Remove outdated package and install from source again.
		remoteOptions := RemoveOptions{
			Purge:      true,
			Recursive:  false,
			BuildCache: true,
		}
		if err := p.Remove(remoteOptions); err != nil {
			return false, fmt.Errorf("failed to remove outdated package.\n %w", err)
		}
		if err := p.doInstallFromSource(options); err != nil {
			return false, fmt.Errorf("failed to install from source.\n %w", err)
		}
	}

	if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
		return false, fmt.Errorf("failed to install from package.\n %w", err)
	}

	// Restore meta file for debuging.
	if metaFileBackup != "" {
		if err := os.Rename(metaFileBackup, p.metaFile+".old"); err != nil {
			return false, fmt.Errorf("failed to restore meta file.\n %w", err)
		}
		metaFileBackup = "" // Reset it indicates no need to clear it.
	}

	if err := p.writeTraceFile("package"); err != nil {
		return false, err
	}

	return true, nil
}

func (p Port) InstallFromBinaryCache(options InstallOptions) (bool, error) {
	installed, err := p.doInstallFromBinaryCache(options)
	if err != nil {
		return false, fmt.Errorf("failed to install from binary cache.\n %w", err)
	}

	if installed {
		// Install dependencies/dev_dependencies also.
		if err := p.installDependencies(options); err != nil {
			return false, err
		}

		if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
			return false, err
		}

		binaryCache := p.ctx.BinaryCache()
		if binaryCache == nil {
			return false, errors.ErrBinaryCacheDirNotConfigured
		}
		if binaryCache.GetDir() == "" {
			return false, errors.ErrBinaryCacheDirNotConfigured
		}
		fromDir := binaryCache.GetDir()
		return true, p.writeTraceFile(fmt.Sprintf("binary cache, dir: %q", fromDir))
	} else if p.Package.Commit != "" {
		return false, errors.ErrCacheNotFoundWithCommit
	}

	return false, nil
}

func (p Port) InstallFromSource(options InstallOptions) error {
	// Clone or download all required source.
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

	if err := p.installDependencies(options); err != nil {
		return err
	}

	// Prepare dependencies.
	if len(p.MatchedConfig.Dependencies) > 0 || len(p.MatchedConfig.DevDependencies) > 0 {
		color.Printf(color.Title, "\n[prepare dependencies for %s]:\n", p.NameVersion())
		preparedTmpDeps = []string{}
		if err := p.prepareTmpDeps(); err != nil {
			return err
		}
	}

	if err := p.doInstallFromSource(options); err != nil {
		return err
	}
	if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
		return err
	}

	return p.writeTraceFile("source")
}

func (p Port) installDependencies(options InstallOptions) error {
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

		// Install it if not installed or forcing with recursive.
		installed, err := port.Installed()
		if err != nil {
			return err
		}
		if !installed || (options.Force && options.Recursive) {
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

		// Install it if not installed or forcing with recursive.
		installed, err := port.Installed()
		if err != nil {
			return err
		}
		if !installed || (options.Force && options.Recursive) {
			if _, err := port.Install(options); err != nil {
				return err
			}
		}
	}

	// Delete CUDA packages directory and build cache immediately after installation to free disk space.
	// This is especially important for large files like CUDA toolkits in CI environments.
	if os.Getenv("GITHUB_ACTIONS") == "true" {
		cudaExtraPkgs := []string{
			"libcufft@", "libcurand@", "libcusolver@", "libcusparse@",
			"libnpp@", "libnvjpeg@", "nsight_compute@", "nsight_vse@",
			"nsight_systems@", "cuda_demo_suite@",
			"visual_studio_integration@",
		}
		for _, pkgName := range cudaExtraPkgs {
			if strings.Contains(p.Name, pkgName) || strings.Contains(p.Name, "cuda_") {
				// Delete package directory.
				if err := os.RemoveAll(p.PackageDir); err != nil {
					fmt.Printf("Warning: failed to delete CUDA package directory %s: %v\n", p.PackageDir, err)
				}
				// Delete build cache in buildtrees.
				if p.MatchedConfig != nil && p.MatchedConfig.PortConfig.RepoDir != "" {
					if err := os.RemoveAll(p.MatchedConfig.PortConfig.RepoDir); err != nil {
						fmt.Printf("Warning: failed to delete CUDA build cache %s: %v\n", p.MatchedConfig.PortConfig.RepoDir, err)
					}
				}
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
		// Use sysroot-relative path for pkg-config prefix so PKG_CONFIG_SYSROOT_DIR works correctly.
		var pkgConfigPrefix = filepath.Join(string(os.PathSeparator), "tmp", "deps", port.ctx.Platform().GetHostName()+"-dev")
		if err := fileio.FixupPkgConfig(port.tmpDepsDir, pkgConfigPrefix); err != nil {
			return fmt.Errorf("failed to fixup pkg-config.\n %w", err)
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
		pkgConfigPrefix := expr.If(port.DevDep || port.Native,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", port.ctx.Platform().GetHostName()+"-dev"),
			filepath.Join(string(os.PathSeparator), "tmp", "deps", port.MatchedConfig.PortConfig.LibraryFolder),
		)
		if err := fileio.FixupPkgConfig(port.tmpDepsDir, pkgConfigPrefix); err != nil {
			return fmt.Errorf("failed to fixup pkg-config.\n %w", err)
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
	if err := os.MkdirAll(filepath.Dir(p.traceFile), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create trace dir.\n %w", err)
	}
	packageFiles, err := p.PackageFiles(p.PackageDir, p.ctx.Platform().GetName(), p.ctx.Project().GetName())
	if err != nil {
		return fmt.Errorf("failed to get package files.\n %w", err)
	}
	if err := os.WriteFile(p.traceFile, []byte(strings.Join(packageFiles, "\n")), os.ModePerm); err != nil {
		return fmt.Errorf("failed to write trace file.\n %w", err)
	}

	// Print install trace.
	color.Printf(color.List, "\n[✔] -- package: %s is installed from %s\n", p.NameVersion(), installedFrom)
	color.Printf(color.Hint, "Location: %s\n", p.InstalledDir)
	return nil
}
