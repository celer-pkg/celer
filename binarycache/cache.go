package binarycache

import (
	"bytes"
	"celer/buildsystems"
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
	Commit(nameVersion string, devDep bool) (string, error)
	GetBuildConfig(nameVersion string, devDep bool) (*buildsystems.BuildConfig, error)
	CheckHostSupported(nameVersion string) bool
}

type Port struct {
	Platform    string
	Project     string
	PortType    portType
	NameVersion string
	DevDep      bool
	Native      bool
	Parents     []string
	BuildConfig buildsystems.BuildConfig
	Callbacks   Callbacks
}

func (p Port) BuildMeta(commit string) (string, error) {
	var buffer bytes.Buffer

	// Write platform content for root port only.
	if p.PortType == portTypePort {
		p.writeDivider(&buffer, p.Parents, p.NameVersion, "platform")
		platform, err := p.Callbacks.GenPlatformTomlString()
		if err != nil {
			return "", err
		}
		buffer.WriteString(platform + "\n")
	}

	// Write port content.
	parts := strings.Split(p.NameVersion, "@")
	if len(parts) != 2 {
		return "", fmt.Errorf("invalid port name and version: %s", p.NameVersion)
	}

	portInProject := filepath.Join(dirs.ConfProjectsDir, p.Project, parts[0], parts[1], "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		return "", fmt.Errorf("port %s not found", p.NameVersion)
	}
	content, err := p.Callbacks.GenPortTomlString(p.NameVersion, p.DevDep)
	if err != nil {
		return "", fmt.Errorf("generate toml string of port %s: %s", p.NameVersion, err)
	}
	p.writeDivider(&buffer, p.Parents, p.NameVersion, "port")
	buffer.WriteString(content + "\n")

	// Write commit of port.
	if commit != "" {
		p.writeDivider(&buffer, p.Parents, p.NameVersion, "commit")
		buffer.WriteString(commit + "\n")
	} else {
		commit, err := p.Callbacks.Commit(p.NameVersion, p.DevDep)
		if err != nil {
			return "", fmt.Errorf("failed to get commit of port %s\n %s", p.NameVersion, err)
		}
		p.writeDivider(&buffer, p.Parents, p.NameVersion, "commit")
		buffer.WriteString(commit + "\n")
	}

	// Write content of patches.
	for _, patch := range p.BuildConfig.Patches {
		content, err := p.readPatch(p.NameVersion, patch)
		if err != nil {
			return "", fmt.Errorf("read patch %s: %s", patch, err)
		}
		p.writeDivider(&buffer, p.Parents, p.NameVersion, fmt.Sprintf("patch: %s", patch))
		buffer.WriteString(content + "\n")
	}

	// Write content of dev_dependencies.
	for _, nameVersion := range p.BuildConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (p.DevDep || p.Native) && p.NameVersion == nameVersion {
			continue
		}

		// Skip if not supported.
		if !p.Callbacks.CheckHostSupported(nameVersion) {
			continue
		}

		buildConfig, err := p.Callbacks.GetBuildConfig(nameVersion, true)
		if err != nil {
			return "", fmt.Errorf("get build config of dependency %s: %s", nameVersion, err)
		}

		port := Port{
			Platform:    p.Platform,
			PortType:    portTypeDevDependency,
			NameVersion: nameVersion,
			Project:     p.Project,
			DevDep:      true,
			Parents: append(p.Parents, expr.If(len(p.Parents) == 0,
				p.NameVersion, fmt.Sprintf("dev_dependency: %s", p.NameVersion))),
			BuildConfig: *buildConfig,
			Callbacks:   p.Callbacks,
		}

		content, err := port.BuildMeta("")
		if err != nil {
			return "", fmt.Errorf("fill content of dev_dependency %s: %s", nameVersion, err)
		}
		buffer.WriteString(string(content))
	}

	// Write content of dependencies.
	for _, nameVersion := range p.BuildConfig.Dependencies {
		buildConfig, err := p.Callbacks.GetBuildConfig(nameVersion, false)
		if err != nil {
			return "", fmt.Errorf("get build config of dependency %s: %s", nameVersion, err)
		}

		port := Port{
			Platform:    p.Platform,
			PortType:    portTypeDependency,
			NameVersion: nameVersion,
			Project:     p.Project,
			DevDep:      p.DevDep,
			Parents: append(p.Parents, expr.If(len(p.Parents) == 0,
				p.NameVersion, fmt.Sprintf("dependency: %s", p.NameVersion))),
			BuildConfig: *buildConfig,
			Callbacks:   p.Callbacks,
		}

		content, err := port.BuildMeta("")
		if err != nil {
			return "", fmt.Errorf("fill content of dependency %s: %s", nameVersion, err)
		}
		buffer.WriteString(string(content))
	}

	return buffer.String(), nil
}

func (p Port) writeDivider(buffer *bytes.Buffer, parents []string, nameVersion, what string) {
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

	defaultPatchPath := filepath.Join(dirs.PortsDir, parts[0], parts[1], patchFileName)
	preferedPatchPath := filepath.Join(dirs.ConfProjectsDir, p.Project, parts[0], parts[1], patchFileName)

	var patchPath string
	if fileio.PathExists(preferedPatchPath) {
		patchPath = preferedPatchPath
	} else if fileio.PathExists(defaultPatchPath) {
		patchPath = defaultPatchPath
	} else {
		return "", fmt.Errorf("patch %s not found", patchFileName)
	}

	bytes, err := os.ReadFile(patchPath)
	if err != nil {
		return "", fmt.Errorf("read patch %s: %s", patchPath, err)
	}

	return string(bytes), nil
}
