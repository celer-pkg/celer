package buildsystems

import (
	"reflect"
	"runtime"
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
		"BuildShared", "BuildStatic", "CStandard", "CXXStandard",
		"Envs", "Patches", "Dependencies", "DevDependencies",
		"PreConfigure", "FreeStyleConfigure", "PostConfigure",
		"PreBuild", "FixBuild", "FreeStyleBuild", "PostBuild",
		"PreInstall", "FreeStyleInstall", "PostInstall",
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
	switch {
	case b.Pattern == "" || b.Pattern == "*":
		return runtime.GOOS

	case strings.Contains(b.Pattern, "windows"):
		return "windows"

	case strings.Contains(b.Pattern, "linux"):
		return "linux"

	case strings.Contains(b.Pattern, "darwin"):
		return "darwin"

	default:
		panic("unknown pattern: " + b.Pattern)
	}
}
