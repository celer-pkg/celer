package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
)

type ExpressVars struct {
	vars map[string]string
}

// Init initialize Variables with values from the context.
func (v *ExpressVars) Init(ctx context.Context) *ExpressVars {
	v.vars = make(map[string]string)

	toolchain := ctx.Platform().GetToolchain()
	sysroot := ctx.Platform().GetRootFS()

	v.vars["HOST"] = toolchain.GetHost()
	v.vars["SYSTEM_NAME"] = strings.ToLower(toolchain.GetSystemName())
	v.vars["SYSTEM_PROCESSOR"] = toolchain.GetSystemProcessor()
	v.vars["CROSSTOOL_PREFIX"] = toolchain.GetCrosstoolPrefix()
	if sysroot != nil {
		v.vars["SYSROOT"] = sysroot.GetAbsPath()
	}

	v.vars["BUILDTREES_DIR"] = dirs.BuildtreesDir
	v.vars["INSTALLED_DIR"] = fileio.ToRelPath(ctx.InstalledDir())
	v.vars["INSTALLED_DEV_DIR"] = fileio.ToRelPath(ctx.InstalledDevDir())

	if buildtools.Python3 != nil {
		v.vars["PYTHON3_PATH"] = fileio.ToRelPath(buildtools.Python3.Path)
	}

	if buildtools.LLVMPath != "" {
		llvmConfig := expr.If(runtime.GOOS == "windows", "llvm-config.exe", "llvm-config")
		llvmRoot := fileio.ToRelPath(buildtools.LLVMPath)
		llvmConfigPath := filepath.Join(llvmRoot, "bin", llvmConfig)
		v.vars["LLVM_CONFIG"] = filepath.ToSlash(llvmConfigPath)
	}

	return v
}

// Replace replace express with values.
func (v ExpressVars) Replace(content string) string {
	for key, value := range v.vars {
		content = strings.ReplaceAll(content, fmt.Sprintf("${%s}", key), value)
		content = strings.ReplaceAll(content, fmt.Sprintf("$%s", key), value)
		content = v.replaceEnvVars(content)
	}

	return content
}

// replaceEnvVars replace env express with env value.
func (v ExpressVars) replaceEnvVars(content string) string {
	reg := regexp.MustCompile(`\$ENV\{([^}]+)\}`)
	return reg.ReplaceAllStringFunc(content, func(match string) string {
		varName := reg.FindStringSubmatch(match)[1]
		if value, ok := os.LookupEnv(varName); ok {
			return value
		}
		return match
	})
}
