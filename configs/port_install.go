package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
	"github.com/celer-pkg/celer/pkgs/pc"
)

// Install install a port and tell me where it was installed from.
func (p *Port) Install(options InstallOptions) (installedFrom string, retErr error) {
	// At the top-level entry, reset the processedInstalls and installReport.
	if p.Parent == "" {
		processedInstalls = map[string]bool{}
		p.installReport = newInstallReport(p.NameVersion())
		processedInstalls = map[string]bool{}
	}
	defer func() {
		if retErr != nil || p.installReport == nil {
			return
		}

		finalFrom := installedFrom
		if finalFrom == "" {
			finalFrom = "preinstalled"
		}
		p.installReport.add(p, finalFrom)

		// Only top-level port writes report files.
		if p.Parent == "" {
			reportPath, err := p.installReport.write(p)
			if err != nil {
				color.PrintWarning("failed to write install report for %s: %s", p.NameVersion(), err)
				return
			}

			color.PrintPass("%s's install report is generated", p.NameVersion())
			color.PrintHint("Location: %s\n", reportPath)
		}
	}()

	installedDir := expr.If(p.DevDep || p.HostDep,
		filepath.Join(dirs.InstalledDir, p.ctx.Platform().GetHostName()+"-dev"),
		filepath.Join(dirs.InstalledDir,
			p.ctx.Platform().GetName()+"@"+p.ctx.Project().GetName()+"@"+p.ctx.BuildType()),
	)

	// Check if installed already.
	installed, err := p.Installed()
	if err != nil {
		return "", err
	}

	// Remvoe installed port when repo source changed.
	if p.sourceModified {
		options := RemoveOptions{
			Purge:      true,
			Recursive:  false,
			BuildCache: false,
		}
		if err := p.Remove(options); err != nil {
			return "", fmt.Errorf("failed to remove installed package -> %w", err)
		}
		installed = false // Mark as not installed.
	}

	// If preinstalled and not with "--force/-f", report and return.
	if installed && !options.Force {
		if p.IsHostSupported() {
			color.PrintPass("package: %s", p.NameVersion())
			color.PrintHint("Location: %s\n", installedDir)
		}
		installedFrom = "preinstalled"
		retErr = nil
		return
	}

	// Check all tools at the beginning (only for top-level port)
	if p.Parent == "" {
		if err := p.checkAllTools(); err != nil {
			return "", err
		}
	}

	// Force clear conditions:
	// 1. root port with --force
	// 2. children port with --force and -recursive
	forceClear := options.Force && (p.Parent == "" || (p.Parent != "" && options.Recursive))

	// Remove build cache and clean source repo when forcing install,
	if forceClear {
		// Remove installed port with its build cache, logs.
		options := RemoveOptions{
			Purge:      true,
			Recursive:  options.Recursive,
			BuildCache: true,
		}
		if err := p.Remove(options); err != nil {
			return "", fmt.Errorf("failed to remove installed package -> %w", err)
		}

		// Clean source repo.
		if err := p.MatchedConfig.Clean(); err != nil {
			return "", fmt.Errorf("failed to clean repo before install -> %w", err)
		}
	}

	// Clear the tmp/deps dir, then copy library files of dependencies into it.
	// This ensures the folder contains exactly the libraries required by the current port.
	// Clean tmp/deps dir only when install with -f or still not configured yet.
	if (options.Force || !p.MatchedConfig.Configured()) && p.Parent == "" {
		color.Printf(color.Title, "\n[clean tmps/deps: %s]\n", p.NameVersion())
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
			installedFrom = "nobuild"
			retErr = nil
			return
		}

		installedFrom = "prebuilt"
		retErr = nil
		return
	}

	// 1. Try to install from package.
	if installed, err := p.InstallFromPackage(options); err != nil {
		return "", err
	} else if installed {
		installedFrom = "package"
		retErr = nil
		return
	}

	// 2. Try to install from artifact package cache for target packages only.
	if !forceClear && !p.shouldSkipArtifactPkgCache() {
		if installed, err := p.InstallFromPkgCache(options); err != nil {
			return "", err
		} else if installed {
			installedFrom = "pkgcache"
			retErr = nil
			return
		}
	}

	// 3. Fallback: install from source.
	if err := p.InstallFromSource(options); err != nil {
		return "", err
	}

	installedFrom = "source"
	retErr = nil
	return
}

