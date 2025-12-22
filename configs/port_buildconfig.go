package configs

import (
	"celer/buildsystems"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"strings"

	"github.com/BurntSushi/toml"
)

func (p *Port) initBuildConfig(nameVersion string) error {
	buildType := p.ctx.BuildType()
	hostName := p.ctx.Platform().GetHostName()
	platformProject := fmt.Sprintf("%s@%s@%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType)

	buildFolder := expr.If(p.DevDep || p.Native,
		filepath.Join(nameVersion, hostName+"-dev"),
		filepath.Join(nameVersion, fmt.Sprintf("%s-%s-%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType)),
	)
	libraryFolder := expr.If(p.DevDep || p.Native,
		hostName+"-dev",
		fmt.Sprintf("%s@%s@%s", p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType),
	)
	packageFolder := expr.If(p.DevDep || p.Native,
		nameVersion+"@"+hostName+"-dev",
		fmt.Sprintf("%s@%s@%s@%s", nameVersion, p.ctx.Platform().GetName(), p.ctx.Project().GetName(), buildType),
	)
	p.traceFile = expr.If(p.DevDep || p.Native,
		filepath.Join(dirs.InstalledDir, "celer", "trace", nameVersion+"@"+hostName+"-dev.trace"),
		filepath.Join(dirs.InstalledDir, "celer", "trace", nameVersion+"@"+platformProject+".trace"),
	)
	p.metaFile = expr.If(p.DevDep || p.Native,
		filepath.Join(dirs.InstalledDir, "celer", "meta", nameVersion+"@"+hostName+"-dev.meta"),
		filepath.Join(dirs.InstalledDir, "celer", "meta", nameVersion+"@"+platformProject+".meta"),
	)

	p.PackageDir = filepath.Join(dirs.WorkspaceDir, "packages", packageFolder)
	p.InstalledDir = filepath.Join(dirs.InstalledDir, libraryFolder)
	p.tmpDepsDir = filepath.Join(dirs.TmpDepsDir, libraryFolder)

	portConfig := buildsystems.PortConfig{
		Ctx:             p.ctx,
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
		Native:          p.Native || p.DevDep,
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
		toolchain := p.ctx.Platform().GetToolchain()

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

				// Convert build type to lowercase for all build configs in project port.
				for i := range portInProject.BuildConfigs {
					portInProject.BuildConfigs[i].BuildType = strings.ToLower(portInProject.BuildConfigs[i].BuildType)
					portInProject.BuildConfigs[i].BuildType_Windows = strings.ToLower(portInProject.BuildConfigs[i].BuildType_Windows)
					portInProject.BuildConfigs[i].BuildType_Linux = strings.ToLower(portInProject.BuildConfigs[i].BuildType_Linux)
					portInProject.BuildConfigs[i].BuildType_Darwin = strings.ToLower(portInProject.BuildConfigs[i].BuildType_Darwin)
				}

				p.mergeFromProject(index, portInProject.MatchedConfig)
			}

			p.BuildConfigs[index].Ctx = p.ctx
			p.BuildConfigs[index].PortConfig = portConfig
			p.BuildConfigs[index].DevDep = p.DevDep
			p.BuildConfigs[index].Native = p.Native || p.DevDep
			p.BuildConfigs[index].Optimize = p.ctx.Optimize(p.MatchedConfig.BuildSystem, toolchain.GetName())
			if err := p.BuildConfigs[index].InitBuildSystem(p.BuildConfigs[index].Optimize); err != nil {
				return err
			}
		}

		// Update matched config.
		p.MatchedConfig = p.findMatchedConfig(p.ctx.BuildType())
	}

	return nil
}

func (p *Port) mergeFromProject(index int, overrideConfig *buildsystems.BuildConfig) {
	if overrideConfig == nil {
		return
	}

	// Helper function to merge field with platform variants.
	mergeField := func(fieldName string) {
		// Get field values from both configs.
		srcVal := reflect.ValueOf(overrideConfig).Elem()
		dstVal := reflect.ValueOf(&p.BuildConfigs[index]).Elem()

		// Merge base field.
		if srcField := srcVal.FieldByName(fieldName); srcField.IsValid() {
			dstField := dstVal.FieldByName(fieldName)
			if dstField.IsValid() && dstField.CanSet() {
				if !isZeroValue(srcField) {
					dstField.Set(srcField)
				}
			}
		}

		// Merge platform-specific variants.
		for _, suffix := range []string{"_Windows", "_Linux", "_Darwin"} {
			platformFieldName := fieldName + suffix
			if srcField := srcVal.FieldByName(platformFieldName); srcField.IsValid() {
				dstField := dstVal.FieldByName(platformFieldName)
				if dstField.IsValid() && dstField.CanSet() {
					if !isZeroValue(srcField) {
						dstField.Set(srcField)
					}
				}
			}
		}
	}

	// List of all fields that need to be merged.
	fields := []string{
		"BuildSystem", "CMakeGenerator", "BuildTools", "LibraryType",
		"BuildShared", "BuildStatic", "CStandard", "CXXStandard", "BuildType",
		"Envs", "Patches", "Dependencies", "DevDependencies",
		"PreConfigure", "CustomConfigure", "PostConfigure",
		"PreBuild", "FixBuild", "CustomBuild", "PostBuild",
		"PreInstall", "CustomInstall", "PostInstall",
		"AutogenOptions", "Options",
	}

	for _, field := range fields {
		mergeField(field)
	}
}

// isZeroValue checks if a reflect.Value is the zero value for its type.
func isZeroValue(v reflect.Value) bool {
	switch v.Kind() {
	case reflect.String:
		return v.String() == ""
	case reflect.Slice, reflect.Array:
		return v.Len() == 0
	case reflect.Pointer:
		return v.IsNil()
	case reflect.Bool:
		return !v.Bool()
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		return v.Int() == 0
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		return v.Uint() == 0
	case reflect.Float32, reflect.Float64:
		return v.Float() == 0
	default:
		return v.IsZero()
	}
}
