## Qt WebSockets

### Package Brief

`QtWebSockets` is a pure Qt implementation of WebSockets - both client and server.
It is implemented as a Qt add-on module that can easily be embedded into existing Qt projects.
Its only dependency is Qt.

### Documentation

The documentation can be found in the following places:
* The online Qt documentation:
   * [Qt WebSockets documentation](https://doc.qt.io/qt-6/qtwebsockets-index.html)
* Build from source:
   * [Building Qt Documentation](https://wiki.qt.io/Building_Qt_Documentation)

*Note that the documentation links in this document will always be for the
latest Qt 6 version. If you need to browse the documentation for a specific
version, you can [browse the archives](https://doc.qt.io/archives/).*

### Features

* Client and server capable
* Text and binary sockets
* Frame-based and message-based signals
* Strict Unicode checking
* WSS and proxy support

### Compliance

`QtWebSockets` is compliant with [RFC6455](http://datatracker.ietf.org/doc/rfc6455/?include_text=1) and has been tested with the [Autobahn Testsuite](http://autobahn.ws/testsuite).

### Missing Features

* Extensions

### Build Process

Building the package/repository does depend on the Qt packages listed in dependencies.yaml.
Further dependencies to system packages are listed in the configure output.

See the [documentation](https://doc.qt.io/qt-6/build-sources.html) for general
advice on building the Qt framework and its modules from sources. Further
information on how to build from source is also available in the
[wiki](https://wiki.qt.io/Building_Qt_6_from_Git).

### Report an Issue

If you spot a bug, follow [these](https://doc.qt.io/qt-6/bughowto.html)
steps to report it.

### Contribute to Qt

We welcome contributions to Qt! If you'd like to contribute, read the
[Qt Contribution Guidelines](https://wiki.qt.io/Qt_Contribution_Guidelines).

### Licensing

Qt is available under various licenses. For details, check out the
[license documentation](https://doc.qt.io/qt-6/licensing.html).
