package pkgcache

import (
	"bytes"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type portType int

const (
	portTypePort portType = iota
	portTypeDependency
	portTypeDevDependency
)

type Callbacks interface {
	GenPortTomlString(nameVersion string, devDep bool) (string, error)
	GenPlatformTomlString() (string, error)
	GenPlatformChecksums() (toolchainChecksum, rootfsChecksum string, err error)
	GenBuildToolsVersions(tools []string) (string, error)
	GetCommitHash(nameVersion string, devDep bool) (string, error)
	GetBuildConfig(nameVersion string, devDep bool) (*BuildConfig, error)
	CheckHostSupported(nameVersion string) bool
}

type BuildConfig struct {
	Patches         []string
	Dependencies    []string
	DevDependencies []string
	BuildTools      []string
}

type Port struct {
	Platform    string
	Project     string
	PortType    portType
	NameVersion string
	DevDep      bool
	HostDev     bool
	Parents     []string
	BuildConfig BuildConfig
	Callbacks   Callbacks
}

func (p Port) BuildMeta(commit string) (string, error) {
	var buffer bytes.Buffer

	// Collect buildtool infos.
	if p.PortType == portTypePort {
		buildTools, err := p.collectBuildTools(map[string]struct{}{}, map[string]struct{}{})
		if err != nil {
			return "", fmt.Errorf("collect build tools -> %w", err)
		}

		toolVersions, err := p.Callbacks.GenBuildToolsVersions(buildTools)
		if err != nil {
			return "", fmt.Errorf("failed to get build tools versions -> %w", err)
		}
		if toolVersions != "" {
			p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "build tools versions")
			buffer.WriteString(toolVersions + "\n\n")
		}
	}

	content, err := p.buildMeta(commit)
	if err != nil {
		return "", err
	}
	buffer.WriteString(content)
	return buffer.String(), nil
}

func (p Port) buildMeta(commit string) (string, error) {
	var buffer bytes.Buffer

	// Write celer version and platform content for root port only.
	if p.PortType == portTypePort {
		// Write content of platform toml.
		p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "platform")
		platform, err := p.Callbacks.GenPlatformTomlString()
		if err != nil {
			return "", err
		}
		fmt.Fprintf(&buffer, "%s\n", platform)

		// Write content of platform archive's checksum.
		toolchainChecksum, rootfsChecksum, err := p.Callbacks.GenPlatformChecksums()
		if err != nil {
			return "", fmt.Errorf("failed to get platform archive checksums -> %w", err)
		}
		if toolchainChecksum != "" {
			buffer.WriteString(newDivider(nil, "toolchain checksum"))
			fmt.Fprintf(&buffer, "%s\n\n", toolchainChecksum)
		}
		if rootfsChecksum != "" {
			buffer.WriteString(newDivider(nil, "rootfs checksum"))
			fmt.Fprintf(&buffer, "%s\n\n", rootfsChecksum)
		}
	}

	// Write port content.
	parts := strings.Split(p.NameVersion, "@")
	if len(parts) != 2 {
		return "", fmt.Errorf("invalid port name and version: %s", p.NameVersion)
	}

	portInProject := filepath.Join(dirs.ConfProjectsDir, p.Project, parts[0], parts[1], "port.toml")
	portInPorts := dirs.GetPortPath(parts[0], parts[1])
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		return "", fmt.Errorf("port %s not found", p.NameVersion)
	}
	content, err := p.Callbacks.GenPortTomlString(p.NameVersion, p.DevDep)
	if err != nil {
		return "", fmt.Errorf("generate toml content of port %s -> %w", p.NameVersion, err)
	}
	p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "port")
	buffer.WriteString(content + "\n")

	// Write commit of port.
	if commit != "" {
		p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "commit")
		buffer.WriteString(commit + "\n\n")
	} else {
		commit, err := p.Callbacks.GetCommitHash(p.NameVersion, p.DevDep)
		if err != nil {
			return "", fmt.Errorf("failed to get commit of port %s\n %w", p.NameVersion, err)
		}
		p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "commit")
		buffer.WriteString(commit + "\n\n")
	}

	// Write content of patches.
	for _, patch := range p.BuildConfig.Patches {
		content, err := p.readPatch(p.NameVersion, patch)
		if err != nil {
			return "", fmt.Errorf("read patch %s -> %w", patch, err)
		}
		p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, fmt.Sprintf("patch: %s", patch))
		buffer.WriteString(content + "\n")
	}

	// Write content of dev_dependencies.
	for _, nameVersion := range p.BuildConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (p.DevDep || p.HostDev) && p.NameVersion == nameVersion {
			continue
		}

		// Skip if not supported.
		if !p.Callbacks.CheckHostSupported(nameVersion) {
			continue
		}

		buildConfig, err := p.Callbacks.GetBuildConfig(nameVersion, true)
		if err != nil {
			return "", fmt.Errorf("get build config of dependency %s -> %w", nameVersion, err)
		}

		parent := expr.If(len(p.Parents) == 0, p.NameVersion, fmt.Sprintf("dev_dependency: %s", p.NameVersion))
		port := Port{
			Platform:    p.Platform,
			PortType:    portTypeDevDependency,
			NameVersion: nameVersion,
			Project:     p.Project,
			DevDep:      true,
			Parents:     append(p.Parents, parent),
			BuildConfig: *buildConfig,
			Callbacks:   p.Callbacks,
		}

		content, err := port.buildMeta("")
		if err != nil {
			return "", fmt.Errorf("fill content of dev_dependency %s -> %w", nameVersion, err)
		}
		buffer.WriteString(string(content))
	}

	// Write content of dependencies.
	for _, nameVersion := range p.BuildConfig.Dependencies {
		buildConfig, err := p.Callbacks.GetBuildConfig(nameVersion, false)
		if err != nil {
			return "", fmt.Errorf("get build config of dependency %s -> %w", nameVersion, err)
		}

		parent := expr.If(len(p.Parents) == 0, p.NameVersion, fmt.Sprintf("dependency: %s", p.NameVersion))
		port := Port{
			Platform:    p.Platform,
			PortType:    portTypeDependency,
			NameVersion: nameVersion,
			Project:     p.Project,
			DevDep:      p.DevDep,
			Parents:     append(p.Parents, parent),
			BuildConfig: *buildConfig,
			Callbacks:   p.Callbacks,
		}

		content, err := port.buildMeta("")
		if err != nil {
			return "", fmt.Errorf("fill content of dependency %s -> %w", nameVersion, err)
		}
		buffer.WriteString(string(content))
	}

	return buffer.String(), nil
}

