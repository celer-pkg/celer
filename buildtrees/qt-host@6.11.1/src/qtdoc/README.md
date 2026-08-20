# Qt Documentation

This repository contains the main Qt Reference Documentation, including
overviews, Qt topics, examples, and tutorials not specific to any Qt module.


## Documentation Projects

The documentation is organized into several documentation projects:


### QtDoc

The main Qt Reference Documentation.

Project File: doc/config/qtdoc.qdocconf

Sources: `doc/src/` (excluding `cmake/` and `platformintegration/`
    subdirectories)

Content:
  - Getting started guides and tutorials
  - Platform-specific documentation (Android, iOS, macOS, Windows, etc.)
  - Qt modules overview and reference
  - Development tools and utilities
  - Legal information and licenses
  - Examples and demos

CMake Targets:  `docs_QtDoc` or `html_docs_QtDoc`

This project also contains the documentation for examples and tutorials in
the `examples/` directory.


### QtCMake

Documentation for using Qt with CMake.

Project File: doc/src/cmake/qtcmake.qdocconf

Sources: `doc/src/cmake`

Content:
  - CMake manual and getting started guide
  - Qt CMake API reference
  - Build system configuration

CMake Targets: `docs_qtcmake` or `html_docs_qtcmake`.


### QtPlatformIntegration

Documentation for integrating Qt with native platforms.

Project File: doc/src/platformintegration/qtplatformintegration.qdocconf

Sources: `doc/src/platformintegration/`

Content:
  - Qt Platform Abstraction (QPA)
  - Platform-specific integration guides
  - Native API interaction

CMake Targets: `docs_QtPlatformIntegration` or `html_docs_QtPlatformIntegration`.


## Building Documentation

The main QtDoc documentation contains links to pages from other documentation
projects (QtCore, QtQuick, etc.). Similarly, these projects link back to
pages and topics in QtDoc.

Therefore, the most reliable approach is to build Qt documentation as part of
a complete Qt build that includes all supported Qt modules. Refer to the
README.md in qt5.git for detailed instructions.

## Building Examples

Build the examples by opening the CMakeLists.txt files in the respective
example directory.


## Writing and Improving Qt Documentation

Qt Documentation is written in QDoc syntax. See
https://doc.qt.io/qt-6/qdoc-index.html for the QDoc manual.

For more information about Qt's documentation, refer to the Qt Project wiki:
https://wiki.qt.io/Qt_Writing_Guidelines


## Writing and Improving Qt Examples

See https://contribute.qt-project.org/quips/13 for some guidance around the
Qt examples.
