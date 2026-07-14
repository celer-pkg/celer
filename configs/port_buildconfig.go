package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"strings"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
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

	// host example: x86_64-linux-dev
	// target example: aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release
	libraryDir := expr.If(p.DevDep || p.HostDep,
		hostName+"-dev", filepath.Join(platformName, projectName, buildType),
	)

	// host example: installed/celer/x86_64-linux-dev/x264@stable.trace
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable.trace
	p.traceFile = filepath.Join(dirs.InstalledDir, "celer", "traces", libraryDir, nameVersion+".trace")

	// host example: installed/celer/x86_64-linux-dev/x264@stable.meta
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable.meta
	p.metaFile = filepath.Join(dirs.InstalledDir, "celer", "metas", libraryDir, nameVersion+".meta")

	// host example: installed/celer/x86_64-linux-dev/x264@stable
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable
	p.PackageDir = filepath.Join(dirs.WorkspaceDir, "packages", filepath.Join(libraryDir, nameVersion))

	// host example: installed/celer/x86_64-linux-dev/x264@stable
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/x264@stable
	p.InstalledDir = filepath.Join(dirs.InstalledDir, libraryDir)

	// host example: installed/celer/x86_64-linux-dev/deps/x264@stable
	// target example: installed/celer/aarch64-linux-ubuntu-22.04-gcc-11.5.0/test_project_001/release/deps/x264@stable
	p.tmpDepsDir = filepath.Join(dirs.TmpDepsDir, libraryDir)

	portConfig := buildsystems.PortConfig{
		Ctx:             p.ctx,
		LibName:         p.Name,
		LibVersion:      p.Version,
		Archive:         p.Package.Archive,
		Url:             p.Package.Url,
		Checksum:        p.Package.Checksum,
		IgnoreSubmodule: p.Package.IgnoreSubmodule,
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
			// Merge ports defined in project if exists.
			publicPort := dirs.GetPortPath(p.Name, p.Version)
			projectPort := p.portFile != "" && p.portFile != publicPort
			if fileio.PathExists(publicPort) && projectPort {
				bytes, err := os.ReadFile(p.portFile)
				if err != nil {
					return fmt.Errorf("failed to read project port -> %w", err)
				}

				var portInProject Port
				if err := toml.Unmarshal(bytes, &portInProject); err != nil {
					return fmt.Errorf("failed to unmarshal project port -> %w", err)
				}
				portInProject.ctx = p.ctx

				// Convert build type to lowercase for all build configs in project port.
				for i := range portInProject.BuildConfigs {
					portInProject.BuildConfigs[i].BuildType = strings.ToLower(portInProject.BuildConfigs[i].BuildType)
					portInProject.BuildConfigs[i].BuildType_Windows = strings.ToLower(portInProject.BuildConfigs[i].BuildType_Windows)
					portInProject.BuildConfigs[i].BuildType_Linux = strings.ToLower(portInProject.BuildConfigs[i].BuildType_Linux)
					portInProject.BuildConfigs[i].BuildType_Darwin = strings.ToLower(portInProject.BuildConfigs[i].BuildType_Darwin)
					p.mergeFromProject(index, &portInProject.BuildConfigs[i])
				}
			}

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
		"Envs", "Patches", "Dependencies", "DevDependencies", "PkgConfigToolVars",
		"PreConfigure", "CustomConfigure", "PostConfigure",
		"PreBuild", "CustomBuild", "PostBuild",
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