func (p Port) collectBuildTools(visitedPorts, seenTools map[string]struct{}) ([]string, error) {
	key := fmt.Sprintf("%s|%t|%t", p.NameVersion, p.DevDep, p.HostDev)
	if _, ok := visitedPorts[key]; ok {
		return nil, nil
	}
	visitedPorts[key] = struct{}{}

	var tools []string
	appendTool := func(tool string) {
		if _, ok := seenTools[tool]; !ok {
			seenTools[tool] = struct{}{}
			tools = append(tools, tool)
		}
	}

	for _, tool := range p.BuildConfig.BuildTools {
		appendTool(tool)
	}

	for _, nameVersion := range p.BuildConfig.DevDependencies {
		// Ignore self.
		if (p.DevDep || p.HostDev) && p.NameVersion == nameVersion {
			continue
		}
		if !p.Callbacks.CheckHostSupported(nameVersion) {
			continue
		}

		buildConfig, err := p.Callbacks.GetBuildConfig(nameVersion, true)
		if err != nil {
			return nil, fmt.Errorf("get build config of dev dependency %s -> %w", nameVersion, err)
		}

		parent := expr.If(len(p.Parents) == 0, p.NameVersion, fmt.Sprintf("dev_dependency: %s", p.NameVersion))
		childTools, err := Port{
			Platform:    p.Platform,
			Project:     p.Project,
			PortType:    portTypeDevDependency,
			NameVersion: nameVersion,
			DevDep:      true,
			Parents:     append(p.Parents, parent),
			BuildConfig: *buildConfig,
			Callbacks:   p.Callbacks,
		}.collectBuildTools(visitedPorts, seenTools)
		if err != nil {
			return nil, err
		}
		tools = append(tools, childTools...)
	}

	for _, nameVersion := range p.BuildConfig.Dependencies {
		buildConfig, err := p.Callbacks.GetBuildConfig(nameVersion, false)
		if err != nil {
			return nil, fmt.Errorf("get build config of dependency %s -> %w", nameVersion, err)
		}

		parent := expr.If(len(p.Parents) == 0, p.NameVersion, fmt.Sprintf("dependency: %s", p.NameVersion))
		childTools, err := Port{
			Platform:    p.Platform,
			Project:     p.Project,
			PortType:    portTypeDependency,
			NameVersion: nameVersion,
			DevDep:      p.DevDep,
			Parents:     append(p.Parents, parent),
			BuildConfig: *buildConfig,
			Callbacks:   p.Callbacks,
		}.collectBuildTools(visitedPorts, seenTools)
		if err != nil {
			return nil, err
		}
		tools = append(tools, childTools...)
	}

	return tools, nil
}

func (p Port) writeSectionTitle(buffer *bytes.Buffer, parents []string, nameVersion, what string) {
	switch p.PortType {
	case portTypePort:
		buffer.WriteString(newDivider(parents, nameVersion, what))
	case portTypeDependency:
		buffer.WriteString(newDivider(parents, fmt.Sprintf("dependency: %s", nameVersion), what))
	case portTypeDevDependency:
		buffer.WriteString(newDivider(parents, fmt.Sprintf("dev_dependency: %s", nameVersion), what))
	}
}

func (p Port) readPatch(portNameVersion, patchFileName string) (string, error) {
	parts := strings.Split(portNameVersion, "@")
	if len(parts) != 2 {
		return "", fmt.Errorf("invalid port name and version: %s", p.NameVersion)
	}

	portPatchPath := filepath.Join(dirs.GetPortDir(parts[0], parts[1]), patchFileName)
	projectPatchPath := filepath.Join(dirs.ConfProjectsDir, p.Project, parts[0], parts[1], patchFileName)

	var patchPath string
	if fileio.PathExists(projectPatchPath) {
		patchPath = projectPatchPath
	} else if fileio.PathExists(portPatchPath) {
		patchPath = portPatchPath
	} else {
		return "", fmt.Errorf("patch %s not found", patchFileName)
	}

	bytes, err := os.ReadFile(patchPath)
	if err != nil {
		return "", fmt.Errorf("read patch %s: %s", patchPath, err)
	}

	return string(bytes), nil
}
