package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"path/filepath"
	"runtime"
	"strings"
)

type Variables struct {
	pairs map[string]string
}

// Initialize initializes Variables with values from the context.
func (v *Variables) Initialize(ctx context.Context) *Variables {
	v.pairs = make(map[string]string)

	toolchain := ctx.Platform().GetToolchain()
	sysroot := ctx.Platform().GetRootFS()

	v.pairs["HOST"] = toolchain.GetHost()
	v.pairs["SYSTEM_NAME"] = strings.ToLower(toolchain.GetSystemName())
	v.pairs["SYSTEM_PROCESSOR"] = toolchain.GetSystemProcessor()
	v.pairs["CROSSTOOL_PREFIX"] = toolchain.GetCrosstoolPrefix()
	if sysroot != nil {
		v.pairs["SYSROOT"] = sysroot.GetAbsPath()
	}

	v.pairs["BUILDTREES_DIR"] = dirs.BuildtreesDir
	v.pairs["INSTALLED_DIR"] = fileio.ToRelPath(ctx.InstalledDir())
	v.pairs["INSTALLED_DEV_DIR"] = fileio.ToRelPath(ctx.InstalledDevDir())

	if buildtools.Python3 != nil {
		v.pairs["PYTHON3_PATH"] = fileio.ToRelPath(buildtools.Python3.Path)
	}

	if buildtools.LLVMPath != "" {
		llvmConfig := expr.If(runtime.GOOS == "windows", "llvm-config.exe", "llvm-config")
		llvmRoot := fileio.ToRelPath(buildtools.LLVMPath)
		llvmConfigPath := filepath.Join(llvmRoot, "bin", llvmConfig)
		v.pairs["LLVM_CONFIG"] = filepath.ToSlash(llvmConfigPath)
	}

	return v
}

// Expand replace express with values.
func (v Variables) Expand(content string) string {
	for key, value := range v.pairs {
		content = strings.ReplaceAll(content, fmt.Sprintf("${%s}", key), value)
		content = strings.ReplaceAll(content, fmt.Sprintf("$%s", key), value)
	}

	return content
}
