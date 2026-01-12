package buildsystems

import (
	"celer/context"
	"fmt"
)

// bazelMSVCGenerator implements bazelToolchainGenerator for MSVC compiler
// Note: MSVC/clang-cl on Windows use msvc_cc_toolchain_config (different mechanism)
// This implementation is a placeholder for future Windows cross-compilation support
type bazelMSVCGenerator struct{}

func (g *bazelMSVCGenerator) GetCompilerName() string {
	return "msvc"
}

func (g *bazelMSVCGenerator) GetToolPaths(toolchain context.Toolchain, toolchainBinDir string) (map[string]string, error) {
	// TODO: Implement MSVC tool paths for Windows cross-compilation
	// This would require msvc_cc_toolchain_config instead of unix_cc_toolchain_config
	return nil, fmt.Errorf("MSVC toolchain generation not yet implemented for Bazel cross-compilation")
}

func (g *bazelMSVCGenerator) GetIncludeDirectories(toolchain context.Toolchain, toolchainRoot, sysrootPath string, depsIncDir string) ([]string, error) {
	// TODO: Implement MSVC include directories for Windows cross-compilation
	return nil, fmt.Errorf("MSVC include directories not yet implemented for Bazel cross-compilation")
}
