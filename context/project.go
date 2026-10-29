package context

type Project interface {
	Init(ctx Context, projectName string) error
	GetName() string
	GetPortsBaseline() string
	GetPorts() []string
	GetTargetPlatform() string
	GetVars() []string
	GetEnvs() []string
	GetPythonConfig() PythonConfig
	Write(platformPath string, override bool) error
}
