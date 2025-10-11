package context

import (
	"crypto/tls"
	"fmt"
	"net/http"
	"net/url"
)

type Context interface {
	Proxy() *Proxy
	Version() string
	Platform() Platform
	Project() Project
	BuildType() string
	Toolchain() Toolchain
	WindowsKit() WindowsKit
	RootFS() RootFS
	Jobs() int
	Offline() bool
	CacheDir() CacheDir
	Verbose() bool
	Optimize(buildsystem, toolchain string) *Optimize
	GenerateToolchainFile() error
}

type CacheDir interface {
	Validate() error
	GetDir() string
	Read(platformName, projectName, buildType, nameVersion, hash, destDir string) (bool, error)
	Write(packageDir, meta string) error
	Remove(platformName, projectName, buildType, nameVersion string) error
	Exist(platformName, projectName, buildType, nameVersion, hash string) bool
}

type Proxy struct {
	Host string `toml:"host"`
	Port int    `toml:"port"`
}

func (p Proxy) HttpClient() *http.Client {
	return &http.Client{
		Transport: &http.Transport{
			Proxy: http.ProxyURL(&url.URL{
				Scheme: "http",
				Host:   fmt.Sprintf("%s:%d", p.Host, p.Port),
			}),
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: false,
			},
		},
	}
}
