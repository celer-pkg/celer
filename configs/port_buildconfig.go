package configs

import (
	"celer/buildsystems"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"

	"github.com/BurntSushi/toml"
)

func (p *Port) initBuildConfig(nameVersion string) error {
	buildType := p.ctx.BuildType()
	hostName := p.ctx.Platform().GetHostName()
	platformProject := fmt.Sprintf("%s@%s@%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType)

	buildFolder := expr.If(p.DevDep,
		filepath.Join(nameVersion, hostName+"-dev"),
		filepath.Join(nameVersion, fmt.Sprintf("%s-%s-%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType)),
	)
	libraryFolder := expr.If(p.DevDep, hostName+"-dev",
		fmt.Sprintf("%s@%s@%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType),
	)
	packageFolder := expr.If(p.DevDep, nameVersion+"@"+hostName+"-dev",
		fmt.Sprintf("%s@%s@%s@%s", nameVersion, p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType),
	)
	p.traceFile = expr.If(p.DevDep,
		filepath.Join(dirs.InstalledDir, "celer", "trace", nameVersion+"@"+hostName+"-dev.trace"),
		filepath.Join(dirs.InstalledDir, "celer", "trace", nameVersion+"@"+platformProject+".trace"),
	)
	p.metaFile = expr.If(p.DevDep,
		filepath.Join(dirs.InstalledDir, "celer", "meta", nameVersion+"@"+hostName+"-dev.meta"),
		filepath.Join(dirs.InstalledDir, "celer", "meta", nameVersion+"@"+platformProject+".meta"),
	)

	p.PackageDir = filepath.Join(dirs.WorkspaceDir, "packages", packageFolder)
	p.InstalledDir = filepath.Join(dirs.InstalledDir, libraryFolder)
	p.tmpDepsDir = filepath.Join(dirs.TmpDepsDir, libraryFolder)

	portConfig := buildsystems.PortConfig{
		Toolchain:       p.toolchain(),
		LibName:         p.Name,
		LibVersion:      p.Version,
		Archive:         p.Package.Archive,
		Url:             p.Package.Url,
		IgnoreSubmodule: p.Package.IgnoreSubmodule,
		ProjectName:     p.ctx.Project().GetName(),
		HostName:        p.ctx.Platform().GetHostName(),
		SrcDir:          filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src"),
		BuildDir:        filepath.Join(dirs.WorkspaceDir, "buildtrees", buildFolder),
		PackageDir:      p.PackageDir,
		LibraryFolder:   libraryFolder,
		DevDep:          p.DevDep,
		Jobs:            p.ctx.Jobs(),
		RepoDir:         filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src"),
	}

	// Source folder may be a inner dir.
	if p.Package.SrcDir != "" {
		portConfig.SrcDir = filepath.Join(portConfig.SrcDir, p.Package.SrcDir)
	}

	if p.ctx.RootFS() != nil {
		portConfig.IncludeDirs = p.ctx.RootFS().GetIncludeDirs()
		portConfig.LibDirs = p.ctx.RootFS().GetLibDirs()
	}

	if len(p.BuildConfigs) > 0 {
		for index := range p.BuildConfigs {
			// Merge ports defined in project if exists.
			portInPorts := filepath.Join(dirs.PortsDir, p.Name, p.Version, "port.toml")
			portInProject := filepath.Join(dirs.ConfProjectsDir, p.ctx.Project().GetName(), p.Name, p.Version, "port.toml")
			if fileio.PathExists(portInPorts) && fileio.PathExists(portInProject) {
				bytes, err := os.ReadFile(portInProject)
				if err != nil {
					return fmt.Errorf("failed to read project port.\n %w", err)
				}

				var portInProject Port
				if err := toml.Unmarshal(bytes, &portInProject); err != nil {
					return fmt.Errorf("failed to unmarshal project port.\n %w", err)
				}

				portInProject.ctx = p.ctx
				p.mergeBuildConfig(index, portInProject.MatchedConfig)
			}

			p.BuildConfigs[index].Ctx = p.ctx
			p.BuildConfigs[index].PortConfig = portConfig
			p.BuildConfigs[index].DevDep = p.DevDep
			p.BuildConfigs[index].Optimize = p.ctx.Optimize(p.MatchedConfig.BuildSystem, portConfig.Toolchain.Name)
			if err := p.BuildConfigs[index].InitBuildSystem(p.BuildConfigs[index].Optimize); err != nil {
				return err
			}
		}
	}

	return nil
}

func (p *Port) mergeBuildConfig(index int, overrideConfig *buildsystems.BuildConfig) {
	if overrideConfig == nil {
		return
	}

	if overrideConfig.BuildSystem != "" {
		p.BuildConfigs[index].BuildSystem = overrideConfig.BuildSystem
	}
	if len(overrideConfig.BuildTools) > 0 {
		p.BuildConfigs[index].BuildTools = overrideConfig.BuildTools
	}
	if overrideConfig.LibraryType != "" {
		p.BuildConfigs[index].LibraryType = overrideConfig.LibraryType
	}
	if overrideConfig.CStandard != "" {
		p.BuildConfigs[index].CStandard = overrideConfig.CStandard
	}
	if overrideConfig.CXXStandard != "" {
		p.BuildConfigs[index].CXXStandard = overrideConfig.CXXStandard
	}
	if len(overrideConfig.Envs) > 0 {
		p.BuildConfigs[index].Envs = overrideConfig.Envs
	}
	if overrideConfig.Patches != nil {
		p.BuildConfigs[index].Patches = overrideConfig.Patches
	}
	if len(overrideConfig.AutogenOptions) > 0 {
		p.BuildConfigs[index].AutogenOptions = overrideConfig.AutogenOptions
	}
	if len(overrideConfig.Options) > 0 {
		p.BuildConfigs[index].Options = overrideConfig.Options
	}
	if len(overrideConfig.Dependencies) > 0 {
		p.BuildConfigs[index].Dependencies = overrideConfig.Dependencies
	}
	if len(overrideConfig.DevDependencies) > 0 {
		p.BuildConfigs[index].DevDependencies = overrideConfig.DevDependencies
	}
	if len(overrideConfig.PreConfigure) > 0 {
		p.BuildConfigs[index].PreConfigure = overrideConfig.PreConfigure
	}
	if len(overrideConfig.PostConfigure) > 0 {
		p.BuildConfigs[index].PostConfigure = overrideConfig.PostConfigure
	}
	if len(overrideConfig.PreBuild) > 0 {
		p.BuildConfigs[index].PreBuild = overrideConfig.PreBuild
	}
	if len(overrideConfig.FixBuild) > 0 {
		p.BuildConfigs[index].FixBuild = overrideConfig.FixBuild
	}
	if len(overrideConfig.PostBuild) > 0 {
		p.BuildConfigs[index].PostBuild = overrideConfig.PostBuild
	}
	if len(overrideConfig.PreInstall) > 0 {
		p.BuildConfigs[index].PreInstall = overrideConfig.PreInstall
	}
	if len(overrideConfig.PostInstall) > 0 {
		p.BuildConfigs[index].PostInstall = overrideConfig.PostInstall
	}
}
