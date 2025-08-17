package proxy

import (
	"fmt"
	"net/url"
	"strings"
)

var (
	githubAssetProxy string
	githubRepoProxy  string
)

// CacheGithubProxies caches the github asset and repo proxy.
func CacheGithubProxies(assetProxy, repoProxy string) {
	githubAssetProxy = assetProxy
	githubRepoProxy = repoProxy
}

// HackAssetkUrl hacks the github asset url to use the proxy.
func HackAssetkUrl(assetUrl string) string {
	if strings.TrimSpace(githubAssetProxy) == "" {
		return assetUrl
	}

	if strings.HasPrefix(assetUrl, "https://github.com") || strings.HasPrefix(assetUrl, "github.com") {
		return strings.ReplaceAll(assetUrl, "https://github.com", githubAssetProxy+"/github.com")
	}

	return assetUrl
}

// HackRepoUrl hacks the github repo url to use the proxy.
func HackRepoUrl(repoUrl string) (string, error) {
	if strings.TrimSpace(githubRepoProxy) == "" {
		return repoUrl, nil
	}

	switch githubRepoProxy {
	case "https://gitclone.com":
		if strings.HasPrefix(repoUrl, "https://github.com") {
			redirectedUrl, err := url.JoinPath(githubRepoProxy, strings.TrimPrefix(repoUrl, "https://"))
			if err != nil {
				return "", fmt.Errorf("github proxy repo url error: %w", err)
			}
			return redirectedUrl, nil
		}

		if strings.HasPrefix(repoUrl, "github.com") {
			redirectedUrl, err := url.JoinPath(githubRepoProxy, repoUrl)
			if err != nil {
				return "", fmt.Errorf("github proxy repo url error: %w", err)
			}
			return redirectedUrl, nil
		}

	case "https://githubfast.com":
		if strings.HasPrefix(repoUrl, "https://github.com") {
			return strings.ReplaceAll(repoUrl, "https://github.com", githubRepoProxy), nil
		}

		if strings.HasPrefix(repoUrl, "github.com") {
			return strings.ReplaceAll(repoUrl, "github.com", githubRepoProxy), nil
		}
	}

	return repoUrl, nil
}
