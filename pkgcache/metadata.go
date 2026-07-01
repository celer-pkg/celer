package pkgcache

import (
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// metaCache caches pkgcache-level buildMeta results keyed by nameVersion|native
var metaCache sync.Map // key: string -> metaResult

type metaResult struct {
	meta string
	err  error
}

// ResetMetaCache clears the pkgcache-level buildMeta cache. Called alongside
// configs.ResetMetaCache at the start of each celer command.
func ResetMetaCache() {
	metaCache.Range(func(k, v any) bool {
		metaCache.Delete(k)
		return true
	})
}

type portType int

const (
	portTypePort portType = iota
	portTypeDependency
	portTypeDevDependency
)

type Callbacks interface {
	GenPortTomlString(nameVersion string, native bool) (string, error)
	GenPlatformTomlString() (string, error)
	GenBuildToolsVersions(tools []string) (string, error)
	GetCommitHash(nameVersion string, native bool) (string, error)
	GetBuildConfig(nameVersion string, native bool) (*BuildConfig, error)
	CheckHostSupported(nameVersion string) bool
}

type BuildConfig struct {
	Patches         []string
	Dependencies    []string
	DevDependencies []string
	BuildTools      []string
	PortFile        string
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

func (p Port) BuildMeta() (string, error) {
	var buffer bytes.Buffer

	// Collect buildtool infos.
	if p.PortType == portTypePort {
		buildTools, err := p.collectBuildTools(map[string]struct{}{}, map[string]struct{}{})
		if err != nil {
			return "", fmt.Errorf("collect build tools -> %w", err)
		}

		toolVersions, err := p.Callbacks.GenBuildToolsVersions(buildTools)
		if err != nil {
			return "", err
		}
		if toolVersions != "" {
			p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "build tools versions")
			fmt.Fprintf(&buffer, "%s\n\n", toolVersions)
		}
	}

	content, err := p.buildMeta()
	if err != nil {
		return "", err
	}
	buffer.WriteString(content)
	return buffer.String(), nil
}

func (p Port) buildMeta() (string, error) {
	key := fmt.Sprintf("%s|%t|%d|%s", p.NameVersion, p.DevDep || p.HostDev, p.PortType, strings.Join(p.Parents, ">"))
	if value, ok := metaCache.Load(key); ok {
		result := value.(metaResult)
		return result.meta, result.err
	}

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
	}

	// Write port content.
	parts := strings.Split(p.NameVersion, "@")
	if len(parts) != 2 {
		return "", fmt.Errorf("invalid port name and version: %s", p.NameVersion)
	}

	content, err := p.Callbacks.GenPortTomlString(p.NameVersion, p.DevDep)
	if err != nil {
		return "", fmt.Errorf("generate toml content of port %s -> %w", p.NameVersion, err)
	}
	p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, "port")
	fmt.Fprintf(&buffer, "%s\n", content)

	// Write content of patches.
	for _, patch := range p.BuildConfig.Patches {
		content, err := p.readPatch(patch)
		if err != nil {
			return "", fmt.Errorf("read patch %s -> %w", patch, err)
		}
		p.writeSectionTitle(&buffer, p.Parents, p.NameVersion, fmt.Sprintf("patch: %s", patch))
		fmt.Fprintf(&buffer, "%s\n", content)
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

		content, err := port.buildMeta()
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

		content, err := port.buildMeta()
		if err != nil {
			return "", fmt.Errorf("fill content of dependency %s -> %w", nameVersion, err)
		}
		buffer.WriteString(string(content))
	}

	result := buffer.String()
	metaCache.Store(key, metaResult{meta: result})
	return result, nil
}

func (p Port) collectBuildTools(visitedPorts, seenTools map[string]struct{}) ([]string, error) {
	key := fmt.Sprintf("%s|%t", p.NameVersion, p.DevDep || p.HostDev)
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

func (p Port) readPatch(patchFileName string) (string, error) {
	patchFilePath := filepath.Join(filepath.Dir(p.BuildConfig.PortFile), patchFileName)
	if !fileio.PathExists(patchFilePath) {
		return "", fmt.Errorf("patch %s not found", patchFileName)
	}

	bytes, err := os.ReadFile(patchFilePath)
	if err != nil {
		return "", fmt.Errorf("read patch %s: %s", patchFilePath, err)
	}

	return string(bytes), nil
}