// shouldSkipArtifactPkgCache reports whether the artifact pkgcache must be
// bypassed for both restore and store. Dev/host builds use the local
// toolchain, which differs per machine; locally modified sources mean the
// developer is iterating and shouldn't see stale cache hits or pollute the
// cache for others.
func (p Port) shouldSkipArtifactPkgCache() bool {
	return p.DevDep || p.HostDep || p.sourceModified
}

// pkgCacheStoreSkipReason returns the reason the artifact pkgcache upload
// should be skipped after a source build. Empty string means upload is OK.
func (p *Port) pkgCacheStoreSkipReason() (string, error) {
	if p.MatchedConfig.DevDep || p.MatchedConfig.HostDev {
		return "host/dev port", nil
	}

	if p.ctx.Offline() {
		return "offline mode", nil
	}

	// Only write cache from a clean source tree before applying patches.
	// This keeps patch-applied dirty repos eligible, but skips developer-modified repos.
	if p.sourceModified {
		return "source repo is modified", nil
	}

	// Only repos that match the configured source ref can store package cache.
	if strings.HasSuffix(p.MatchedConfig.PortConfig.Url, ".git") {
		repoRef := expr.If(p.Package.Checksum != "", p.Package.Checksum, p.Package.Ref)
		mismatchDetails, err := git.CheckIfRefMatches(p.ctx, p.NameVersion(), p.MatchedConfig.PortConfig.RepoDir, repoRef)
		if err != nil {
			return "", fmt.Errorf("failed to check if ref matches for %s -> %w", p.NameVersion(), err)
		}
		if mismatchDetails != "" {
			return mismatchDetails, nil
		}
	}
	return "", nil
}

func (p Port) Clone() error {
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		var port = Port{
			DevDep: true,
			Parent: p.NameVersion(),
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Skip clone/reset for already-installed dependencies.
		if installed, _ := port.Installed(); installed {
			continue
		}

		// Clone repo is allowed only for third-party ports and public ports of project.
		if port.Package.Checksum == "" || port.IsThirdParty() {
			if err := port.MatchedConfig.Clone(
				port.Package.Url,
				port.Package.Ref,
				port.Package.Archive,
				port.Package.Depth,
			); err != nil {
				return err
			}
		}
	}

	for _, nameVersion := range p.MatchedConfig.Dependencies {
		var port = Port{
			DevDep:  false,
			HostDep: p.DevDep || p.HostDep,
			Parent:  p.NameVersion(),
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Skip clone/reset for already-installed dependencies.
		if installed, _ := port.Installed(); installed {
			continue
		}

		// Clone repo is allowed only for third-party ports and public ports of project.
		if port.Package.Checksum == "" || port.IsThirdParty() {
			if err := port.MatchedConfig.Clone(
				port.Package.Url,
				port.Package.Ref,
				port.Package.Archive,
				port.Package.Depth,
			); err != nil {
				return err
			}
		}
	}

	// url with "_" means virtual port, no need to clone.
	if p.Package.Url != "_" {
		// Repo may archived in pkgcache/repos with filename of commit hash,
		// we prefer clone with commit, so that repo can restore from pkgcache/repos.
		repoRef := expr.If(p.Package.Checksum != "", p.Package.Checksum, p.Package.Ref)
		if err := p.MatchedConfig.Clone(p.Package.Url, repoRef, p.Package.Archive, p.Package.Depth); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) doInstallFromPkgCache(options InstallOptions, artifactCache context.AritifactCache) (bool, error) {
	// Try to install dependencies first.
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		var port Port
		port.DevDep = false
		port.HostDep = p.HostDep
		port.Parent = p.NameVersion()
		port.installReport = p.installReport
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return false, err
		}
		if _, err := port.Install(options); err != nil {
			return false, err
		}
	}

	// Calculate buildhash.
	buildhash, err := p.buildhash()
	if err != nil {
		return false, fmt.Errorf("failed to calculate buildhash -> %w", err)
	}

	// Read cache file and extract them to package dir.
	if artifactCache != nil {
		if fromWhere, err := artifactCache.Restore(p.NameVersion(), buildhash, p.PackageDir); err != nil {
			return false, fmt.Errorf("read cache with buildhash: %s", err)
		} else if fromWhere != "" {
			return true, nil
		}
	}

	return false, nil
}

