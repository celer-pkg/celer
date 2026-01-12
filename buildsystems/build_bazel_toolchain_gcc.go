package buildsystems

import (
	"celer/context"
	"celer/pkgs/fileio"
	"fmt"
	"os/exec"
	"path/filepath"
	"strings"
)

// bazelGCCGenerator implements bazelToolchainGenerator for GCC compiler
type bazelGCCGenerator struct{}

func (b *bazelGCCGenerator) GetCompilerName() string {
	return "gcc"
}

func (b *bazelGCCGenerator) GetToolPaths(toolchain context.Toolchain, toolchainBinDir string) (map[string]string, error) {
	ccPath := filepath.Join(toolchainBinDir, toolchain.GetCC())
	ccBase := filepath.Base(ccPath)

	// For GCC, tools follow the pattern: {prefix}-{tool}
	prefix := strings.TrimSuffix(ccBase, "gcc")

	toolPaths := map[string]string{
		"gcc":      filepath.Join(toolchainBinDir, toolchain.GetCC()),
		"g++":      filepath.Join(toolchainBinDir, toolchain.GetCXX()),
		"ar":       filepath.Join(toolchainBinDir, toolchain.GetAR()),
		"ld":       filepath.Join(toolchainBinDir, toolchain.GetLD()),
		"cpp":      filepath.Join(toolchainBinDir, prefix+"cpp"),
		"gcov":     filepath.Join(toolchainBinDir, prefix+"gcov"),
		"nm":       filepath.Join(toolchainBinDir, prefix+"nm"),
		"objdump":  filepath.Join(toolchainBinDir, prefix+"objdump"),
		"strip":    filepath.Join(toolchainBinDir, prefix+"strip"),
		"objcopy":  filepath.Join(toolchainBinDir, prefix+"objcopy"),
		"as":       filepath.Join(toolchainBinDir, prefix+"as"),
		"llvm-cov": "/usr/bin/llvm-cov",
		"dwp":      "/usr/bin/dwp",
	}

	return toolPaths, nil
}

func (b *bazelGCCGenerator) GetIncludeDirectories(toolchain context.Toolchain, toolchainRoot, sysrootPath string, depsIncDir string) ([]string, error) {
	incDirs := []string{
		sysrootPath + "/include",
		sysrootPath + "/usr/include",
	}

	// Get GCC version
	toolchainBinDir := toolchain.GetFullPath()
	ccPath := filepath.Join(toolchainBinDir, toolchain.GetCC())
	ccBase := filepath.Base(ccPath)
	targetTriple := strings.TrimSuffix(ccBase, "-gcc")

	gccVersionOutput, _ := exec.Command(ccPath, "-dumpversion").Output()
	gccVersion := strings.TrimSpace(string(gccVersionOutput))
	if gccVersion == "" {
		return nil, fmt.Errorf("failed to get GCC version")
	}

	// Add GCC's builtin header paths
	incDirs = append(incDirs,
		fmt.Sprintf("%s/lib/gcc/%s/%s/include", toolchainRoot, targetTriple, gccVersion),
		fmt.Sprintf("%s/lib/gcc/%s/%s/include-fixed", toolchainRoot, targetTriple, gccVersion),
		fmt.Sprintf("%s/%s/include/c++/%s", toolchainRoot, targetTriple, gccVersion),
		fmt.Sprintf("%s/%s/include/c++/%s/%s", toolchainRoot, targetTriple, gccVersion, targetTriple),
		fmt.Sprintf("%s/%s/include", toolchainRoot, targetTriple),
	)

	// Add dependency libraries' include directories
	if depsIncDir != "" && fileio.PathExists(depsIncDir) {
		incDirs = append(incDirs, depsIncDir)
	}

	return incDirs, nil
}
