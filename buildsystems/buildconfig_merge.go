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

	// Override string fields with system-specific suffix.
	stringFields := []string{
		"BuildSystem", "CMakeGenerator", "BuildTools",
		"CStandard", "CXXStandard", "BuildType",
		"Envs", "Patches", "Dependencies", "DevDependencies",
		"PreConfigure", "CustomConfigure", "PostConfigure",
		"PreBuild", "FixBuild", "CustomBuild", "PostBuild",
		"PreInstall", "CustomInstall", "PostInstall",
		"AutogenOptions", "DisableDevCache", "Options",
	}

	for _, fieldName := range stringFields {
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

	// Override bool fields with system-specific suffix.
	// The base field is bool for BuildInSource and *bool for BuildShared/BuildStatic;
	// the per-platform variants are always *bool so "not configured" (nil) can be told
	// apart from "configured false".
	boolFields := []string{"BuildInSource", "BuildShared", "BuildStatic"}
	for _, name := range boolFields {
		variant := bVal.FieldByName(name + platformSuffix)
		if !variant.IsValid() || variant.IsNil() {
			continue
		}

		value := variant.Elem().Bool()
		field := bVal.FieldByName(name)

		switch field.Kind() {
		case reflect.Bool:
			field.SetBool(value)
		case reflect.Pointer:
			field.Set(reflect.ValueOf(&value))
		}
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
