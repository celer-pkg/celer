package configs

import (
	"fmt"
	"hash/fnv"
	"path/filepath"
	"runtime"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
)

func (p *Port) initBuildConfig(nameVersion string) error {
	buildType := p.ctx.BuildType()
	hostName := p.ctx.Platform().GetHostName()
	platformName := p.ctx.Platform().GetName()
	projectName := p.ctx.Project().GetName()

	// host example: x86_64-linux-dev
	// target example: aarch64-linux-ubuntu-22.04-gcc-11.5.0-test_project_001-release
	buildFolder := expr.If(p.DevDep || p.HostDep,
		filepath.Join(nameVersion, hostName+"-dev"),
		filepath.Join(nameVersion, fmt.Sprintf("%s-%s-%s", platformName, projectName, buildType)),
	)
	if runtime.GOOS == "windows" && !p.DevDep && !p.HostDep && p.MatchedConfig.ShortenBuildFolder {
		buildFolder = p.shortenBuildFolder(buildFolder, nameVersion, platformName, projectName, buildType)
	}

	// host example: x86_64-linux-dev
	// target example: aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release
	libraryDir := expr.If(p.DevDep || p.HostDep,
		hostName+"-dev", filepath.Join(platformName, projectName, buildType),
	)

	// host example: installed/celer/x86_64-linux-dev/x264@stable.trace
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable.trace
	p.traceFile = filepath.Join(dirs.InstalledDir, "infos", "traces", libraryDir, nameVersion+".trace")

	// host example: installed/celer/x86_64-linux-dev/x264@stable.meta
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable.meta
	p.metaFile = filepath.Join(dirs.InstalledDir, "infos", "metas", libraryDir, nameVersion+".meta")

	// host example: installed/celer/x86_64-linux-dev/x264@stable
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable
	p.PackageDir = filepath.Join(dirs.WorkspaceDir, "packages", filepath.Join(libraryDir, nameVersion))

	// host example: installed/celer/x86_64-linux-dev/x264@stable
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable
	p.InstalledDir = filepath.Join(dirs.InstalledDir, libraryDir)

	portConfig := buildsystems.PortConfig{
		Ctx:             p.ctx,
		LibName:         p.Name,
		LibVersion:      p.Version,
		Archive:         p.Package.Archive,
		Url:             p.Package.Url,
		Checksum:        p.Package.Checksum,
		IgnoreSubmodule: p.Package.IgnoreSubmodule,
		IgnoreLFS:       p.Package.IgnoreLFS,
		ProjectName:     projectName,
		HostName:        hostName,
		SrcDir:          filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src"),
		BuildDir:        filepath.Join(dirs.WorkspaceDir, "buildtrees", buildFolder),
		PackageDir:      p.PackageDir,
		LibraryDir:      libraryDir,
		DevDep:          p.DevDep,
		HostDev:         p.HostDep || p.DevDep,
		Jobs:            p.ctx.Jobs(),
		RepoDir:         filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src"),
		PortFile:        p.portFile,
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
			p.BuildConfigs[index].Ctx = p.ctx
			p.BuildConfigs[index].ExprVars = p.exprVars
			p.BuildConfigs[index].PortConfig = portConfig
			p.BuildConfigs[index].DevDep = p.DevDep
			p.BuildConfigs[index].HostDev = p.HostDep || p.DevDep
			if err := p.BuildConfigs[index].InitBuildSystem(); err != nil {
				return err
			}
		}

		// Update matched config.
		matchedConfig, err := p.findMatchedConfig(p.ctx.BuildType())
		if err != nil {
			return err
		}
		p.MatchedConfig = matchedConfig

		// Register common fixed variables.
		p.registerExprVars(matchedConfig)
	}

	return nil
}

const (
	windowsMaxObjPath           = 260
	windowsShortBuildObjReserve = 210
)

func (p *Port) windowsBuildDirFits(buildFolder string, objReserve int) bool {
	buildDir := filepath.Join(dirs.WorkspaceDir, "buildtrees", buildFolder)
	return len(buildDir)+1+objReserve <= windowsMaxObjPath
}

func (p *Port) shortenBuildFolder(longFolder, nameVersion, platformName, projectName, buildType string) string {
	if p.windowsBuildDirFits(longFolder, windowsShortBuildObjReserve) {
		return longFolder
	}

	hash := fnv.New32a()
	fmt.Fprintf(hash, "%s\x00%s\x00%s", platformName, projectName, buildType)
	tag := fmt.Sprintf("%06x", hash.Sum32()&0xffffff)
	bt := expr.If(buildType == "", "r", buildType[:1])

	// Prefer keeping platform in the folder name when it still fits.
	withPlatform := filepath.Join(nameVersion, fmt.Sprintf("%s-%s-%s", platformName, tag, bt))
	if p.windowsBuildDirFits(withPlatform, windowsShortBuildObjReserve) {
		return withPlatform
	}
	return filepath.Join(nameVersion, fmt.Sprintf("%s-%s", tag, bt))
}
