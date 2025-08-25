package configs

import (
	"bufio"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

func (p Port) Remove(recurse, purge, removeBuildCache bool) error {
	matchedConfig := p.MatchedConfig

	// Try to remove dependencies firstly.
	if recurse {
		removeFunc := func(nameVersion string, devDep bool) error {
			// Skip self dependency.
			if devDep == p.DevDep && nameVersion == p.NameVersion() {
				return nil
			}

			// Check and validate dependency.
			var port Port
			port.DevDep = devDep
			port.Parent = p.NameVersion()
			if err := port.Init(p.ctx, nameVersion, p.buildType); err != nil {
				return fmt.Errorf("init dependency %s error: %w", nameVersion, err)
			}

			// Remove dependency.
			if err := port.Remove(recurse, purge, removeBuildCache); err != nil {
				return fmt.Errorf("remove dependency %s error: %w", nameVersion, err)
			}

			return nil
		}

		if matchedConfig != nil {
			for _, nameVersion := range matchedConfig.Dependencies {
				if err := removeFunc(nameVersion, false); err != nil {
					return fmt.Errorf("remove dependency %s error: %w", nameVersion, err)
				}
			}
			for _, nameVersion := range matchedConfig.DevDependencies {
				if err := removeFunc(nameVersion, true); err != nil {
					return fmt.Errorf("remove dev_dependency %s error: %w", nameVersion, err)
				}
			}
		}
	}

	// Do remove port itself.
	if err := p.doRemovePort(); err != nil {
		return fmt.Errorf("remove port error: %w", err)
	}

	// Remove port's package files.
	if purge {
		if err := p.removePackage(); err != nil {
			return fmt.Errorf("remove package error: %w", err)
		}
	}

	// Remove build cache and logs.
	if removeBuildCache {
		if err := os.RemoveAll(matchedConfig.PortConfig.BuildDir); err != nil {
			return fmt.Errorf("remove build cache error: %w", err)
		}

		if err := p.RemoveLogs(); err != nil {
			return fmt.Errorf("remove logs error: %w", err)
		}
	}

	return nil
}

func (p Port) doRemovePort() error {
	color.Printf(color.Blue, "\n[remove %s]: from \"%s\"\n", p.NameVersion(), p.installedDir)

	if !fileio.PathExists(p.traceFile) {
		return nil
	}

	// Open install info file.
	file, err := os.OpenFile(p.traceFile, os.O_RDONLY, os.ModePerm)
	if err != nil {
		return fmt.Errorf("cannot open install info file: %s", err)
	}

	// Read line by line to remove installed file.
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()

		// CMake project may generate a checksum file after install,
		// it would be like "/home/phil/.cmake/packages/gflags/4fbe0d242b1c0f095b87a43a7aeaf0d6",
		// We'll try to remove it also.
		fileToRemove := line
		if !fileio.PathExists(line) {
			fileToRemove = filepath.Join(dirs.WorkspaceDir, "installed", line)
		}
		if err := p.removeFiles(fileToRemove); err != nil {
			return fmt.Errorf("cannot remove file: %s", err)
		}

		// Try remove parent folder if it's empty.
		if err := fileio.RemoveFolderRecursively(filepath.Dir(fileToRemove)); err != nil {
			return fmt.Errorf("cannot remove parent folder: %s", err)
		}

		fmt.Printf("-- remove: %s\n", fileToRemove)
	}
	file.Close()

	// Remove generated cmake config if exist.
	portName := strings.Split(p.NameVersion(), "@")[0]
	platformProject := fmt.Sprintf("%s@%s@%s", p.ctx.Platform().Name, p.ctx.Project().Name, p.buildType)
	cmakeConfigDir := filepath.Join(dirs.InstalledDir, platformProject, "lib", "cmake", portName)
	if err := os.RemoveAll(cmakeConfigDir); err != nil {
		return fmt.Errorf("cannot remove cmake config folder: %s", err)
	}
	if err := fileio.RemoveFolderRecursively(filepath.Dir(cmakeConfigDir)); err != nil {
		return fmt.Errorf("cannot clean cmake config folder: %s", err)
	}

	// Remove info file and clean info dir.
	if err := os.Remove(p.traceFile); err != nil {
		return fmt.Errorf("cannot remove info file: %s", err)
	}
	traceDir := filepath.Join(dirs.WorkspaceDir, "installed", "celer", "trace")
	if err := fileio.RemoveFolderRecursively(traceDir); err != nil {
		return fmt.Errorf("cannot remove info dir: %s", err)
	}

	// Remove meta file and clean meta dir.
	buildSystem := p.MatchedConfig.BuildSystem
	if buildSystem != "nobuild" && buildSystem != "prebuilt" {
		metaDir := filepath.Join(dirs.WorkspaceDir, "installed", "celer", "meta")
		if err := os.Remove(p.metaFile); err != nil {
			return fmt.Errorf("cannot remove meta file: %s", err)
		}
		if err := fileio.RemoveFolderRecursively(metaDir); err != nil {
			return fmt.Errorf("cannot remove meta dir: %s", err)
		}
	}

	return nil
}

func (p Port) removePackage() error {
	// Remove port's package files.
	// packageDir := filepath.Join(dirs.WorkspaceDir, "packages", p.NameVersion()+"@"+p.matchedConfig.PortConfig.LibraryFolder)
	if err := os.RemoveAll(p.packageDir); err != nil {
		return fmt.Errorf("cannot remove package files: %s", err)
	}

	// Try remove parent folder if it's empty.
	if err := fileio.RemoveFolderRecursively(filepath.Dir(p.packageDir)); err != nil {
		return fmt.Errorf("cannot remove parent folder: %s", err)
	}

	return nil
}

// removeFiles remove files and all related shared libraries.
func (p Port) removeFiles(path string) error {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return nil
	}

	if !strings.Contains(path, "so") {
		return os.Remove(path)
	}

	index := strings.Index(path, ".so")
	if index == -1 {
		return os.Remove(path)
	}

	matches, err := filepath.Glob(path[:index] + ".so*")
	if err != nil {
		return err
	}

	for _, item := range matches {
		if err := os.Remove(item); err != nil {
			return err
		}
	}

	return nil
}

func (p Port) RemoveLogs() error {
	platformProject := fmt.Sprintf("%s-%s-%s", p.ctx.Platform().Name, p.ctx.Project().Name, p.buildType)
	logPathPrefix := filepath.Join(p.NameVersion(), expr.If(p.DevDep || p.Native, p.ctx.Platform().HostName()+"-dev", platformProject))
	matches, err := filepath.Glob(filepath.Join(dirs.BuildtreesDir, logPathPrefix+"-*.log"))
	if err != nil {
		return fmt.Errorf("glob syntax error: %w", err)
	}

	for _, match := range matches {
		if err := os.Remove(match); err != nil {
			return fmt.Errorf("remove log %s error: %w", match, err)
		}
	}

	return nil
}