func (p *Port) doInstallFromSource() error {
	var installFailed bool
	defer func() {
		// Remove package dir if install failed.
		if installFailed {
			if err := os.RemoveAll(p.PackageDir); err != nil {
				fmt.Printf("remove broken package dir %s: %s\n", p.PackageDir, err)
			}
		}
	}()

	// Clean package directory.
	if err := os.RemoveAll(p.PackageDir); err != nil {
		installFailed = true
		return fmt.Errorf("failed to clean package dir %s -> %w", p.PackageDir, err)
	}

	// Check if need to store pkgcache and remember the skip reason.
	skipReason, err := p.pkgCacheStoreSkipReason()
	if err != nil {
		return err
	}
	p.pkgCacheStoreSkippedReason = skipReason

	// Call matched buildsystem to configure, build and install.
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
			color.Printf(color.Warning, "\n======== virtual project, skipping meta file generation and cache storing. ========\n")
			return nil
		}

		// Write meta file with installed files and build environment.
		metaData, err := p.buildMeta()
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

		// Store package cache with meta file inside.
		pkgCache := p.ctx.PkgCache()
		if pkgCache != nil && pkgCache.IsWritable() {
			if p.pkgCacheStoreSkippedReason == "" {
				artifactCache := pkgCache.GetArtifactCache()
				if artifactCache != nil {
					if err := artifactCache.Store(p.MatchedConfig.PortConfig.PackageDir, metaData); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func (p Port) doInstallFromPackage(destDir string) error {
	// Check and repair current port.
	files, err := p.PackageFiles(
		p.PackageDir,
		p.ctx.Platform().GetName(),
		p.ctx.Project().GetName(),
	)
	if err != nil {
		return err
	}

	// Copy files from package to installed dir.
	libraryFolder := filepath.Join(p.ctx.Platform().GetName(), p.ctx.Project().GetName(), p.ctx.BuildType())
	for _, file := range files {
		if p.DevDep || p.HostDep {
			file = strings.TrimPrefix(file, p.ctx.Platform().GetHostName()+"-dev"+string(os.PathSeparator))
		} else {
			file = strings.TrimPrefix(file, filepath.Join(libraryFolder, string(os.PathSeparator)))
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

	// Install dependencies.
	if err := p.installDependencies(options); err != nil {
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
	newMeta, err := p.buildMeta()
	if err != nil {
		return false, fmt.Errorf("failed to calculate meta of %s -> %w", p.NameVersion(), err)
	}

	// Remove outdated package.
	localMeta := string(metaBytes)
	if localMeta != newMeta {
		color.Printf(color.Warning, "\n================ The outdated package of %s will be removed now. ================\n", p.NameVersion())

		// Backup current installed meta file if it exists.
		if fileio.PathExists(p.metaFile) {
			metaFileBackup := filepath.Join(dirs.InstalledDir, "celer", "metas", "outdated", filepath.Base(p.metaFile))
			if err := fileio.MkdirAll(filepath.Dir(metaFileBackup), os.ModePerm); err != nil {
				return false, fmt.Errorf("failed to mkdir %s", filepath.Dir(metaFileBackup))
			}
			if err := fileio.CopyFile(p.metaFile, metaFileBackup); err != nil {
				return false, fmt.Errorf("failed to backup meta file -> %w", err)
			}
		} else {
			color.Printf(color.Warning, "installed meta file not found, skip backup: %s\n", p.metaFile)
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
		if err := p.doInstallFromSource(); err != nil {
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

func (p *Port) InstallFromPkgCache(options InstallOptions) (bool, error) {
	// Check if pkgCache has been configured.
	pkgCache := p.ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetDir(context.PkgCacheDirRoot) == "" {
		return false, nil
	}

	installed, err := p.doInstallFromPkgCache(options, pkgCache.GetArtifactCache())
	if err != nil {
		// Repo not exist is not error.
		if errors.Is(err, errors.ErrRepoNotExit) {
			return false, nil
		}
		return false, err
	}

	if installed {
		// Install dependencies also.
		if err := p.installDependencies(options); err != nil {
			return false, err
		}

		if err := p.doInstallFromPackage(p.InstalledDir); err != nil {
			return false, err
		}

		fromDir := pkgCache.GetDir(context.PkgCacheDirRoot)
		return true, p.writeTraceFile(fmt.Sprintf("pkgcache: %q", fromDir))
	}

	return false, nil
}

func (p *Port) InstallFromSource(options InstallOptions) error {
	// Clone or download source of all repos.
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
	if err := p.installAllDependencies(options); err != nil {
		return err
	}

	// Prepare dependencies to tmp/deps before build it, below are conditions:
	// 1. It's a dependency project and it has its own dependencies.
	// 2. It's a top project, build with --force, and it has its own dependencies.
	// 3. It's a top project, it has its own dependencies but is not configured yet, even not build with --force.
	haveDependencies := len(p.MatchedConfig.Dependencies) > 0 || len(p.MatchedConfig.DevDependencies) > 0
	isTopProject := p.Parent == ""
	if haveDependencies && (!isTopProject || (isTopProject && (options.Force || !p.MatchedConfig.Configured()))) {
		color.Printf(color.Title, "\n[prepare dependencies: %s]\n", p.NameVersion())
		preparedTmpDeps = []string{}
		if err := p.prepareTmpDeps(); err != nil {
			return err
		}
	}

	// Firstly, install to package dir.
	if err := p.doInstallFromSource(); err != nil {
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
		port := Port{
			DevDep: true,
			Parent: p.NameVersion(),
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Skip clone/reset for already-installed dependencies.
		if installed, _ := port.Installed(); installed {
			continue
		}

		// Clone repo is allowed only for third-party ports and public ports of project.
		if port.Package.Checksum == "" || port.IsThirdParty() {
			if err := port.Clone(); err != nil {
				return err
			}
		}
	}
	for _, nameVersion := range buildConfig.Dependencies {
		port := Port{
			DevDep:  false,
			HostDep: p.DevDep || p.HostDep,
			Parent:  p.NameVersion(),
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Skip clone/reset for already-installed dependencies.
		if installed, _ := port.Installed(); installed {
			continue
		}

		// Clone repo is allowed only for third-party ports and public ports of project.
		if port.Package.Checksum == "" || port.IsThirdParty() {
			if err := port.Clone(); err != nil {
				return err
			}
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
		port := Port{DevDep: false, HostDep: p.DevDep || p.HostDep}
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

	// Refresh dynamic expression variables after tool detection because some
	// ports rely on build_tools-provided paths during option expansion.
	if exprVars := p.ctx.ExprVars(); exprVars != nil {
		if buildtools.PythonTool != nil && buildtools.PythonTool.Path != "" {
			exprVars.Put("PYTHON3_PATH", fileio.ToRelPath(buildtools.PythonTool.Path))
		}
		if buildtools.LLVMPath != "" {
			llvmConfig := expr.If(runtime.GOOS == "windows", "llvm-config.exe", "llvm-config")
			llvmRoot := fileio.ToRelPath(buildtools.LLVMPath)
			exprVars.Put("LLVM_CONFIG", filepath.ToSlash(filepath.Join(llvmRoot, "bin", llvmConfig)))
		}
	}

	if p.MatchedConfig != nil {
		p.putExprVars(*p.MatchedConfig)
		p.MatchedConfig.ExprVars = p.exprVars
	}

	return nil
}

func (p Port) installAllDependencies(options InstallOptions) error {
	if err := p.installDevDependencies(options); err != nil {
		return err
	}
	if err := p.installDependencies(options); err != nil {
		return err
	}
	return nil
}

func (p Port) installDependencies(options InstallOptions) error {
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		name, _, _ := strings.Cut(nameVersion, "@")
		if name == p.Name {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), name)
		}

		// Init port.
		var port = Port{
			DevDep:        p.DevDep,
			Parent:        p.NameVersion(),
			installReport: p.installReport,
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Then install the dependency itself if needed.
		installed, err := port.Installed()
		if err != nil {
			return err
		}

		// With --force --recursive, ports get reinstalled even if already
		// present, but each port still only needs to be reinstalled once per
		// top-level command — guard against the same port appearing under many parents.
		key := port.processedKey()
		_, alreadyProcessed := processedInstalls[key]
		if !installed || (options.Force && options.Recursive && !alreadyProcessed) {
			// Always ensure sub-dependencies are installed first.
			// This ensures transitive dependencies are always available before installing the dependency.
			if err := port.installAllDependencies(options); err != nil {
				return err
			}

			if _, err := port.Install(options); err != nil {
				return err
			}
			processedInstalls[key] = true
		} else if p.installReport != nil {
			p.installReport.add(&port, "preinstalled")
			if err := port.collectInstalledDepsForReport(); err != nil {
				return err
			}
		}
	}

	return nil
}

func (p Port) installDevDependencies(options InstallOptions) error {
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (p.DevDep || p.HostDep) && p.NameVersion() == nameVersion {
			continue
		}

		// Init port.
		var port = Port{
			DevDep:        true,
			HostDep:       true,
			Parent:        p.NameVersion(),
			installReport: p.installReport,
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		// Then install the dependency itself if needed.
		installed, err := port.Installed()
		if err != nil {
			return err
		}

		// With --force --recursive, ports get reinstalled even if already
		// present, but each port still only needs to be reinstalled once per
		// top-level command — guard against the same port appearing under many parents.
		key := port.processedKey()
		_, alreadyProcessed := processedInstalls[key]
		if !installed || (options.Force && options.Recursive && !alreadyProcessed) {
			// Always ensure sub-dependencies are installed first, even if the dependency itself is preinstalled.
			// This ensures transitive dependencies are always available before installing the dependency.
			if err := port.installAllDependencies(options); err != nil {
				return err
			}

			if _, err := port.Install(options); err != nil {
				return err
			}
			processedInstalls[key] = true
		} else if p.installReport != nil {
			p.installReport.add(&port, "preinstalled")
			if err := port.collectInstalledDepsForReport(); err != nil {
				return err
			}
		}
	}

	return nil
}

// collectInstalledDepsForReport recursively collects dependency entries into install report
// when a dependency is preinstalled and we skip real installation.
func (p Port) collectInstalledDepsForReport() error {
	if p.installReport == nil {
		return nil
	}

	// Collect dev_dependencies.
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Same name/version as parent and both are in native toolchain chain, skip.
		if (p.DevDep || p.HostDep) && p.NameVersion() == nameVersion {
			continue
		}

		port := Port{
			DevDep:        true,
			HostDep:       true,
			Parent:        p.NameVersion(),
			installReport: p.installReport,
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		p.installReport.add(&port, "preinstalled")
		if err := port.collectInstalledDepsForReport(); err != nil {
			return err
		}
	}

	// Collect dependencies.
	for _, nameVersion := range p.MatchedConfig.Dependencies {
		name := strings.Split(nameVersion, "@")[0]
		if name == p.Name {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), name)
		}

		port := Port{
			DevDep:        p.DevDep,
			Parent:        p.NameVersion(),
			installReport: p.installReport,
		}
		if err := port.Init(p.ctx, nameVersion); err != nil {
			return err
		}

		p.installReport.add(&port, "preinstalled")
		if err := port.collectInstalledDepsForReport(); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) prepareTmpDeps() error {
	for _, nameVersion := range p.MatchedConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (p.DevDep || p.HostDep) && p.NameVersion() == nameVersion {
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
		port.HostDep = true
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
		var pkgConfig pc.PkgConfig
		if err := pkgConfig.Apply(port.tmpDepsDir, pkgConfigPrefix); err != nil {
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
		name, _, _ := strings.Cut(nameVersion, "@")
		if name == p.Name {
			return fmt.Errorf("%s's dependencies contains circular dependency: %s", p.NameVersion(), nameVersion)
		}

		// Ignore duplicated.
		if slices.Contains(preparedTmpDeps, nameVersion+expr.If(p.DevDep || p.HostDep, " [dev]", "")) {
			continue
		}

		// Init port.
		var port Port
		port.DevDep = p.DevDep
		port.HostDep = p.DevDep || p.HostDep
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
		pkgConfigPrefix := expr.If(port.DevDep || port.HostDep,
			port.tmpDepsDir,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", port.MatchedConfig.PortConfig.LibraryDir),
		)
		var pkgConfig pc.PkgConfig
		if err := pkgConfig.Apply(port.tmpDepsDir, pkgConfigPrefix); err != nil {
			return fmt.Errorf("failed to fixup pkg-config -> %w", err)
		}

		// Provider tmp deps recursively.
		preparedTmpDeps = append(preparedTmpDeps, nameVersion+expr.If(p.DevDep || p.HostDep, " [dev]", ""))
		if err := port.prepareTmpDeps(); err != nil {
			return err
		}

		content := expr.If(port.DevDep || port.HostDep, "✔ %-15s -- [dev]\n", "✔ %s\n")
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
	color.PrintPass("%s is installed from %s", p.NameVersion(), installedFrom)
	color.PrintHint("Location: %s\n", p.InstalledDir)

	// Print reason why skip store artifact to pkgcache.
	if p.pkgCacheStoreSkippedReason != "" {
		color.PrintWarning("skip storing package cache for %s because %s\n", p.NameVersion(), p.pkgCacheStoreSkippedReason)
	}

	return nil
}
