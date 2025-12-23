package generator

// type modules struct {
// 	cmakeConfig cmakeConfig
// }

// func (m *modules) generate(installedDir string) error {
// 	if len(m.cmakeConfig.Components) == 0 {
// 		return fmt.Errorf("components is empty")
// 	}

// 	if m.cmakeConfig.Libname == "" {
// 		return fmt.Errorf("lib name is empty")
// 	}

// 	if m.cmakeConfig.Version == "" {
// 		return fmt.Errorf("version is empty")
// 	}

// 	if m.cmakeConfig.Libtype == "" {
// 		return fmt.Errorf("lib type is empty")
// 	}
// 	m.cmakeConfig.Libtype = strings.ToLower(m.cmakeConfig.Libtype)

// 	if m.cmakeConfig.Namespace == "" {
// 		m.cmakeConfig.Namespace = m.cmakeConfig.Libname
// 	}

// 	modules, err := templates.ReadFile("templates/Modules.cmake.in")
// 	if err != nil {
// 		return err
// 	}

// 	addLibraryDepedencies, err := templates.ReadFile("templates/Modules-addLibrary-depedencies.cmake.in")
// 	if err != nil {
// 		return err
// 	}

// 	addLibraryIndependent, err := templates.ReadFile("templates/Modules-addLibrary-independent.cmake.in")
// 	if err != nil {
// 		return err
// 	}

// 	var libNames []string
// 	var addLibrarySections strings.Builder

// 	for index, component := range m.cmakeConfig.Components {
// 		if strings.ToLower(m.cmakeConfig.SystemName) == "linux" {
// 			if m.cmakeConfig.Libtype == "shared" {
// 				// Check soname and filename are not empty.
// 				var emptyFields []string
// 				if component.Soname == "" {
// 					emptyFields = append(emptyFields, "soname")
// 				}
// 				if component.Filename == "" {
// 					emptyFields = append(emptyFields, "filename")
// 				}
// 				if len(emptyFields) > 0 {
// 					return fmt.Errorf("%s of %s is empty to generate cmake config for linux shared library",
// 						strings.Join(emptyFields, " and "), component.Component)
// 				}

// 				// Check soname and filename are exist.
// 				var notExistFields []string
// 				if !fileio.PathExists(filepath.Join(installedDir, "lib", component.Filename)) {
// 					notExistFields = append(notExistFields, "filename")
// 				}
// 				if !fileio.PathExists(filepath.Join(installedDir, "lib", component.Soname)) {
// 					notExistFields = append(notExistFields, "soname")
// 				}
// 				if len(notExistFields) > 0 {
// 					return fmt.Errorf("%s of %s is not exist to generate cmake config for linux shared library",
// 						strings.Join(notExistFields, " and "), component.Component)
// 				}
// 			}
// 			if m.cmakeConfig.Libtype == "static" && component.Filename == "" {
// 				return fmt.Errorf("filename of %s is empty to generate cmake config for linux static library", component.Component)
// 			}
// 		}

// 		if strings.ToLower(m.cmakeConfig.SystemName) == "windows" {
// 			if m.cmakeConfig.Libtype == "shared" {
// 				// Check impname and filename are not empty.
// 				var emptyFields []string
// 				if component.Impname == "" {
// 					emptyFields = append(emptyFields, "impname")
// 				}
// 				if component.Filename == "" {
// 					emptyFields = append(emptyFields, "filename")
// 				}
// 				if len(emptyFields) > 0 {
// 					return fmt.Errorf("%s of %s is empty to generate cmake config for windows shared library",
// 						strings.Join(emptyFields, " and "), component.Component)
// 				}

// 				// Check impname and filename are exist.
// 				var notExistFields []string
// 				if !fileio.PathExists(filepath.Join(installedDir, "bin", component.Filename)) {
// 					notExistFields = append(notExistFields, "filename "+component.Filename)
// 				}
// 				if !fileio.PathExists(filepath.Join(installedDir, "lib", component.Impname)) {
// 					notExistFields = append(notExistFields, "impname "+component.Impname)
// 				}
// 				if len(notExistFields) > 0 {
// 					return fmt.Errorf("%s of %s is not exist to generate cmake config for windows shared library",
// 						strings.Join(notExistFields, " and "), component.Component)
// 				}
// 			}
// 			if m.cmakeConfig.Libtype == "static" && component.Filename == "" {
// 				return fmt.Errorf("filename of %s is empty to generate cmake config for windows static library", component.Component)
// 			}
// 		}

// 		libNames = append(libNames, m.cmakeConfig.Libname+"::"+component.Component)

// 		var section string
// 		if len(component.Dependencies) > 0 {
// 			section = string(addLibraryDepedencies)
// 		} else {
// 			section = string(addLibraryIndependent)
// 		}

// 		var dependencies []string
// 		for _, dependency := range component.Dependencies {
// 			dependencies = append(dependencies, m.cmakeConfig.Namespace+"::"+dependency)
// 		}

// 		section = strings.ReplaceAll(section, "@NAMESPACE@", m.cmakeConfig.Namespace)
// 		// section = strings.ReplaceAll(section, "@LIBNAME@", m.cmakeConfig.Libname)
// 		section = strings.ReplaceAll(section, "@COMPONENT@", component.Component)
// 		section = strings.ReplaceAll(section, "@LIBTYPE_UPPER@", strings.ToUpper(m.cmakeConfig.Libtype))
// 		section = strings.ReplaceAll(section, "@DEPEDENCIES@", strings.Join(dependencies, ";"))

// 		if index == 0 {
// 			addLibrarySections.WriteString(section + "\n")
// 		} else if index == len(m.cmakeConfig.Components)-1 {
// 			addLibrarySections.WriteString("\n" + section)
// 		} else {
// 			addLibrarySections.WriteString("\n" + section + "\n")
// 		}
// 	}

// 	content := string(modules)
// 	content = strings.ReplaceAll(content, "@NAMESPACE@", m.cmakeConfig.Namespace)
// 	content = strings.ReplaceAll(content, "@LIB_NAMES@", strings.Join(libNames, " "))
// 	content = strings.ReplaceAll(content, "@LIBNAME@", m.cmakeConfig.Libname)
// 	content = strings.ReplaceAll(content, "@LIBNAME_UPPER@", strings.ToUpper(m.cmakeConfig.Libname))
// 	content = strings.ReplaceAll(content, "@ADD_LIBRARY_SECTIONS@", addLibrarySections.String())

// 	// Make dirs for writing file.
// 	filePath := filepath.Join(installedDir, "lib", "cmake", m.cmakeConfig.Namespace, m.cmakeConfig.Namespace+"Modules.cmake")
// 	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
// 		return err
// 	}

// 	// Do write file.
// 	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
// 		return err
// 	}

// 	return nil
// }
