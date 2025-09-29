package configs

import (
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

// Install install a port and tell me where it was installed from.
func (p Port) Install() (string, error) {
	installedDir := expr.If(p.DevDep,
		filepath.Join(dirs.InstalledDir, p.ctx.Platform().HostName()+"-dev"),
		filepath.Join(dirs.InstalledDir,
			p.ctx.Platform().Name+"@"+p.ctx.Project().Name+"@"+p.buildType),
	)

	// Check if installed already.
	installed, err := p.Installed()
	if err != nil {
		return "", err
	}

	if installed {
		if p.Reinstall {
			if err := p.Remove(p.Recurse, true, true); err != nil {
				return "", err
			}
		} else {
			// Don't show installed info when building in host is not supported.
			if p.IsHostSupported() {
				title := color.Sprintf(color.Green, "\n[✔] ---- package: %s\n", p.NameVersion())
				fmt.Printf("%sLocation: %s\n", title, installedDir)
			}
			return "", nil
		}
	} else {
		// Remove build cache and logs.
		if p.Reinstall {
			if err := os.RemoveAll(p.MatchedConfig.PortConfig.BuildDir); err != nil {
				return "", fmt.Errorf("remove build cache error: %w", err)
			}

			if err := p.RemoveLogs(); err != nil {
				return "", fmt.Errorf("remove logs error: %w", err)
			}
		}
	}

	// Clear the tmp/deps dir, then copy only the needed library files into it.
	// This ensures the folder contains exactly the libraries required by the current port.
	if p.Parent == "" {
		if err := os.RemoveAll(dirs.TmpDepsDir); err != nil {
			return "", err
		}
		if err := os.MkdirAll(dirs.TmpDepsDir, os.ModePerm); err != nil {
			return "", err
		}
	}

	// No config found, install as prebuilt.
	if len(p.BuildConfigs) == 0 ||
		(p.MatchedConfig.BuildSystem == "prebuilt" && p.MatchedConfig.Url != "") {
		if err := p.installFromSource(); err != nil {
			return "", err
		}
		if len(p.BuildConfigs) == 0 {
			return "source", nil
		}
		return "prebuilt", nil
	}

	// 1. try to install from package.
	if installed, err := p.installFromPackage(); err != nil {
		return "", err
	} else if installed {
		return "package", nil
	}

	// 2. try to install from cache.
	if !p.StoreCache && !p.Reinstall {
		if installed, err := p.installFromCache(); err != nil {
			return "", err
		} else if installed {
			return "cache", nil
		}
	}

	// 3. try to install from source.
	if err := p.installFromSource(); err != nil {
		return "", err
	}

	return "source", nil
}

func (p Port) doInstallFromCache() (bool, error) {
	// No cache dir configured, skip it.
	if p.ctx.CacheDir() == nil {
		return false, nil
	}

	// Try to install dependencies first.
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		var port Port
		port.DevDep = p.DevDep
		port.Parent = p.NameVersion()
		if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
			return false, err
		}
		if _, err := port.Install(); err != nil {
			return false, err
		}
	}

	var port Port
	if err := port.Init(p.ctx, p.NameVersion(), p.buildType); err != nil {
		return false, err
	}

	// Calculate buildhash.
	buildhash, err := p.buildhash(p.Package.Commit)
	if err != nil {
		return false, fmt.Errorf("calculate buildhash error: %w", err)
	}

	// Read cache file and extract them to package dir.
	if ok, err := p.ctx.CacheDir().Read(
		p.ctx.Platform().Name,
		p.ctx.Project().Name,
		p.buildType,
		p.NameVersion(),
		buildhash+".tar.gz",
		p.MatchedConfig.PortConfig.PackageDir,
	); err != nil {
		return false, fmt.Errorf("read cache with buildhash: %s", err)
	} else if ok {
		return true, nil
	}

	return false, nil
}

func (p Port) doInstallFromSource() error {
	var installFailed bool
	defer func() {
		// Remove package dir if install failed.
		if installFailed {
			if err := os.RemoveAll(p.packageDir); err != nil {
				fmt.Printf("remove broken package dir %s: %s\n", p.packageDir, err)
			}
		}
	}()

	var writeCacheAfterInstall bool
	cacheDir := p.ctx.CacheDir()
	if p.StoreCache {
		if cacheDir == nil || cacheDir.Dir == "" {
			return ErrCacheDirNotConfigured
		}

		if cacheDir.Token == "" {
			return ErrCacheTokenNotConfigured
		}

		if p.CacheToken == "" {
			return ErrCacheTokenNotSpecified
		}

		if p.CacheToken != cacheDir.Token {
			return ErrCacheTokenNotMatch
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
		metaData, err := p.buildMeta(p.Package.Commit)
		if err != nil {
			installFailed = true
			return err
		}
		metaFile := filepath.Join(p.packageDir, p.meta2hash(metaData)) + ".meta"
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
			if err := cacheDir.Write(p.MatchedConfig.PortConfig.PackageDir, metaData); err != nil {
				return err
			}
		}
	}

	return nil
}

