package git

import (
	"celer/pkgs/cmd"
	"celer/pkgs/proxy"
	"fmt"
)

type Git struct {
	proxy *proxy.Proxy
}

func (g Git) proxyArgs() []string {
	var args []string
	if g.proxy != nil {
		args = append(args, "-c", fmt.Sprintf("http.proxy=http://%s:%d", g.proxy.Host, g.proxy.Port))
		args = append(args, "-c", fmt.Sprintf("https.proxy=https://%s:%d", g.proxy.Host, g.proxy.Port))
	}
	return args
}

func NewGit(proxy *proxy.Proxy) Git {
	return Git{proxy: proxy}
}

func (g Git) Execute(title, workDir string, commands []string) error {
	for _, command := range commands {
		var args []string
		args = append(args, g.proxyArgs()...)
		args = append(args, command)

		executor := cmd.NewExecutor(title, "git", args...)
		executor.SetWorkDir(workDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}
