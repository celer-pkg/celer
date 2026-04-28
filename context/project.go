package context

type Project interface {
	Init(ctx Context, projectName string) error
	GetName() string
	GetPorts() []string
	GetTargetPlatform() string
	GetPythonVersion() string
	Write(platformPath string, override bool) error
}
