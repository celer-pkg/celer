package buildsystems

import (
	"celer/pkgs/dirs"
	"fmt"
	"maps"
	"path/filepath"
	"strings"
)

type Variables struct {
	pairs map[string]string
}

func (v *Variables) Inflat(parent map[string]string, config BuildConfig) *Variables {
	v.pairs = make(map[string]string)

	// Merge from parent.
	maps.Copy(v.pairs, parent)

	v.pairs["REPO_DIR"] = config.PortConfig.RepoDir
	v.pairs["SRC_DIR"] = config.PortConfig.SrcDir
	v.pairs["BUILD_DIR"] = config.PortConfig.BuildDir
	v.pairs["PACKAGE_DIR"] = config.PortConfig.PackageDir
	v.pairs["DEPS_DEV_DIR"] = filepath.Join(dirs.TmpDepsDir, config.PortConfig.HostName+"-dev")

	if config.DevDep {
		v.pairs["DEPS_DIR"] = filepath.Join(dirs.TmpDepsDir, config.PortConfig.HostName+"-dev")
	} else {
		v.pairs["DEPS_DIR"] = filepath.Join(dirs.TmpDepsDir, config.PortConfig.LibraryFolder)
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
