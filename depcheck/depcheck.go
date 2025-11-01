package depcheck

import (
	"celer/configs"
	"celer/context"
	"celer/pkgs/expr"
	"fmt"
	"log"
	"slices"
	"strings"
)

func NewDepCheck() *depcheck {
	return &depcheck{
		debugMode: false,
	}
}

type versionInfo struct {
	parent  string
	version string
	devDep  bool
	native  bool
}

type depcheck struct {
	debugMode    bool
	ctx          context.Context
	versionInfos map[string][]versionInfo
	visited      map[string]bool
	path         []string
}

func (d *depcheck) log(format string, v ...any) {
	if d.debugMode {
		log.Printf(format, v...)
	}
}

func (d *depcheck) CheckConflict(ctx context.Context, ports ...configs.Port) error {
	d.ctx = ctx
	d.versionInfos = make(map[string][]versionInfo)

	// Collect version info of ports.
	for _, port := range ports {
		// Update cached version infos or add new version info.
		newVersionInfo := versionInfo{
			version: port.Version,
			parent:  ctx.Project().GetName(),
			devDep:  port.DevDep,
			native:  port.DevDep || port.Native,
		}
		if infos, ok := d.versionInfos[port.Name]; ok {
			// Check if the version is already defined.
			// If so, we can skip it.
			contains := slices.ContainsFunc(infos, func(info versionInfo) bool {
				return info.version == port.Version
			})
			if !contains {
				d.versionInfos[port.Name] = append(d.versionInfos[port.Name], newVersionInfo)
			}
		} else {
			d.versionInfos[port.Name] = []versionInfo{newVersionInfo}
		}

		if err := d.collectInfos(port.NameVersion(), port.DevDep, port.Native); err != nil {
			return err
		}
	}

	// Print version conflicts info if exist.
	var summaries []string
	for portName, conflicted := range d.versionInfos {
		if len(conflicted) > 1 {
			var conflicts []string
			for _, info := range conflicted {
				nameVersion := fmt.Sprintf("%s@%s", portName, info.version)
				format := expr.If(info.devDep || info.native, "%s is defined as dev in %s", "%s is defined in %s")
				conflicts = append(conflicts, fmt.Sprintf(format, nameVersion, info.parent))
			}

			summaries = append(summaries, fmt.Sprintf("    - %s", strings.Join(conflicts, ", ")))
		}
	}
	if len(summaries) > 0 {
		return fmt.Errorf("conflicting versions of ports detected:\n%s", strings.Join(summaries, "\n"))
	}

	return nil
}

// CheckCircular check if have circular dependency in port.
func (d *depcheck) CheckCircular(ctx context.Context, port configs.Port) error {
	d.ctx = ctx
	d.versionInfos = make(map[string][]versionInfo)
	d.visited = make(map[string]bool)
	d.path = make([]string, 0)
	return d.checkCircular(port)
}

func (d *depcheck) checkCircular(port configs.Port) error {
	if port.DevDep || port.Native {
		portKey := port.NameVersion() + "@dev"

		// Check if the port is already in the path.
		if slices.Contains(d.path, portKey) {
			cyclePath := append(d.path, portKey)
			return fmt.Errorf("circular dev_dependency detected: %s", strings.Join(cyclePath, " -> "))
		}

		// Check if the port is already visited.
		if d.visited[portKey] {
			d.log("dev port %s is already visited", portKey)
			return nil
		}
		d.visited[portKey] = true

		// Add the current port to the path.
		d.path = append(d.path, portKey)
		defer func() {
			// Remove the current port from the path.
			d.path = d.path[:len(d.path)-1]
		}()
		d.log("visiting %s", portKey)
	} else {
		portKey := port.NameVersion()

		// Check if the port is already in the path.
		if slices.Contains(d.path, portKey) {
			cyclePath := append(d.path, portKey)
			return fmt.Errorf("circular dependency detected: %s", strings.Join(cyclePath, " -> "))
		}

		// Check if the port is already visited.
		if d.visited[portKey] {
			d.log("port %s is already visited", portKey)
			return nil
		}
		d.visited[portKey] = true

		// Add the current port to the path.
		d.path = append(d.path, portKey)
		defer func() {
			// Remove the current port from the path.
			d.path = d.path[:len(d.path)-1]
		}()
		d.log("visiting %s", portKey)
	}

	// Check circular dependency of dev_dependencies.
	for _, nameVersion := range port.MatchedConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (port.DevDep || port.Native) && port.NameVersion() == nameVersion {
			d.log("skip self %s", nameVersion)
			continue
		}

		// Init port.
		var devPort = configs.Port{
			DevDep: true,
			Native: true,
		}
		if err := devPort.Init(d.ctx, nameVersion); err != nil {
			return err
		}

		// Add new version info or update version info.
		newVersionInfo := versionInfo{
			version: devPort.Version,
			parent:  port.NameVersion(),
			devDep:  true,
			native:  true,
		}
		if infos, ok := d.versionInfos[devPort.Name]; ok {
			contains := slices.ContainsFunc(infos, func(info versionInfo) bool {
				return info.version == devPort.Version
			})
			if !contains {
				d.versionInfos[devPort.Name] = append(d.versionInfos[devPort.Name], newVersionInfo)
				d.log("add dev dependency %s", nameVersion)
			}
		} else {
			d.versionInfos[devPort.Name] = []versionInfo{newVersionInfo}
			d.log("add dev dependency %s", nameVersion)
		}

		// Check dependencies recursive.
		if err := d.checkCircular(devPort); err != nil {
			return err
		}
	}

	// Check circular dependency of dependencies.
	for _, nameVersion := range port.MatchedConfig.Dependencies {
		var devPort = configs.Port{
			DevDep: false,
			Native: port.DevDep || port.Native,
		}
		if err := devPort.Init(d.ctx, nameVersion); err != nil {
			return err
		}

		newVersionInfo := versionInfo{
			version: devPort.Version,
			parent:  devPort.Name,
			devDep:  false,
			native:  port.DevDep,
		}
		if infos, ok := d.versionInfos[devPort.Name]; ok {
			contains := slices.ContainsFunc(infos, func(info versionInfo) bool {
				return info.version == devPort.Version
			})
			if !contains {
				d.versionInfos[devPort.Name] = append(d.versionInfos[devPort.Name], newVersionInfo)
				d.log("add dependency %s", nameVersion)
			}
		} else {
			d.versionInfos[devPort.Name] = []versionInfo{newVersionInfo}
			d.log("add dependency %s", nameVersion)
		}

		// Recursively check dependencies.
		if err := d.checkCircular(devPort); err != nil {
			return err
		}
	}

	return nil
}

