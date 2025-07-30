package configs

type SetupArgs interface {
	Repair() bool
	InstallPorts() bool
}

type setupArgs struct {
	repair       bool // Called to check and fix build environment.
	installPorts bool // Called to install a 3rd party ports.
}

func (s setupArgs) Repair() bool {
	return s.repair
}

func (s setupArgs) InstallPorts() bool {
	return s.installPorts
}

func (s *setupArgs) SetInstallPorts(installPort bool) *setupArgs {
	s.installPorts = installPort
	return s
}

func NewSetupArgs(repaire, installPorts bool) *setupArgs {
	return &setupArgs{
		repair:       repaire,
		installPorts: installPorts,
	}
}
