package configs

import (
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
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
		// Don't show installed info when building in host is not supported.
		if !DevMode && p.IsHostSupported() {
			title := color.Sprintf(color.Green, "\n[✔] ---- package: %s\n", p.NameVersion())
			fmt.Printf("%sLocation: %s\n", title, installedDir)
		}
		return "", nil
	}

	// Clear the tmp/deps dir, then copy only the needed library files into it.
	// This ensures the folder contains exactly the libraries required for building.
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
	if installed, err := p.installFromCache(); err != nil {
		return "", err
	} else if installed {
		return "cache", nil
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

	// For downloaded ports, check if the archive file exists, if not exists, skip it.
	if !strings.HasSuffix(port.Package.Url, ".git") {
		fileName := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
		filePath := filepath.Join(dirs.DownloadedDir, fileName)
		if !fileio.PathExists(filePath) {
			return false, nil
		}
	}

	// Calculate buildhash.
	buildhash, err := p.buildhash()
	if err != nil {
		return false, fmt.Errorf("calculate buildhash: %s", err)
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
		return false, fmt.Errorf("read cache: %s", err)
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

	if err := p.MatchedConfig.Install(p.Package.Url, p.Package.Ref, p.Package.Archive); err != nil {
		installFailed = true
		return err
	}

	// Generate hash file.
	if p.MatchedConfig.BuildSystem != "nobuild" {
		builddesc, err := p.builddesc()
		if err != nil {
			installFailed = true
			return err
		}
		hashFile := filepath.Join(p.packageDir, p.desc2hash(builddesc))
		if err := os.MkdirAll(filepath.Dir(hashFile), os.ModePerm); err != nil {
			installFailed = true
			return err
		}
		if err := os.WriteFile(hashFile, []byte(builddesc), os.ModePerm); err != nil {
			installFailed = true
			return err
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

		// Rename hash file as new name in hash folder.
		if p.isChecksumFile(filepath.Join(p.packageDir, file)) {
			dest = p.hashFile
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

	// Check if have hash file in package, no hash file indicates the package is invalid.
	var hashFile string
	entities, err := os.ReadDir(p.MatchedConfig.PortConfig.PackageDir)
	if err != nil {
		return false, fmt.Errorf("read package dir: %w", err)
	}
	for _, entity := range entities {
		if p.isChecksumFile(filepath.Join(p.MatchedConfig.PortConfig.PackageDir, entity.Name())) {
			hashFile = filepath.Join(p.MatchedConfig.PortConfig.PackageDir, entity.Name())
			break
		}
	}
	if hashFile == "" {
		suffix := expr.If(p.DevDep, "@dev", "")
		return false, fmt.Errorf("invalid package %s, since hash is not found for %s", p.packageDir, p.NameVersion()+suffix)
	}

	// Install from package if buildhash matches.
	buildBytes, err := os.ReadFile(hashFile)
	if err != nil {
		return false, fmt.Errorf("read package buildhash of %s: %w", p.NameVersion(), err)
	}
	newBuilddesc, err := p.builddesc()
	if err != nil {
		return false, fmt.Errorf("calculate buildhash of %s: %w", p.NameVersion(), err)
	}

	localBuilddesc := string(buildBytes)
	if localBuilddesc != newBuilddesc {
		color.Printf(color.Green, "================ build desc not match for %s: ================\n", p.NameVersion())
		color.Println(color.Green, ">>>>>>>>>>>>>>>>> Local build desc: <<<<<<<<<<<<<<<<<")
		color.Println(color.Blue, newBuilddesc)
		color.Println(color.Green, ">>>>>>>>>>>>>>>>> New build desc: <<<<<<<<<<<<<<<<<")
		color.Println(color.Blue, newBuilddesc)

		if err := p.doInstallFromPackage(p.installedDir); err != nil {
			return false, fmt.Errorf("install from package: %w", err)
		}
		return true, p.writeInfoFile("package")
	}

	// Remove overdue package.
	if err := p.Remove(false, false, false); err != nil {
		color.Printf(color.Yellow, "[✘] ======== failed to remove overdue package %s. ========\n", err)
	}

	color.Printf(color.Magenta, "[✔] ======== remove overdue package %s successfully. ========\n", p.NameVersion())
	return false, nil
}

func (p Port) installFromCache() (bool, error) {
	installed, err := p.doInstallFromCache()
	if err != nil {
		return false, fmt.Errorf("install from cache: %w", err)
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
		return true, p.writeInfoFile(fmt.Sprintf("cache [%s]", fromDir))
	}

	return false, nil
}

func (p Port) installFromSource() error {
	if err := p.installDependencies(); err != nil {
		return err
	}

	color.Printf(color.Cyan, "-- Preparing build [dev_]dependencies for %s\n", p.NameVersion())
	preparedTmpDeps = []string{}
	if err := p.providerTmpDeps(); err != nil {
		return err
	}
	if err := p.doInstallFromSource(); err != nil {
		return err
	}

	if err := p.doInstallFromPackage(p.installedDir); err != nil {
		return err
	}

	// Write package to cache dirs so that others can share installed libraries,
	// but only for none-dev package currently.
	if !p.DevDep {
		if p.ctx.CacheDir() != nil {
			builddesc, err := p.builddesc()
			if err != nil {
				return err
			}

			if err := p.ctx.CacheDir().Write(p.MatchedConfig.PortConfig.PackageDir, builddesc); err != nil {
				return err
			}
		}
	}

	return p.writeInfoFile("source")
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
		var prefix = expr.If(p.crossTools().RootFS == "" || p.DevDep,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.ctx.Platform().HostName()+"-dev"),
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.MatchedConfig.PortConfig.LibraryFolder),
		)
		if err := fileio.FixupPkgConfig(p.packageDir, prefix); err != nil {
			return fmt.Errorf("fixup pkg-config failed: %w", err)
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
		var prefix = expr.If(p.crossTools().RootFS == "" || p.DevDep,
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.ctx.Platform().HostName()+"-dev"),
			filepath.Join(string(os.PathSeparator), "tmp", "deps", p.MatchedConfig.PortConfig.LibraryFolder),
		)
		if err := fileio.FixupPkgConfig(port.tmpDepsDir, prefix); err != nil {
			return fmt.Errorf("fixup pkg-config failed: %w", err)
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

func (p Port) writeInfoFile(installedFrom string) error {
	// Write installed files info into its installation info list.
	if err := os.MkdirAll(filepath.Dir(p.infoFile), os.ModePerm); err != nil {
		return fmt.Errorf("create info dir: %w", err)
	}
	packageFiles, err := p.PackageFiles(p.packageDir, p.ctx.Platform().Name, p.ctx.Project().Name)
	if err != nil {
		return fmt.Errorf("get package files: %w", err)
	}
	if err := os.WriteFile(p.infoFile, []byte(strings.Join(packageFiles, "\n")), os.ModePerm); err != nil {
		return fmt.Errorf("write info file: %w", err)
	}

	// Print install info.
	title := color.Sprintf(color.Green, "\n[✔] ---- package: %s is installed from %s\n",
		p.NameVersion(), installedFrom)
	fmt.Printf("%sLocation: %s\n", title, p.installedDir)
	return nil
}

func (p Port) isChecksumFile(filePath string) bool {
	fileName := filepath.Base(filePath)

	// Sha-256 always has 64 characters.
	if len(fileName) != 64 {
		return false
	}

	// Check if contains only hexadecimal characters (0-9, a-f, A-F).
	matched, err := regexp.MatchString(`^[0-9a-fA-F]{64}$`, fileName)
	if err != nil || !matched {
		return false
	}

	// Check if the checksum matches the file content.
	checksum, err := fileio.CalculateChecksum(filePath)
	if err != nil {
		return false
	}
	return checksum == fileName
}
