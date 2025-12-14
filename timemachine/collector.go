package timemachine

import (
	"celer/configs"
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"path/filepath"
	"strings"
)

// Collector collects ports and their dependencies.
type Collector struct {
	ctx       context.Context
	collected map[string]*configs.Port
}

// NewCollector creates a new Collector instance.
func NewCollector(ctx context.Context) *Collector {
	return &Collector{
		ctx:       ctx,
		collected: make(map[string]*configs.Port),
	}
}

// CollectUsedPorts collects all ports used by the project.
func (c *Collector) CollectUsedPorts(ctx context.Context) (map[string]*configs.Port, error) {
	projectPorts := ctx.Project().GetPorts()
	for _, nameVersion := range projectPorts {
		if err := c.collectRecursive(nameVersion); err != nil {
			return nil, err
		}
	}

	return c.collected, nil
}

func (c *Collector) collectRecursive(nameVersion string) error {
	// Skip if already collected.
	if _, exists := c.collected[nameVersion]; exists {
		return nil
	}

	// Load port.
	var port configs.Port
	if err := port.Init(c.ctx, nameVersion); err != nil {
		return fmt.Errorf("failed to init port %s: %w", nameVersion, err)
	}
	c.collected[nameVersion] = &port

	// Recursively collect dependencies.
	for _, config := range port.BuildConfigs {
		for _, nameVersion := range config.Dependencies {
			if err := c.collectRecursive(nameVersion); err != nil {
				return err
			}
		}
		for _, nameVersion := range config.DevDependencies {
			if err := c.collectRecursive(nameVersion); err != nil {
				return err
			}
		}
	}

	return nil
}

// GetPortCommit gets the actual commit hash from the downloaded source repository.
// This reads the commit from the actual git repository after it has been cloned.
func (c *Collector) GetPortCommit(port *configs.Port) (string, error) {
	// For archive downloads (zip/tar), use the sha-256 as commit.
	if !strings.HasSuffix(port.Package.Url, ".git") {
		archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
		filePath := filepath.Join(dirs.DownloadedDir, archive)
		commit, err := fileio.CalculateChecksum(filePath)
		if err != nil {
			return "", fmt.Errorf("failed to get checksum of port's archive %s.\n %w", port.NameVersion(), err)
		}
		return "sha-256:" + commit, nil
	}

	// For private repositories with fixed commit, just use the specified commit.
	if port.Package.Commit != "" {
		return port.Package.Commit, nil
	}

	// For git repositories, read the actual commit from the cloned repo.
	commit, error := git.ReadLocalCommit(port.Package.SrcDir)
	if error != nil {
		return "", fmt.Errorf("failed to read local commit for %s: %w", port.NameVersion(), error)
	}
	if commit != "" {
		return commit, nil
	}
	return "", fmt.Errorf("commit not found for %s", port.NameVersion())
}
