package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"fmt"
	"path/filepath"
	"runtime"
	"strings"
)

type Variables struct {
	pairs map[string]string
}

func (v *Variables) Inflat(ctx context.Context) *Variables {
	v.pairs = make(map[string]string)

	toolchain := ctx.Platform().GetToolchain()
	sysroot := ctx.Platform().GetRootFS()

	v.pairs["HOST"] = toolchain.GetHost()
	v.pairs["SYSTEM_NAME"] = strings.ToLower(toolchain.GetSystemName())
	v.pairs["SYSTEM_PROCESSOR"] = toolchain.GetSystemProcessor()
	v.pairs["CROSSTOOL_PREFIX"] = toolchain.GetCrosstoolPrefix()
	if sysroot != nil {
		v.pairs["SYSROOT"] = sysroot.GetFullPath()
	}

	v.pairs["BUILDTREES_DIR"] = dirs.BuildtreesDir
	v.pairs["INSTALLED_DIR"] = ctx.InstalledDir(true)
	v.pairs["INSTALLED_DEV_DIR"] = ctx.InstalledDevDir(true)

	if buildtools.Python3 != nil {
		v.pairs["PYTHON3_PATH"] = buildtools.Python3.Path
	}

	if buildtools.LLVMPath != "" {
		llvmConfig := expr.If(runtime.GOOS == "windows", "llvm-config.exe", "llvm-config")
		v.pairs["LLVM_CONFIG"] = filepath.Join(buildtools.LLVMPath, "bin", llvmConfig)
	}

	return v
}

func (v Variables) Expand(content string) string {
	for key, value := range v.pairs {
		content = strings.ReplaceAll(content, fmt.Sprintf("${%s}", key), value)
		content = strings.ReplaceAll(content, fmt.Sprintf("$%s", key), value)
	}

	return content
}
