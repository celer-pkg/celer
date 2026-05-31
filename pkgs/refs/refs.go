package refs

import (
	"strings"

	"github.com/celer-pkg/celer/pkgs/git"
)

// SourceType indicates how a port's source is fetched.
type SourceType string

const (
	SourceGit     SourceType = "git"
	SourceArchive SourceType = "archive"
	SourceVirtual SourceType = "virtual"
)

// ResolvedRef holds the resolved reference information for a single port.
type ResolvedRef struct {
	NameVersion    string
	SourceType     SourceType
	Url            string // effective URL (after build_config override)
	OriginalRef    string // ref as declared in port.toml
	Checksum       string // checksum field (if set, overrides ref)
	ResolvedCommit string // full 40-char commit hash (git only)
	Error          string // non-empty if resolution failed
}

// PortInfo holds the minimal port information needed for ref resolution.
// This decouples the refs package from the configs package to avoid import cycles.
type PortInfo struct {
	NameVersion string
	Url         string
	Ref         string
	Checksum    string
}

// ResolvePorts resolves each port's reference to a full commit hash or URL.
func ResolvePorts(ports []PortInfo) []ResolvedRef {
	results := make([]ResolvedRef, 0, len(ports))
	for _, info := range ports {
		results = append(results, resolvePort(info))
	}
	return results
}

func resolvePort(info PortInfo) ResolvedRef {
	result := ResolvedRef{
		NameVersion: info.NameVersion,
		Url:         info.Url,
		OriginalRef: info.Ref,
		Checksum:    info.Checksum,
	}

	// Virtual port: url == "_"
	if info.Url == "_" {
		result.SourceType = SourceVirtual
		result.Url = "-"
		result.OriginalRef = "-"
		return result
	}

	// Git source: url ends in .git
	if strings.HasSuffix(info.Url, ".git") {
		result.SourceType = SourceGit
		commit, err := resolveGitRef(info)
		if err != nil {
			result.Error = err.Error()
		} else {
			result.ResolvedCommit = commit
		}
		return result
	}

	// Archive source: everything else.
	result.SourceType = SourceArchive
	return result
}

func resolveGitRef(info PortInfo) (string, error) {
	if info.Checksum != "" {
		return info.Checksum, nil
	}
	if git.IsFullCommitHash(info.Ref) {
		return info.Ref, nil
	}
	if info.Ref == "" {
		return git.GetRemoteHeadCommit(info.NameVersion, info.Url)
	}
	return git.GetRemoteRefCommit(info.NameVersion, info.Url, info.Ref)
}
