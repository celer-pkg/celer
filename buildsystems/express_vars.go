package buildsystems

import (
	"celer/pkgs/dirs"
	"fmt"
	"maps"
	"os"
	"path/filepath"
	"regexp"
	"strings"
)

type ExpressVars struct {
	vars map[string]string
}

// Init initialize Variables with values from the context.
func (v *ExpressVars) Init(parent map[string]string, config BuildConfig) *ExpressVars {
	v.vars = make(map[string]string)

	// Merge from parent.
	maps.Copy(v.vars, parent)

	v.vars["REPO_DIR"] = config.PortConfig.RepoDir
	v.vars["SRC_DIR"] = config.PortConfig.SrcDir
	v.vars["BUILD_DIR"] = config.PortConfig.BuildDir
	v.vars["PACKAGE_DIR"] = config.PortConfig.PackageDir
	v.vars["DEPS_DEV_DIR"] = filepath.Join(dirs.TmpDepsDir, config.PortConfig.HostName+"-dev")

	if config.DevDep {
		v.vars["DEPS_DIR"] = filepath.Join(dirs.TmpDepsDir, config.PortConfig.HostName+"-dev")
	} else {
		v.vars["DEPS_DIR"] = filepath.Join(dirs.TmpDepsDir, config.PortConfig.LibraryFolder)
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
