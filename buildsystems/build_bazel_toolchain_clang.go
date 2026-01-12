package buildsystems

import (
	"celer/context"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// bazelClangGenerator implements bazelToolchainGenerator for Clang compiler.
type bazelClangGenerator struct{}

func (b *bazelClangGenerator) GetCompilerName() string {
	return "clang"
}

func (b *bazelClangGenerator) GetToolPaths(toolchain context.Toolchain, toolchainBinDir string) (map[string]string, error) {
	ccPath := filepath.Join(toolchainBinDir, toolchain.GetCC())
	ccBase := filepath.Base(ccPath)

	// For Clang, tools may use the same prefix or be standalone.
	prefix := strings.TrimSuffix(ccBase, "clang")
	if prefix == "" {
		prefix = strings.TrimSuffix(ccBase, "clang++")
	}

	toolPaths := make(map[string]string)
	putIfNotEmpty := func(toolKey, filename string) {
		if filename != "" {
			toolPaths[toolKey] = filepath.Join(toolchainBinDir, filename)
		}
	}

	putIfNotEmpty("gcc", toolchain.GetCC())
	putIfNotEmpty("g++", toolchain.GetCXX())
	putIfNotEmpty("ar", toolchain.GetAR())
	putIfNotEmpty("ld", toolchain.GetLD())
	putIfNotEmpty("cpp", toolchain.GetCPP())
	putIfNotEmpty("gcov", toolchain.GetGCOV())
	putIfNotEmpty("nm", toolchain.GetNM())
	putIfNotEmpty("objdump", toolchain.GetOBJDUMP())
	putIfNotEmpty("strip", toolchain.GetSTRIP())
	putIfNotEmpty("objcopy", toolchain.GetOBJCOPY())
	putIfNotEmpty("as", toolchain.GetAS())

	// Bazel requires llvm-cov and dwp in tool_paths
	// Try toolchain directory first (llvm-cov and llvm-dwp), fallback to /usr/bin
	llvmCovPath := filepath.Join(toolchainBinDir, "llvm-cov")
	if fileio.PathExists(llvmCovPath) {
		toolPaths["llvm-cov"] = llvmCovPath
	} else {
		toolPaths["llvm-cov"] = "/usr/bin/llvm-cov"
	}
	// Bazel expects "dwp" key, but LLVM toolchain may have "llvm-dwp"
	dwpPath := filepath.Join(toolchainBinDir, "llvm-dwp")
	if !fileio.PathExists(dwpPath) {
		dwpPath = filepath.Join(toolchainBinDir, "dwp")
	}
	if fileio.PathExists(dwpPath) {
		toolPaths["dwp"] = dwpPath
	} else {
		toolPaths["dwp"] = "/usr/bin/dwp"
	}

	return toolPaths, nil
}

func (b *bazelClangGenerator) GetIncludeDirectories(toolchain context.Toolchain, toolchainRoot, sysrootPath string, depsIncDir string) ([]string, error) {
	incDirs := []string{
		sysrootPath + "/include",
		sysrootPath + "/usr/include",
	}

	toolchainBinDir := toolchain.GetFullPath()
	ccPath := filepath.Join(toolchainBinDir, toolchain.GetCC())
	ccBase := filepath.Base(ccPath)

	// Try to detect target triple from compiler.
	targetTriple := strings.TrimSuffix(ccBase, "-clang")
	if targetTriple == "" {
		targetTriple = strings.TrimSuffix(ccBase, "-clang++")
	}

	// If Clang uses GCC toolchain (common), add GCC paths.
	gccPath := filepath.Join(toolchainBinDir, strings.Replace(ccBase, "clang", "gcc", 1))
	if fileio.PathExists(gccPath) {
		gccVersionOutput, _ := exec.Command(gccPath, "-dumpversion").Output()
		gccVersion := strings.TrimSpace(string(gccVersionOutput))
		if gccVersion != "" {
			incDirs = append(incDirs,
				fmt.Sprintf("%s/lib/gcc/%s/%s/include", toolchainRoot, targetTriple, gccVersion),
				fmt.Sprintf("%s/lib/gcc/%s/%s/include-fixed", toolchainRoot, targetTriple, gccVersion),
				fmt.Sprintf("%s/%s/include/c++/%s", toolchainRoot, targetTriple, gccVersion),
				fmt.Sprintf("%s/%s/include/c++/%s/%s", toolchainRoot, targetTriple, gccVersion, targetTriple),
				fmt.Sprintf("%s/%s/include", toolchainRoot, targetTriple),
			)
		}
	}

	// Add Clang's own builtin headers (if using libc++ or standalone).
	clangLibDir := filepath.Join(toolchainRoot, "lib", "clang")
	if fileio.PathExists(clangLibDir) {
		if entries, err := os.ReadDir(clangLibDir); err == nil {
			for _, entry := range entries {
				if entry.IsDir() {
					clangIncDir := filepath.Join(clangLibDir, entry.Name(), "include")
					if fileio.PathExists(clangIncDir) {
						incDirs = append(incDirs, clangIncDir)
						break // Use first version found.
					}
				}
			}
		}
	}

	// Add libc++ include directories if using lld linker (which typically uses libc++).
	if strings.Contains(toolchain.GetLD(), "lld") {
		// Try to detect platform triple for libc++ paths.
		// For LLVM distributions, libc++ headers are in include/c++/v1 and include/<triple>/c++/v1
		libcxxInclude := filepath.Join(toolchainRoot, "include", "c++", "v1")
		if fileio.PathExists(libcxxInclude) {
			incDirs = append(incDirs, libcxxInclude)
		}

		// Try to detect platform triple dynamically by checking compiler output.
		ccPath := filepath.Join(toolchainBinDir, toolchain.GetCC())
		if tripleOutput, err := exec.Command(ccPath, "-print-effective-triple").Output(); err == nil {
			triple := strings.TrimSpace(string(tripleOutput))
			if triple != "" {
				platformLibcxxInclude := filepath.Join(toolchainRoot, "include", triple, "c++", "v1")
				if fileio.PathExists(platformLibcxxInclude) {
					incDirs = append(incDirs, platformLibcxxInclude)
				}
			}
		}
	}

	// Add dependency libraries' include directories.
	if depsIncDir != "" && fileio.PathExists(depsIncDir) {
		incDirs = append(incDirs, depsIncDir)
	}

	return incDirs, nil
}
