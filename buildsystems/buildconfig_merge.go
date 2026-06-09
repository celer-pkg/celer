package buildsystems

import (
	"reflect"
	"runtime"
	"slices"
	"strings"
)

// mergeConfig merges the platform-specific fields into the BuildConfig struct.
func (b *BuildConfig) mergeConfig() {
	platformSuffix := getPlatformSuffix(b.buildTarget())
	if platformSuffix == "" {
		return
	}

	bVal := reflect.ValueOf(b).Elem()

	// List of all fields that need platform-specific merging
	fields := []string{
		"BuildSystem", "CMakeGenerator", "BuildTools", "LibraryType",
		"BuildShared", "BuildStatic", "CStandard", "CXXStandard", "BuildType",
		"Envs", "Patches", "Dependencies", "DevDependencies",
		"PreConfigure", "CustomConfigure", "PostConfigure",
		"PreBuild", "FixBuild", "CustomBuild", "PostBuild",
		"PreInstall", "CustomInstall", "PostInstall",
		"AutogenOptions", "Options",
	}

	for _, fieldName := range fields {
		platformFieldName := fieldName + platformSuffix
		baseField := bVal.FieldByName(fieldName)
		platformField := bVal.FieldByName(platformFieldName)

		if !baseField.IsValid() || !platformField.IsValid() || !baseField.CanSet() {
			continue
		}

		// Merge based on field type
		switch baseField.Kind() {
		case reflect.String:
			if platformField.String() != "" {
				baseField.SetString(platformField.String())
			}
		case reflect.Slice:
			if platformField.Len() > 0 {
				baseField.Set(platformField)
			}
		}
	}

	// Special handling for BuildInSource (pointer type)
	buildInSourceField := bVal.FieldByName("BuildInSource" + platformSuffix)
	if buildInSourceField.IsValid() && !buildInSourceField.IsNil() {
		bVal.FieldByName("BuildInSource").SetBool(buildInSourceField.Elem().Bool())
	}

	// Ensure BuildType is always lowercase after platform-specific merge.
	buildTypeField := bVal.FieldByName("BuildType")
	if buildTypeField.IsValid() && buildTypeField.CanSet() {
		if buildType := buildTypeField.String(); buildType != "" {
			buildTypeField.SetString(strings.ToLower(buildType))
		}
	}
}

// getPlatformSuffix returns the platform suffix for field names
func getPlatformSuffix(target string) string {
	switch target {
	case "windows":
		return "_Windows"
	case "linux":
		return "_Linux"
	case "darwin":
		return "_Darwin"
	default:
		return ""
	}
}

func (b BuildConfig) buildTarget() string {
	systemName := strings.ToLower(b.Ctx.Platform().GetToolchain().GetSystemName())
	if !slices.Contains([]string{"windows", "linux", "darwin"}, systemName) {
		return runtime.GOOS
	}
	return systemName
}
