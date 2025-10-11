package context

type CacheDir interface {
	Validate() error
	GetDir() string
	Read(platformName, projectName, buildType, nameVersion, hash, destDir string) (bool, error)
	Write(packageDir, meta string) error
	Remove(platformName, projectName, buildType, nameVersion string) error
	Exist(platformName, projectName, buildType, nameVersion, hash string) bool
}
