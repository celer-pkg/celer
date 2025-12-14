package timemachine

import (
	"celer/buildsystems"
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

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
	if err := os.MkdirAll(e.exportDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create export directory: %w", err)
	}

	title := fmt.Sprintf("\nExporting snapshot: %s", e.exportDir)
	color.Println(color.Title, title)
	color.Println(color.Line, strings.Repeat("-", len(title)))

	// 1. Collect used ports.
	color.Println(color.List, "✔ Collecting dependencies...")
	usedPorts, err := e.collector.CollectUsedPorts(e.celer)
	if err != nil {
		return fmt.Errorf("failed to collect ports: %w", err)
	}
	e.usedPorts = usedPorts
	color.Printf(color.List, "  Found %d port(s)\n", len(e.usedPorts))

	// 2. Export ports with fixed commits.
	color.Println(color.List, "✔ Exporting ports...")
	portSnapshots, err := e.exportPorts()
	if err != nil {
		return fmt.Errorf("failed to export ports: %w", err)
	}

	// 3. Export conf directory.
	color.Println(color.List, "✔ Exporting configuration...")
	if err := e.exportConf(); err != nil {
		return fmt.Errorf("failed to export conf: %w", err)
	}

	// 4. Export celer.toml.
	color.Println(color.List, "✔ Exporting celer.toml...")
	if err := e.exportCelerToml(); err != nil {
		return fmt.Errorf("failed to export celer.toml: %w", err)
	}

	// 5. Export toolchain_file.cmake (if exists).
	color.Println(color.List, "✔ Exporting toolchain file...")
	if err := e.exportToolchainFile(); err != nil {
		// Non-fatal if toolchain file doesn't exist.
		color.Printf(color.Warning, "  Warning: %v\n", err)
	}

	// 6. Export celer executable.
	color.Println(color.List, "✔ Exporting celer executable...")
	if err := e.exportCelerExecutable(); err != nil {
		// Non-fatal if executable doesn't exist.
		color.Printf(color.Warning, "  Warning: %v\n", err)
	}

	// 7. Generate snapshot.
	color.Println(color.List, "✔ Generating snapshot...")
	snapshot := &Snapshot{
		ExportedAt:   time.Now(),
		CelerVersion: "0.1.0", // TODO: Get from version.
		Platform:     e.celer.Platform().GetName(),
		Project:      e.celer.Project().GetName(),
		Dependencies: portSnapshots,
		Notes:        "Exported workspace for reproducible builds",
	}

	if err := snapshot.Save(e.exportDir); err != nil {
		return fmt.Errorf("failed to save snapshot: %w", err)
	}

	configs.PrintSuccess("Snapshot exported to: %s", e.exportDir)
	return nil
}

// exportPorts exports ports with fixed commits.
func (e *Exporter) exportPorts() ([]PortSnapshot, error) {
	portsDir := filepath.Join(e.exportDir, "ports")
	if err := os.MkdirAll(portsDir, os.ModePerm); err != nil {
		return nil, err
	}

	var snapshots []PortSnapshot
	platformName := e.celer.Platform().GetName()

	for nameVersion, port := range e.usedPorts {
		// Get commit hash.
		commit, err := e.collector.GetPortCommit(port)
		if err != nil {
			return nil, fmt.Errorf("failed to get commit for %s: %w", nameVersion, err)
		}

		// Create port directory.
		portDir := filepath.Join(portsDir, nameVersion)
		if err := os.MkdirAll(portDir, os.ModePerm); err != nil {
			return nil, err
		}

		// Create a copy of the port with fixed commit and only matched config.
		exportedPort := *port
		exportedPort.Package.Ref = commit // Change ref to commit.

		// Only export the matched build config for current platform.
		if port.MatchedConfig == nil {
			return nil, fmt.Errorf("no matched build config for port %s", nameVersion)
		}
		matchedConfig := *port.MatchedConfig
		matchedConfig.Pattern = platformName
		exportedPort.BuildConfigs = []buildsystems.BuildConfig{matchedConfig}

		// Write port.toml.
		portTomlPath := filepath.Join(portDir, "port.toml")
		file, err := os.Create(portTomlPath)
		if err != nil {
			return nil, err
		}

		encoder := toml.NewEncoder(file)
		encoder.Indent = "  "
		if err := encoder.Encode(exportedPort); err != nil {
			file.Close()
			return nil, err
		}
		file.Close()

		// Add to snapshots.
		snapshots = append(snapshots, PortSnapshot{
			Name:    port.Name,
			Version: port.Version,
			Commit:  commit,
			URL:     port.Package.Url,
		})
	}

	return snapshots, nil
}

// exportConf copies the conf directory (excluding .git).
func (e *Exporter) exportConf() error {
	srcConf := dirs.ConfDir
	dstConf := filepath.Join(e.exportDir, "conf")

	// Copy conf directory but skip .git directory.
	return filepath.Walk(srcConf, func(srcPath string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		// Skip .git directory.
		if info.IsDir() && info.Name() == ".git" {
			return filepath.SkipDir
		}

		relPath, err := filepath.Rel(srcConf, srcPath)
		if err != nil {
			return err
		}
		dstPath := filepath.Join(dstConf, relPath)

		if info.IsDir() {
			return os.MkdirAll(dstPath, info.Mode())
		}

		return fileio.CopyFile(srcPath, dstPath)
	})
}

// exportCelerToml copies celer.toml.
func (e *Exporter) exportCelerToml() error {
	src := filepath.Join(dirs.WorkspaceDir, "celer.toml")
	dst := filepath.Join(e.exportDir, "celer.toml")

	return fileio.CopyFile(src, dst)
}

// exportToolchainFile copies toolchain_file.cmake.
func (e *Exporter) exportToolchainFile() error {
	src := filepath.Join(dirs.WorkspaceDir, "toolchain_file.cmake")
	dst := filepath.Join(e.exportDir, "toolchain_file.cmake")

	if !fileio.PathExists(src) {
		return fmt.Errorf("toolchain file not found (skipping)")
	}

	return fileio.CopyFile(src, dst)
}

// exportCelerExecutable copies celer executable.
func (e *Exporter) exportCelerExecutable() error {
	// Try to find celer executable in workspace.
	possibleNames := []string{"celer", "celer.exe"}
	var src string

	for _, name := range possibleNames {
		candidate := filepath.Join(dirs.WorkspaceDir, name)
		if fileio.PathExists(candidate) {
			src = candidate
			break
		}
	}

	if src == "" {
		return fmt.Errorf("celer executable not found (skipping)")
	}

	dst := filepath.Join(e.exportDir, filepath.Base(src))
	if err := fileio.CopyFile(src, dst); err != nil {
		return err
	}

	// Make it executable on Unix systems.
	if err := os.Chmod(dst, 0755); err != nil {
		return fmt.Errorf("failed to set executable permission: %w", err)
	}

	return nil
}
