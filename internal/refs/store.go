package refs

// resolvedCommits maps NameVersion -> resolved full commit hash.
var resolvedCommits map[string]string

// StoreResolvedCommits stores the resolved commit hashes for later lookup
// during clone/checkout, so that the exact resolved commit is used instead
// of re-resolving the branch/tag.
func StoreResolvedCommits(commits map[string]string) {
	resolvedCommits = commits
}

// GetResolvedCommit returns the previously resolved commit hash for the
// given nameVersion, or an empty string if none was stored.
func GetResolvedCommit(nameVersion string) string {
	if resolvedCommits == nil {
		return ""
	}
	return resolvedCommits[nameVersion]
}
