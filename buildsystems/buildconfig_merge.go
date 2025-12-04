package buildsystems

import (
	"celer/pkgs/expr"
	"runtime"
	"strings"
)

// mergeConfig merges the platform-specific fields into the BuildConfig struct.
func (b *BuildConfig) mergeConfig() {
	switch b.buildTarget() {
	case "windows":
		b.BuildSystem = expr.If(b.BuildSystem_Windows != "", b.BuildSystem_Windows, b.BuildSystem)
		b.CMakeGenerator = expr.If(b.CMakeGenerator_Windows != "", b.CMakeGenerator_Windows, b.CMakeGenerator)
		b.BuildTools = expr.If(len(b.BuildTools_Windows) > 0, b.BuildTools_Windows, b.BuildTools)
		b.LibraryType = expr.If(b.LibraryType_Windows != "", b.LibraryType_Windows, b.LibraryType)
		b.BuildShared = expr.If(len(b.BuildShared_Windows) > 0, b.BuildShared_Windows, b.BuildShared)
		b.BuildStatic = expr.If(len(b.BuildStatic_Windows) > 0, b.BuildStatic_Windows, b.BuildShared)
		b.CStandard = expr.If(b.CStandard_Windows != "", b.CStandard_Windows, b.CStandard)
		b.CXXStandard = expr.If(b.CXXStandard_Windows != "", b.CXXStandard_Windows, b.CXXStandard)
		b.Envs = expr.If(len(b.Envs_Windows) > 0, b.Envs_Windows, b.Envs)
		b.Patches = expr.If(len(b.Patches_Windows) > 0, b.Patches_Windows, b.Patches)
		if b.BuildInSource_Windows != nil {
			b.BuildInSource = *b.BuildInSource_Windows
		}
		b.Dependencies = expr.If(len(b.Dependencies_Windows) > 0, b.Dependencies_Windows, b.Dependencies)
		b.DevDependencies = expr.If(len(b.DevDependencies_Windows) > 0, b.DevDependencies_Windows, b.DevDependencies)
		b.PreConfigure = expr.If(len(b.PreConfigure_Windows) > 0, b.PreConfigure_Windows, b.PreConfigure)
		b.FreeStyleConfigure = expr.If(len(b.FreeStyleConfigure_Windows) > 0, b.FreeStyleConfigure_Windows, b.FreeStyleConfigure)
		b.PostConfigure = expr.If(len(b.PostConfigure_Windows) > 0, b.PostConfigure_Windows, b.PostConfigure)
		b.PreBuild = expr.If(len(b.PreBuild_Windows) > 0, b.PreBuild_Windows, b.PreBuild)
		b.FixBuild = expr.If(len(b.FixBuild_Windows) > 0, b.FixBuild_Windows, b.FixBuild)
		b.FreeStyleBuild = expr.If(len(b.FreeStyleBuild_Windows) > 0, b.FreeStyleBuild_Windows, b.FreeStyleBuild)
		b.PostBuild = expr.If(len(b.PostBuild_Windows) > 0, b.PostBuild_Windows, b.PostBuild)
		b.PreInstall = expr.If(len(b.PreInstall_Windows) > 0, b.PreInstall_Windows, b.PreInstall)
		b.FreeStyleInstall = expr.If(len(b.FreeStyleInstall_Windows) > 0, b.FreeStyleInstall_Windows, b.FreeStyleInstall)
		b.PostInstall = expr.If(len(b.PostInstall_Windows) > 0, b.PostInstall_Windows, b.PostInstall)
		b.AutogenOptions = expr.If(len(b.AutogenOptions_Windows) > 0, b.AutogenOptions_Windows, b.AutogenOptions)
		b.Options = expr.If(len(b.Options_Windows) > 0, b.Options_Windows, b.Options)

	case "linux":
		b.BuildSystem = expr.If(b.BuildSystem_Linux != "", b.BuildSystem_Linux, b.BuildSystem)
		b.CMakeGenerator = expr.If(b.CMakeGenerator_Linux != "", b.CMakeGenerator_Linux, b.CMakeGenerator)
		b.BuildTools = expr.If(len(b.BuildTools_Linux) > 0, b.BuildTools_Linux, b.BuildTools)
		b.LibraryType = expr.If(b.LibraryType_Linux != "", b.LibraryType_Linux, b.LibraryType)
		b.BuildShared = expr.If(len(b.BuildShared_Linux) > 0, b.BuildShared_Linux, b.BuildShared)
		b.BuildStatic = expr.If(len(b.BuildStatic_Linux) > 0, b.BuildStatic_Linux, b.BuildStatic)
		b.CStandard = expr.If(b.CStandard_Linux != "", b.CStandard_Linux, b.CStandard)
		b.CXXStandard = expr.If(b.CXXStandard_Linux != "", b.CXXStandard_Linux, b.CXXStandard)
		b.Envs = expr.If(len(b.Envs_Linux) > 0, b.Envs_Linux, b.Envs)
		b.Patches = expr.If(len(b.Patches_Linux) > 0, b.Patches_Linux, b.Patches)
		if b.BuildInSource_Linux != nil {
			b.BuildInSource = *b.BuildInSource_Linux
		}
		b.Dependencies = expr.If(len(b.Dependencies_Linux) > 0, b.Dependencies_Linux, b.Dependencies)
		b.DevDependencies = expr.If(len(b.DevDependencies_Linux) > 0, b.DevDependencies_Linux, b.DevDependencies)
		b.PreConfigure = expr.If(len(b.PreConfigure_Linux) > 0, b.PreConfigure_Linux, b.PreConfigure)
		b.FreeStyleConfigure = expr.If(len(b.FreeStyleConfigure_Linux) > 0, b.FreeStyleConfigure_Linux, b.FreeStyleConfigure)
		b.PostConfigure = expr.If(len(b.PostConfigure_Linux) > 0, b.PostConfigure_Linux, b.PostConfigure)
		b.PreBuild = expr.If(len(b.PreBuild_Linux) > 0, b.PreBuild_Linux, b.PreBuild)
		b.FixBuild = expr.If(len(b.FixBuild_Linux) > 0, b.FixBuild_Linux, b.FixBuild)
		b.FreeStyleBuild = expr.If(len(b.FreeStyleBuild_Linux) > 0, b.FreeStyleBuild_Linux, b.FreeStyleBuild)
		b.PostBuild = expr.If(len(b.PostBuild_Linux) > 0, b.PostBuild_Linux, b.PostBuild)
		b.PreInstall = expr.If(len(b.PreInstall_Linux) > 0, b.PreInstall_Linux, b.PreInstall)
		b.FreeStyleInstall = expr.If(len(b.FreeStyleInstall_Linux) > 0, b.FreeStyleInstall_Linux, b.FreeStyleInstall)
		b.PostInstall = expr.If(len(b.PostInstall_Linux) > 0, b.PostInstall_Linux, b.PostInstall)
		b.AutogenOptions = expr.If(len(b.AutogenOptions_Linux) > 0, b.AutogenOptions_Linux, b.AutogenOptions)
		b.Options = expr.If(len(b.Options_Linux) > 0, b.Options_Linux, b.Options)

	case "darwin":
		b.BuildSystem = expr.If(b.BuildSystem_Darwin != "", b.BuildSystem_Darwin, b.BuildSystem)
		b.CMakeGenerator = expr.If(b.CMakeGenerator_Darwin != "", b.CMakeGenerator_Darwin, b.CMakeGenerator)
		b.BuildTools = expr.If(len(b.BuildTools_Darwin) > 0, b.BuildTools_Darwin, b.BuildTools)
		b.LibraryType = expr.If(b.LibraryType_Darwin != "", b.LibraryType_Darwin, b.LibraryType)
		b.BuildShared = expr.If(len(b.BuildShared_Darwin) > 0, b.BuildShared_Darwin, b.BuildShared)
		b.BuildStatic = expr.If(len(b.BuildStatic_Darwin) > 0, b.BuildStatic_Darwin, b.BuildShared)
		b.CStandard = expr.If(b.CStandard_Darwin != "", b.CStandard_Darwin, b.CStandard)
		b.CXXStandard = expr.If(b.CXXStandard_Darwin != "", b.CXXStandard_Darwin, b.CXXStandard)
		b.Envs = expr.If(len(b.Envs_Darwin) > 0, b.Envs_Darwin, b.Envs)
		b.Patches = expr.If(len(b.Patches_Darwin) > 0, b.Patches_Darwin, b.Patches)
		if b.BuildInSource_Linux != nil {
			b.BuildInSource = *b.BuildInSource_Darwin
		}
		b.Dependencies = expr.If(len(b.Dependencies_Darwin) > 0, b.Dependencies_Darwin, b.Dependencies)
		b.DevDependencies = expr.If(len(b.DevDependencies_Darwin) > 0, b.DevDependencies_Darwin, b.DevDependencies)
		b.PreConfigure = expr.If(len(b.PreConfigure_Darwin) > 0, b.PreConfigure_Darwin, b.PreConfigure)
		b.FreeStyleConfigure = expr.If(len(b.FreeStyleConfigure_Darwin) > 0, b.FreeStyleConfigure_Darwin, b.FreeStyleConfigure)
		b.PostConfigure = expr.If(len(b.PostConfigure_Darwin) > 0, b.PostConfigure_Darwin, b.PostConfigure)
		b.PreBuild = expr.If(len(b.PreBuild_Darwin) > 0, b.PreBuild_Darwin, b.PreBuild)
		b.FixBuild = expr.If(len(b.FixBuild_Darwin) > 0, b.FixBuild_Darwin, b.FixBuild)
		b.FreeStyleBuild = expr.If(len(b.FreeStyleBuild_Darwin) > 0, b.FreeStyleBuild_Darwin, b.FreeStyleBuild)
		b.PostBuild = expr.If(len(b.PostBuild_Darwin) > 0, b.PostBuild_Darwin, b.PostBuild)
		b.PreInstall = expr.If(len(b.PreInstall_Darwin) > 0, b.PreInstall_Darwin, b.PreInstall)
		b.FreeStyleInstall = expr.If(len(b.FreeStyleInstall_Darwin) > 0, b.FreeStyleInstall_Darwin, b.FreeStyleInstall)
		b.PostInstall = expr.If(len(b.PostInstall_Darwin) > 0, b.PostInstall_Darwin, b.PostInstall)
		b.AutogenOptions = expr.If(len(b.AutogenOptions_Darwin) > 0, b.AutogenOptions_Darwin, b.AutogenOptions)
		b.Options = expr.If(len(b.Options_Darwin) > 0, b.Options_Darwin, b.Options)
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
