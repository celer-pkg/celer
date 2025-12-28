//go:build windows

package envs

import (
	"celer/pkgs/env"
	"os"
	"path/filepath"
)

// CleanEnv clear all environments that not required and reset PATH.
func CleanEnv() {
	// Cache necessary environments.
	temp := os.Getenv("TEMP")
	tmp := os.Getenv("TMP")
	operatingSystem := os.Getenv("OS")
	homeDriver := os.Getenv("HOMEDRIVE")
	homePath := os.Getenv("HOMEPATH")
	username := os.Getenv("USERNAME")
	userProfile := os.Getenv("USERPROFILE")
	systemRoot := os.Getenv("SystemRoot")
	systemDrive := os.Getenv("SystemDrive")
	localAppData := os.Getenv("LOCALAPPDATA")
	processorArchitecture := os.Getenv("PROCESSOR_ARCHITECTURE")
	processorIdentifier := os.Getenv("PROCESSOR_IDENTIFIER")
	processorLevel := os.Getenv("PROCESSOR_LEVEL")
	processorRevision := os.Getenv("PROCESSOR_REVISION")
	numberOfProcessors := os.Getenv("NUMBER_OF_PROCESSORS")
	portsRepo := os.Getenv("CELER_PORTS_REPO")
	githubActions := os.Getenv("GITHUB_ACTIONS")

	os.Clearenv()

	// Restore necessary environemnts.
	setEnvIfNotEmpty("TEMP", temp)
	setEnvIfNotEmpty("TMP", tmp)
	setEnvIfNotEmpty("OS", operatingSystem)
	setEnvIfNotEmpty("HOMEDRIVE", homeDriver)
	setEnvIfNotEmpty("HOMEPATH", homePath)
	setEnvIfNotEmpty("USERNAME", username)
	setEnvIfNotEmpty("USERPROFILE", userProfile)
	setEnvIfNotEmpty("SystemRoot", systemRoot)
	setEnvIfNotEmpty("SystemDrive", systemDrive)
	setEnvIfNotEmpty("LOCALAPPDATA", localAppData)
	setEnvIfNotEmpty("PROCESSOR_ARCHITECTURE", processorArchitecture)
	setEnvIfNotEmpty("PROCESSOR_IDENTIFIER", processorIdentifier)
	setEnvIfNotEmpty("PROCESSOR_LEVEL", processorLevel)
	setEnvIfNotEmpty("PROCESSOR_REVISION", processorRevision)
	setEnvIfNotEmpty("NUMBER_OF_PROCESSORS", numberOfProcessors)
	setEnvIfNotEmpty("CELER_PORTS_REPO", portsRepo)
	setEnvIfNotEmpty("GITHUB_ACTIONS", githubActions)

	// Reset PATH.
	var paths []string
	paths = append(paths, `C:\Windows`)
	paths = append(paths, `C:\Windows\System32`)
	paths = append(paths, `C:\Windows\SysWOW64`)
	paths = append(paths, `C:\Windows\System32\Wbem`)
	paths = append(paths, `C:\Windows\System32\downlevel`)
	paths = append(paths, `C:\Windows\SysWOW64\WindowsPowerShell\v1.0`)
	paths = append(paths, `C:\Windows\System32\WindowsPowerShell\v1.0`)
	paths = append(paths, `C:\ProgramData\chocolatey\bin`)

	// Add python launcher path so that we can call py from cmd.
	paths = append(paths, filepath.Join(os.Getenv("USERPROFILE"), `AppData\Local\Programs\Python\Launcher`))

	// Use PATH instead of Path.
	os.Unsetenv("Path")
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
}

func setEnvIfNotEmpty(key, value string) {
	if value != "" {
		os.Setenv(key, value)
	}
}
