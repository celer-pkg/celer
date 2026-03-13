package timemachine

import (
	"celer/configs"
	"celer/context"
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
		return fmt.Errorf("failed to init port %s -> %w", nameVersion, err)
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

// GetPortChecksum gets the reproducibility checksum from the downloaded source.
// For git sources this is the checked-out commit hash. For archive sources this
// is the archive sha-256 prefixed with "sha-256:".
func (c *Collector) GetPortChecksum(port *configs.Port) (string, error) {
	// For archive downloads (zip/tar), use the sha-256 as checksum.
	if !strings.HasSuffix(port.Package.Url, ".git") {
		archive := expr.If(port.Package.Archive != "", port.Package.Archive, filepath.Base(port.Package.Url))
		filePath := filepath.Join(c.ctx.Downloads(), archive)
		sha256, err := fileio.GetFileSha256(filePath)
		if err != nil {
			return "", fmt.Errorf("failed to get checksum of port's archive %s -> %w", port.NameVersion(), err)
		}
		return "sha-256:" + sha256, nil
	}

	// For private repositories with fixed checksum, just use the specified value.
	if port.Package.Checksum != "" {
		return port.Package.Checksum, nil
	}

	// For git repositories, read the actual commit from the cloned repo.
	commitHash, err := git.GetCommitHash(port.Package.SrcDir)
	if err != nil {
		return "", fmt.Errorf("failed to read local source checksum for %s -> %w", port.NameVersion(), err)
	}
	if commitHash != "" {
		return commitHash, nil
	}
	return "", fmt.Errorf("source checksum not found for %s", port.NameVersion())
}
