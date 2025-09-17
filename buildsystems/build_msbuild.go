package buildsystems

import "celer/buildtools"

func NewMSBuild(config *BuildConfig, optLevel *OptLevel) *msbuild {
	return &msbuild{
		BuildConfig: config,
		OptLevel:    optLevel,
	}
}

type msbuild struct {
	*BuildConfig
	*OptLevel
}

func (m msbuild) Name() string {
	return "msbuild"
}

func (m msbuild) CheckTools() error {
	m.BuildConfig.BuildTools = append(m.BuildConfig.BuildTools, "git", "cmake")
	return buildtools.CheckTools(m.BuildConfig.BuildTools...)
}

func (m msbuild) Clean() error {
	return nil
}

func (m msbuild) Configure(options []string) error {
	// msbuild source\allinone\allinone.sln
	// /p:Configuration=Release
	// /p:Platform=x64
	// /p:SkipUWP=true
	// /p:InstallDir="D:\icu-install\"
	return nil
}

func (m msbuild) Build(options []string) error {
	return nil
}

func (m msbuild) Install(options []string) error {
	return nil
}