func (d *depcheck) collectInfos(nameVersion string, devDep, native bool) error {
	var port = configs.Port{
		DevDep: devDep,
		Native: native || devDep,
	}
	if err := port.Init(d.ctx, nameVersion); err != nil {
		return err
	}

	matchedConfig := port.MatchedConfig

	// Collect dev_dependency ports.
	for _, devDepNameVersion := range matchedConfig.DevDependencies {
		// Same name, version as parent and they are booth build with native toolchain, so skip.
		if (port.DevDep || port.Native) && port.DevDep && port.Native && port.NameVersion() == devDepNameVersion {
			d.log("skip self %s", devDepNameVersion)
			continue
		}

		var devDepPort = configs.Port{
			DevDep: true,
			Native: port.DevDep,
		}
		if err := devDepPort.Init(d.ctx, devDepNameVersion); err != nil {
			return err
		}

		newVersionInfo := versionInfo{
			version: devDepPort.Version,
			parent:  port.NameVersion(),
			devDep:  true,
			native:  port.DevDep,
		}
		if infos, ok := d.versionInfos[devDepPort.Name]; ok {
			contains := slices.ContainsFunc(infos, func(info versionInfo) bool {
				return info.version == devDepPort.Version
			})
			if !contains {
				d.versionInfos[devDepPort.Name] = append(d.versionInfos[devDepPort.Name], newVersionInfo)
				if err := d.collectInfos(devDepNameVersion, true, devDep); err != nil {
					return err
				}
			}
		} else {
			d.versionInfos[devDepPort.Name] = []versionInfo{newVersionInfo}
			if err := d.collectInfos(devDepNameVersion, true, devDep); err != nil {
				return err
			}
		}
	}

	// Collect dependency ports.
	for _, depNameVersion := range matchedConfig.Dependencies {
		// Init port to check if can locate the port.
		var depPort = configs.Port{
			DevDep: false,
			Native: port.DevDep,
		}
		if err := depPort.Init(d.ctx, depNameVersion); err != nil {
			return err
		}

		newVersionInfo := versionInfo{
			version: depPort.Version,
			parent:  port.NameVersion(),
			devDep:  false,
			native:  port.DevDep,
		}

		if infos, ok := d.versionInfos[depPort.Name]; ok {
			// Check if the version is already defined.
			// If so, we can skip it.
			contains := slices.ContainsFunc(infos, func(info versionInfo) bool {
				return info.version == depPort.Version
			})
			if !contains {
				d.versionInfos[depPort.Name] = append(d.versionInfos[depPort.Name], newVersionInfo)
			}
			if err := d.collectInfos(depNameVersion, false, devDep); err != nil {
				return err
			}
		} else {
			d.versionInfos[depPort.Name] = []versionInfo{newVersionInfo}
			if err := d.collectInfos(depNameVersion, false, devDep); err != nil {
				return err
			}
		}
	}

	return nil
}
