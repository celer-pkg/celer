package snapshot

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/refs"

	"github.com/BurntSushi/toml"
)

// Exporter handles workspace export operations.
type Exporter struct {
	celer     *configs.Celer
	exportDir string
	collector *Collector
	usedPorts map[string]*configs.Port
}

// NewExporter creates a new Exporter instance.
func NewExporter(celer *configs.Celer, exportDir string) *Exporter {
	return &Exporter{
		celer:     celer,
		exportDir: exportDir,
		collector: NewCollector(celer),
	}
}

// Export exports the current workspace to a snapshot directory.
func Export(celer *configs.Celer, exportDir string) error {
	exporter := NewExporter(celer, exportDir)
	return exporter.Export()
}

// Export performs the export operation.
func (e *Exporter) Export() error {
	// Create export directory.
	if err := os.RemoveAll(e.exportDir); err != nil {
		return fmt.Errorf("failed to clear existing export directory -> %w", err)
	}
	if err := os.MkdirAll(e.exportDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create export directory -> %w", err)
	}

	title := fmt.Sprintf("\nExporting snapshot: %s", e.exportDir)
	color.Println(color.Title, title)
	color.Println(color.Line, strings.Repeat("-", len(title)))

	// 1. Collect used ports.
	color.Println(color.Hint, "✔ Collecting dependencies...")
	usedPorts, err := e.collector.CollectUsedPorts(e.celer)
	if err != nil {
		return fmt.Errorf("failed to collect ports -> %w", err)
	}
	e.usedPorts = usedPorts
	color.Printf(color.Hint, "  Found %d port(s)\n", len(e.usedPorts))

	// 2. Export ports with fixed source checksums.
	color.Println(color.Hint, "✔ Exporting ports...")
	if err := e.exportPorts(); err != nil {
		return fmt.Errorf("failed to export ports -> %w", err)
	}

	// 3. Export conf directory.
	color.Println(color.Hint, "✔ Exporting configuration...")
	if err := e.exportConf(); err != nil {
		return fmt.Errorf("failed to export conf -> %w", err)
	}

	// 4. Export celer.toml.
	color.Println(color.Hint, "✔ Exporting celer.toml...")
	if err := e.exportCelerToml(); err != nil {
		return fmt.Errorf("failed to export celer.toml -> %w", err)
	}

	// 5. Export toolchain_file.cmake (if exists).
	color.Println(color.Hint, "✔ Exporting toolchain file...")
	if err := e.exportToolchainFile(); err != nil {
		return fmt.Errorf("failed to export toolchain_file.cmake -> %w", err)
	}

	// 6. Export celer executable.
	color.Println(color.Hint, "✔ Exporting celer executable...")
	if err := e.exportCelerExecutable(); err != nil {
		return fmt.Errorf("failed to export celer executable -> %w", err)
	}

	// 7. Generate snapshot.
	color.Println(color.Hint, "✔ Generating snapshot report...")
	buildEnv := BuildEnv{
		ExportedAt:   time.Now(),
		CelerVersion: e.celer.Version(),
		Platform:     e.celer.Platform().GetName(),
		Project:      e.celer.Project().GetName(),
	}
	resolvedRefs := e.buildResolvedRefs()
	snapshotPath := filepath.Join(e.exportDir, "snapshot.md")
	if err := SaveSnapshotMarkdown(snapshotPath, buildEnv, resolvedRefs); err != nil {
		return fmt.Errorf("failed to save snapshot -> %w", err)
	}

	color.PrintSuccess("Snapshot is exported to: %s", e.exportDir)
	return nil
}

