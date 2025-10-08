package proxy

import (
	"crypto/tls"
	"fmt"
	"net/http"
	"net/url"
)

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
