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
	os.Setenv("TEMP", temp)
	os.Setenv("TMP", tmp)
	os.Setenv("OS", operatingSystem)
	os.Setenv("HOMEDRIVE", homeDriver)
	os.Setenv("HOMEPATH", homePath)
	os.Setenv("USERNAME", username)
	os.Setenv("USERPROFILE", userProfile)
	os.Setenv("SystemRoot", systemRoot)
	os.Setenv("SystemDrive", systemDrive)
	os.Setenv("LOCALAPPDATA", localAppData)
	os.Setenv("PROCESSOR_ARCHITECTURE", processorArchitecture)
	os.Setenv("PROCESSOR_IDENTIFIER", processorIdentifier)
	os.Setenv("PROCESSOR_LEVEL", processorLevel)
	os.Setenv("PROCESSOR_REVISION", processorRevision)
	os.Setenv("NUMBER_OF_PROCESSORS", numberOfProcessors)
	os.Setenv("CELER_PORTS_REPO", portsRepo)
	os.Setenv("GITHUB_ACTIONS", githubActions)

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