func (e *Exporter) exportPorts() error {
	portsDir := filepath.Join(e.exportDir, "ports")
	if err := os.MkdirAll(portsDir, os.ModePerm); err != nil {
		return err
	}

	var (
		systemName      string
		systemProcessor string
	)

	toolchain := e.celer.Platform().GetToolchain()
	if toolchain != nil {
		systemName = toolchain.GetSystemName()
		systemProcessor = toolchain.GetSystemProcessor()
	}

	for nameVersion, port := range e.usedPorts {
		// Get the reproducibility checksum for this port source.
		checksum, err := e.collector.GetPortChecksum(port)
		if err != nil {
			return fmt.Errorf("failed to get checksum for %s -> %w", nameVersion, err)
		}

		// Create port directory with first-letter grouping (e.g. ports/g/glog/0.6.0).
		parts := strings.SplitN(nameVersion, "@", 2)
		if len(parts) != 2 {
			return fmt.Errorf("invalid port name@version: %s", nameVersion)
		}

		portName := parts[0]
		groupChar := strings.ToLower(string([]rune(portName)[0]))
		portDir := filepath.Join(portsDir, groupChar, parts[0], parts[1])
		if err := os.MkdirAll(portDir, os.ModePerm); err != nil {
			return err
		}

		// Copy all supplementary files from the source port directory
		// (patches, cmake_config.toml, CMakeLists.txt, etc.).
		srcPortDir := dirs.GetPortDir(parts[0], parts[1])
		if fileio.PathExists(srcPortDir) {
			entries, err := os.ReadDir(srcPortDir)
			if err != nil {
				return fmt.Errorf("failed to read port dir %s -> %w", srcPortDir, err)
			}
			for _, entry := range entries {
				if entry.Name() == "port.toml" {
					continue // port.toml is written separately with modifications.
				}
				if entry.IsDir() {
					continue // port version dirs contain only flat files.
				}

				srcPath := filepath.Join(srcPortDir, entry.Name())
				dstPath := filepath.Join(portDir, entry.Name())
				if err := fileio.CopyFile(srcPath, dstPath); err != nil {
					return fmt.Errorf("failed to copy %s -> %w", srcPath, err)
				}
			}
		}

		// Create a copy of the port with a fixed checksum and only matched config.
		exportedPort := *port
		exportedPort.Package.Checksum = checksum

		// Only export the matched build config for current platform.
		if port.MatchedConfig == nil {
			return fmt.Errorf("no matched build config for port %s", nameVersion)
		}
		matchedConfig := *port.MatchedConfig
		matchedConfig.SystemName = systemName
		matchedConfig.SystemProcessor = systemProcessor
		exportedPort.BuildConfigs = []buildsystems.BuildConfig{matchedConfig}

		// Write port.toml.
		portTomlPath := filepath.Join(portDir, "port.toml")
		file, err := os.Create(portTomlPath)
		if err != nil {
			return err
		}

		encoder := toml.NewEncoder(file)
		encoder.Indent = "  "
		if err := encoder.Encode(exportedPort); err != nil {
			file.Close()
			return err
		}
		file.Close()
	}

	return nil
}

