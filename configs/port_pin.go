package configs

// pinnedSources holds the applied source pins, keyed by name@version.
var pinnedSources map[string]string

// PinSources pins every source revision to the value recorded in an exported
// build DAG, so that all agents of a distributed build use the same revision.
func PinSources(pins map[string]string) {
	pinnedSources = pins
	ResetMetaCache()
}

// PinSource same as PinSources but pin the source revision of a single port.
func PinSource(nameVersion, checksum string) {
	if pinnedSources == nil {
		pinnedSources = map[string]string{}
	}
	pinnedSources[nameVersion] = checksum
}

// pinnedSourceOf returns the pin of a port, if one was applied.
func pinnedSourceOf(nameVersion string) (string, bool) {
	pin, ok := pinnedSources[nameVersion]
	return pin, ok && pin != ""
}