func (p Port) doInstallFromPackage(destDir string) error {
	// Check and repair current port.
	packageFiles, err := p.PackageFiles(
		p.packageDir,
		p.ctx.Platform().Name,
		p.ctx.Project().Name,
	)
	if err != nil {
		return err
	}

	// Copy files from package to installed dir.
	platformProject := fmt.Sprintf("%s@%s@%s", p.ctx.Platform().Name, p.ctx.Project().Name, p.buildType)
	for _, file := range packageFiles {
		if p.DevDep {
			file = strings.TrimPrefix(file, p.ctx.Platform().HostName()+"-dev"+string(os.PathSeparator))
		} else {
			file = strings.TrimPrefix(file, filepath.Join(platformProject, string(os.PathSeparator)))
		}

		src := filepath.Join(p.packageDir, file)
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

func (p Port) installFromPackage() (bool, error) {
	// No package no install.
	if !fileio.PathExists(p.MatchedConfig.PortConfig.PackageDir) {
		return false, nil
	}

	// Install dependencies/dev_dependencies.
	if err := p.installDependencies(); err != nil {
		return false, err
	}

	// Check if have meta file in package, no meta file means the package is invalid.
	var metaFile string
	entities, err := os.ReadDir(p.MatchedConfig.PortConfig.PackageDir)
	if err != nil {
		return false, fmt.Errorf("read package dir error: %w", err)
	}
	for _, entity := range entities {
		if strings.HasSuffix(entity.Name(), ".meta") {
			metaFile = filepath.Join(p.MatchedConfig.PortConfig.PackageDir, entity.Name())
			break
		}
	}
	if metaFile == "" {
		suffix := expr.If(p.DevDep, "@dev", "")
		return false, fmt.Errorf("invalid package %s, since meta file is not found for %s", p.packageDir, p.NameVersion()+suffix)
	}

	// Install from package if meta matches.
	metaBytes, err := os.ReadFile(metaFile)
	if err != nil {
		return false, fmt.Errorf("read package meta of %s error: %w", p.NameVersion(), err)
	}
	newMeta, err := p.buildMeta(p.Package.Commit)
	if err != nil {
		return false, fmt.Errorf("calculate meta of %s error: %w", p.NameVersion(), err)
	}

	// Remove outdated package.
	var metaFileBackup string
	localMeta := string(metaBytes)
	if localMeta != newMeta {
		color.Printf(color.Yellow, "\n================ The outdated package of %s will be removed now. ================", p.NameVersion())

		// Backup installed meta file to tmp dir.
		metaFileBackup = filepath.Join(dirs.TmpDir, filepath.Base(p.metaFile)+".old")
		if err := fileio.CopyFile(p.metaFile, metaFileBackup); err != nil {
			return false, fmt.Errorf("backup meta file error: %w", err)
		}

		// Remove outdated package and install from source again.
		if err := p.Remove(false, true, true); err != nil {
			return false, fmt.Errorf("remove outdated package error: %w", err)
		}
		if err := p.doInstallFromSource(); err != nil {
			return false, fmt.Errorf("install from package error: %w", err)
		}
	}

	if err := p.doInstallFromPackage(p.installedDir); err != nil {
		return false, fmt.Errorf("install from package error: %w", err)
	}

	// Restore meta file for compare difference when debug.
	if metaFileBackup != "" {
		if err := os.Rename(metaFileBackup, p.metaFile+".old"); err != nil {
			return false, fmt.Errorf("restore meta file error: %w", err)
		}
	}

	if err := p.writeTraceFile("package"); err != nil {
		return false, err
	}

	return true, nil
}

func (p Port) installFromCache() (bool, error) {
	installed, err := p.doInstallFromCache()
	if err != nil {
		return false, fmt.Errorf("install from cache error: %w", err)
	}

	if installed {
		// Install dependencies/dev_dependencies also.
		if err := p.installDependencies(); err != nil {
			return false, err
		}

		if err := p.doInstallFromPackage(p.installedDir); err != nil {
			return false, err
		}

		fromDir := p.ctx.CacheDir().Dir
		return true, p.writeTraceFile(fmt.Sprintf("cache [%s]", fromDir))
	} else if p.Package.Commit != "" {
		return false, ErrCacheNotFoundWithCommit
	}

	return false, nil
}

func (p Port) installFromSource() error {
	if err := p.installDependencies(); err != nil {
		return err
	}

	// Prepare build dependencies.
	if len(p.MatchedConfig.Dependencies) > 0 || len(p.MatchedConfig.DevDependencies) > 0 {
		color.Printf(color.Cyan, "[preparing build [dev_]dependencies for %s]:\n", p.NameVersion())
		preparedTmpDeps = []string{}
		if err := p.providerTmpDeps(); err != nil {
			return err
		}
	}

	if err := p.doInstallFromSource(); err != nil {
		return err
	}
	if err := p.doInstallFromPackage(p.installedDir); err != nil {
		return err
	}

	return p.writeTraceFile("source")
}

func (p Port) installDependencies() error {
	// Check and repair dev_dependencies.
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Skip self.
		if p.DevDep && p.Native && p.NameVersion() == nameVersion {
			continue
		}

		// Check and repair dependency.
		var port Port
		port.Parent = p.NameVersion()
		port.DevDep = true
		port.Native = true
		if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
			return err
		}
		if _, err := port.Install(); err != nil {
			return err
		}
	}

	// Check and repair dependencies.
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		if strings.HasPrefix(nameVersion, p.Name) {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), nameVersion)
		}

		// Check and repair dependency.
		var port Port
		port.DevDep = p.DevDep
		port.Parent = p.NameVersion()
		if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
			return err
		}
		if _, err := port.Install(); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) providerTmpDeps() error {
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Skip self.
		if p.DevDep && p.NameVersion() == nameVersion {
			continue
		}

		// Ignore duplicated.
		if slices.Contains(preparedTmpDeps, nameVersion+"@dev") {
			continue
		}

		// Init port.
		var port Port
		port.Parent = p.NameVersion()
		port.DevDep = true
		if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
			return err
		}

		// Copy package files to tmp/deps.
		if err := port.doInstallFromPackage(port.tmpDepsDir); err != nil {
			return err
		}

		// Fixup pkg config files.
		var prefix = expr.If(p.toolchain().RootFS == "" || p.DevDep,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.ctx.Platform().HostName()+"-dev"),
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.MatchedConfig.PortConfig.LibraryFolder),
		)
		if err := fileio.FixupPkgConfig(p.packageDir, prefix); err != nil {
			return fmt.Errorf("fixup pkg-config error: %w", err)
		}

		// Provider tmp deps recursively.
		preparedTmpDeps = append(preparedTmpDeps, nameVersion+"@dev")
		if err := port.providerTmpDeps(); err != nil {
			return err
		}

		color.Printf(color.Gray, "✔ %-15s -- [dev]\n", port.NameVersion())
	}

	for _, nameVersion := range p.MatchedConfig.Dependencies {
		if strings.HasPrefix(nameVersion, p.Name) {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), nameVersion)
		}

		// Ignore duplicated.
		if slices.Contains(preparedTmpDeps, nameVersion+expr.If(p.DevDep || p.Native, "@dev", "")) {
			continue
		}

		// Init port.
		var port Port
		port.DevDep = p.DevDep
		port.Parent = p.NameVersion()
		if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
			return err
		}

		// Copy package files to tmp/deps.
		if err := port.doInstallFromPackage(port.tmpDepsDir); err != nil {
			return err
		}

		// Fixup pkg config files.
		var prefix = expr.If(p.toolchain().RootFS == "" || p.DevDep,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.ctx.Platform().HostName()+"-dev"),
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.MatchedConfig.PortConfig.LibraryFolder),
		)
		if err := fileio.FixupPkgConfig(port.tmpDepsDir, prefix); err != nil {
			return fmt.Errorf("fixup pkg-config error: %w", err)
		}

		// Provider tmp deps recursively.
		preparedTmpDeps = append(preparedTmpDeps, nameVersion+expr.If(p.DevDep || p.Native, "@dev", ""))
		if err := port.providerTmpDeps(); err != nil {
			return err
		}

		content := expr.If(port.DevDep, "✔ %-15s -- [dev]\n", "✔ %s\n")
		color.Printf(color.Gray, content, port.NameVersion())
	}

	return nil
}

func (p Port) writeTraceFile(installedFrom string) error {
	// Write installed files trace into its installation trace list.
	if err := os.MkdirAll(filepath.Dir(p.traceFile), os.ModePerm); err != nil {
		return fmt.Errorf("create trace dir error: %w", err)
	}
	packageFiles, err := p.PackageFiles(p.packageDir, p.ctx.Platform().Name, p.ctx.Project().Name)
	if err != nil {
		return fmt.Errorf("get package files error: %w", err)
	}
	if err := os.WriteFile(p.traceFile, []byte(strings.Join(packageFiles, "\n")), os.ModePerm); err != nil {
		return fmt.Errorf("write trace file error: %w", err)
	}

	// Print install trace.
	title := color.Sprintf(color.Green, "\n[✔] ---- package: %s is installed from %s\n",
		p.NameVersion(), installedFrom)
	fmt.Printf("%sLocation: %s\n", title, p.installedDir)
	return nil
}
