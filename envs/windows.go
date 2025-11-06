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
	home := os.Getenv("HOME")
	homeDriver := os.Getenv("HOMEDRIVE")
	homePath := os.Getenv("HOMEPATH")
	username := os.Getenv("USERNAME")
	userProfile := os.Getenv("USERPROFILE")
	systemRoot := os.Getenv("SystemRoot")
	comSpec := os.Getenv("ComSpec")
	systemDrive := os.Getenv("SystemDrive")
	driverData := os.Getenv("DriverData")
	appData := os.Getenv("APPDATA")
	localAppData := os.Getenv("LOCALAPPDATA")
	processorArchitecture := os.Getenv("PROCESSOR_ARCHITECTURE")
	processorIdentifier := os.Getenv("PROCESSOR_IDENTIFIER")
	processorLevel := os.Getenv("PROCESSOR_LEVEL")
	processorRevision := os.Getenv("PROCESSOR_REVISION")
	numberOfProcessors := os.Getenv("NUMBER_OF_PROCESSORS")
	allUsersProfile := os.Getenv("ALLUSERSPROFILE")
	programData := os.Getenv("ProgramData")
	programFiles := os.Getenv("ProgramFiles")
	programFilesX86 := os.Getenv("ProgramFiles(x86)")
	commonProgramFiles := os.Getenv("CommonProgramFiles")
	commonProgramFilesX86 := os.Getenv("CommonProgramFiles(x86)")
	commonProgramW6432 := os.Getenv("CommonProgramW6432")
	programW6432 := os.Getenv("ProgramW6432")
	psModulePath := os.Getenv("PSModulePath")
	portsRepo := os.Getenv("CELER_PORTS_REPO")

	os.Clearenv()

	// Restore necessary environemnts.
	os.Setenv("TEMP", temp)
	os.Setenv("TMP", tmp)
	os.Setenv("OS", operatingSystem)
	os.Setenv("HOME", homeDriver)
	os.Setenv("HOMEDRIVE", home)
	os.Setenv("HOMEPATH", homePath)
	os.Setenv("USERNAME", username)
	os.Setenv("USERPROFILE", userProfile)
	os.Setenv("SystemRoot", systemRoot)
	os.Setenv("ComSpec", comSpec)
	os.Setenv("SystemDrive", systemDrive)
	os.Setenv("DriverData", driverData)
	os.Setenv("APPDATA", appData)
	os.Setenv("LOCALAPPDATA", localAppData)
	os.Setenv("PROCESSOR_ARCHITECTURE", processorArchitecture)
	os.Setenv("PROCESSOR_IDENTIFIER", processorIdentifier)
	os.Setenv("PROCESSOR_LEVEL", processorLevel)
	os.Setenv("PROCESSOR_REVISION", processorRevision)
	os.Setenv("NUMBER_OF_PROCESSORS", numberOfProcessors)
	os.Setenv("ALLUSERSPROFILE", allUsersProfile)
	os.Setenv("ProgramData", programData)
	os.Setenv("ProgramFiles", programFiles)
	os.Setenv("ProgramFiles(x86)", programFilesX86)
	os.Setenv("ProgramW6432", programW6432)
	os.Setenv("CommonProgramFiles", commonProgramFiles)
	os.Setenv("CommonProgramFiles(x86)", commonProgramFilesX86)
	os.Setenv("CommonProgramW6432", commonProgramW6432)
	os.Setenv("PSModulePath", psModulePath)
	os.Setenv("CELER_PORTS_REPO", portsRepo)

	// Reset PATH.
	var paths []string
	paths = append(paths, `C:\Windows`)
	paths = append(paths, `C:\Windows\System32`)
	paths = append(paths, `C:\Windows\SysWOW64`)
	paths = append(paths, `C:\Windows\System32\Wbem`)
	paths = append(paths, `C:\Windows\System32\downlevel`)
	paths = append(paths, `C:\Windows\SysWOW64\WindowsPowerShell\v1.0`)
	paths = append(paths, `C:\Windows\System32\WindowsPowerShell\v1.0`)

	// Add python launcher path so that we can call py from cmd.
	homeDir, err := os.UserHomeDir()
	if err != nil {
		panic("cannot get home directory in your system.")
	}
	paths = append(paths, filepath.Join(homeDir, `AppData\Local\Programs\Python\Launcher`))

	// Use PATH instead of Path.
	os.Unsetenv("Path")
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
}