func (e *Exporter) exportConf() error {
	dstConf := filepath.Join(e.exportDir, "conf")

	// 1. Export only the current platform's .toml file.
	platformName := e.celer.Platform().GetName()
	srcPlatformFile := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
	dstPlatformDir := filepath.Join(dstConf, "platforms")
	if err := os.MkdirAll(dstPlatformDir, os.ModePerm); err != nil {
		return err
	}
	if err := fileio.CopyFile(srcPlatformFile, filepath.Join(dstPlatformDir, platformName+".toml")); err != nil {
		return fmt.Errorf("failed to copy platform file -> %w", err)
	}

	// 2. Export only the current project's .toml file and its subdirectory (port overrides).
	projectName := e.celer.Project().GetName()
	srcProjectFile := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
	dstProjectDir := filepath.Join(dstConf, "projects")
	if err := os.MkdirAll(dstProjectDir, os.ModePerm); err != nil {
		return err
	}
	if err := fileio.CopyFile(srcProjectFile, filepath.Join(dstProjectDir, projectName+".toml")); err != nil {
		return fmt.Errorf("failed to copy project file -> %w", err)
	}

	// Copy project-specific port overrides only for used ports.
	srcProjectSubdir := filepath.Join(dirs.ConfProjectsDir, projectName)
	for nameVersion := range e.usedPorts {
		parts := strings.SplitN(nameVersion, "@", 2)
		if len(parts) != 2 {
			continue
		}
		srcPortDir := filepath.Join(srcProjectSubdir, parts[0], parts[1])
		if !fileio.PathExists(srcPortDir) {
			continue
		}
		dstPortDir := filepath.Join(dstProjectDir, projectName, parts[0], parts[1])
		if err := filepath.Walk(srcPortDir, func(srcPath string, info os.FileInfo, err error) error {
			if err != nil {
				return err
			}
			relPath, err := filepath.Rel(srcPortDir, srcPath)
			if err != nil {
				return err
			}
			dstPath := filepath.Join(dstPortDir, relPath)
			if info.IsDir() {
				return os.MkdirAll(dstPath, info.Mode())
			}
			return fileio.CopyFile(srcPath, dstPath)
		}); err != nil {
			return fmt.Errorf("failed to copy project overrides for %s -> %w", nameVersion, err)
		}
	}

	// 3. Export only the host's buildtools .toml file.
	hostName := e.celer.Platform().GetHostName()
	srcBuildtoolsFile := filepath.Join(dirs.ConfDir, "buildtools", hostName+".toml")
	dstBuildtoolsDir := filepath.Join(dstConf, "buildtools")
	if fileio.PathExists(srcBuildtoolsFile) {
		if err := os.MkdirAll(dstBuildtoolsDir, os.ModePerm); err != nil {
			return err
		}
		if err := fileio.CopyFile(srcBuildtoolsFile, filepath.Join(dstBuildtoolsDir, hostName+".toml")); err != nil {
			return fmt.Errorf("failed to copy buildtools file -> %w", err)
		}
	}

	return nil
}

func (e Exporter) exportCelerToml() error {
	// Clear so it defaults to <workspace>/downloads at runtime.
	e.celer.Main.Downloads = ""

	bytes, err := toml.Marshal(e.celer)
	if err != nil {
		return fmt.Errorf("failed to marshal celer.toml -> %w", err)
	}

	dstFile := filepath.Join(e.exportDir, "celer.toml")
	if err := os.WriteFile(dstFile, bytes, os.ModePerm); err != nil {
		return fmt.Errorf("failed to write celer.toml -> %w", err)
	}

	return nil
}

func (e *Exporter) exportToolchainFile() error {
	src := filepath.Join(dirs.WorkspaceDir, "toolchain_file.cmake")
	dst := filepath.Join(e.exportDir, "toolchain_file.cmake")

	if !fileio.PathExists(src) {
		return fmt.Errorf("toolchain file not found")
	}

	return fileio.CopyFile(src, dst)
}

func (e *Exporter) exportCelerExecutable() error {
	exePath, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer executable path -> %w", err)
	}

	dst := filepath.Join(e.exportDir, filepath.Base(exePath))
	if err := fileio.CopyFile(exePath, dst); err != nil {
		return err
	}

	return nil
}

// buildResolvedRefs converts usedPorts to []refs.ResolvedRef for the snapshot markdown.
// It reuses already-resolved commits from refs.StoreResolvedCommits() populated during deploy.
func (e *Exporter) buildResolvedRefs() []refs.ResolvedRef {
	var results []refs.ResolvedRef
	for _, port := range e.usedPorts {
		ref := refs.ResolvedRef{
			NameVersion: port.NameVersion(),
			Url:         port.Package.Url,
			OriginalRef: port.Package.Ref,
			Checksum:    port.Package.Checksum,
		}
		switch {
		case port.Package.Url == "_":
			ref.SourceType = refs.SourceVirtual
			ref.Url, ref.OriginalRef = "-", "-"
		case strings.HasSuffix(port.Package.Url, ".git"):
			ref.SourceType = refs.SourceGit
			if commit := refs.GetResolvedCommit(port.NameVersion()); commit != "" {
				ref.ResolvedCommit = commit
			}
		default:
			ref.SourceType = refs.SourceArchive
		}
		results = append(results, ref)
	}
	return results
}
