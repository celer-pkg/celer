package meta

import (
	"strings"

	"github.com/BurntSushi/toml"
)

const (
	sectionPrefix = "# -------- "
	sectionSuffix = " --------"
)

// PortCommit returns the source ref recorded for nameVersion in a build meta
// (a git commit, or the archive sha256 for non-git sources). The port TOML keeps
// it under [package].ref and does not marshal name/version, so the section
// divider is the only way to locate the right port.
func PortCommit(metaText, nameVersion string) (string, bool) {
	var (
		sectionName string
		sectionType string
		inSection   bool
		body        []string
	)

	for line := range strings.SplitSeq(metaText, "\n") {
		if strings.HasPrefix(line, sectionPrefix) && strings.HasSuffix(line, sectionSuffix) {
			if ref, ok := sectionRef(inSection, nameVersion, sectionName, sectionType, body); ok {
				return ref, true
			}

			title := strings.TrimSuffix(strings.TrimPrefix(line, sectionPrefix), sectionSuffix)
			parts := strings.Split(title, " <<< ")
			inSection = true
			body = nil
			if len(parts) >= 2 {
				sectionName = parts[len(parts)-2]
				sectionType = parts[len(parts)-1]
			} else {
				sectionName, sectionType = "", ""
			}
			continue
		}

		if inSection {
			body = append(body, line)
		}
	}

	if ref, ok := sectionRef(inSection, nameVersion, sectionName, sectionType, body); ok {
		return ref, true
	}
	return "", false
}

// sectionRef returns [package].ref from a section's TOML body when that section
// is the port section of nameVersion; ok is false otherwise.
func sectionRef(inSection bool, nameVersion, sectionName, sectionType string, body []string) (string, bool) {
	if !inSection || sectionType != "port" || stripLabel(sectionName) != nameVersion {
		return "", false
	}

	var port struct {
		Package struct {
			Ref string `toml:"ref"`
		} `toml:"package"`
	}
	if _, err := toml.Decode(strings.Join(body, "\n"), &port); err != nil {
		return "", false
	}
	return port.Package.Ref, port.Package.Ref != ""
}

// stripLabel removes the section-kind label a nested port carries in its
// divider, e.g. "dependency: libexpat@2.7.1" -> "libexpat@2.7.1".
func stripLabel(token string) string {
	for _, prefix := range []string{"dependency: ", "dev_dependency: "} {
		if after, ok := strings.CutPrefix(token, prefix); ok {
			return after
		}
	}
	return token
}
