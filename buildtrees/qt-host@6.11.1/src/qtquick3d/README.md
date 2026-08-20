# Qt Quick 3D

Qt Quick 3D is a module within the Qt framework that provides a high-level interface for creating 3D content for user interfaces.

![Dragon](src/quick3d/doc/images/dragon.jpg)

## Key Features

* Spatial (3D) Scene Graph
* Mixing 2D and 3D Qt Quick Content
* Physically Based Rendering (PBR) materials
* Punctual and Image based lighting
* Powerful Custom Material and Effect System
* GLTF 2.0 Model Import
* Animation Support
    * QML based animations
    * Keyframe animations
    * Skeletal animations
    * Morph animations
* 3D input handling and picking
* 3D Particle System
* Spatial Audio (via Qt Multimedia module)
* 3D Physics (vi Qt Quick3DPhysics module)

## Getting Started

There are a couple of ways to get started with Qt Quick 3D.  The easiest way is to download the Qt Installer and install the Qt Quick 3D module.  Alternatively, you can build Qt Quick 3D from source. The following sections will walk you through the process.

### Qt Installer

To install Qt Quick 3D via the Qt Installer, you will need to download the Qt Installer from the [Qt website](https://www.qt.io/download).  Once you have downloaded the installer, you can install Qt Quick 3D by selecting the "Qt Quick 3D" module in the installer.  In addition to selecting the Qt Quick 3D Module, you should also enable the following modules for the full experience:
* Qt Quick Timeline (required for keyframe animations)
* Qt Shader Tools (required for runtime shader generation)
* Additional Libraries
    * Quick: 3D Physics (required for physics)
    * Qt Multimedia (required for spatial audio)

### Building Qt Quick 3D

Since you are reading this, you are probably interested in building Qt Quick 3D from source.  If you are building the Qt super project, then Qt Quick 3D will be built automatically.  If you are building Qt Quick 3D as a standalone project, then you will need to follow the instructions below.

The following modules are required to build Qt Quick 3D:
- QtShaderTools
- QtDeclarative
- QtQuickTimeline

After cloning this repository, you will need to run the following commands to build Qt Quick 3D:
```
git clone https://code.qt.io/qt/qtquick3d.git
cd qtquick3d
git submodule update --init --recursive
cd ../your/build/directory/
mkdir qtquick3d
cd qtquick3d
../qtbase/bin/qt-configure-module ../../path/to/qt/source/qtquick3d
ninja # or make/nmake
ninja install # or make/nmake install
```

### Usage

Qt Quick 3D is primarily used via the QML API.  The following example shows how to create a simple 3D scene with a cube and a sphere:
```
import QtQuick
import QtQuick3D

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Simple 3D Scene")

    // Create a view and 3D scene
    View3D {
        anchors.fill: parent
        camera: activeCamera
        PerspectiveCamera {
            id: activeCamera
            z: 400
        }

        DirectionalLight {
            color: "white"
        }

        Model {
            x: -100
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "red"
            }
        }
        Model {
            x: 100
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "green"
            }
        }
    }
}
```

Saving the above example as `main.qml` and running the following command will launch the example:
```
qmlscene main.qml
```

## Examples and Demos

Qt Quick 3D comes with a number of examples and demos that demonstrate how to use the various features of the module. You can find these examples in the examples/quick3d directory of your Qt installation.

To run the examples, open the CMakeLists.txt file for the example you want to run in Qt Creator, and then click the "Run" button.  It is also possible to find the examples from within Qt Creator by opening the "Examples" pane and selecting the "Qt Quick 3D" category.

You can see the full list of examples here: [Qt Quick 3D Examples](https://doc-snapshots.qt.io/qt6-dev/quick3d-examples.html)

## Documentation

The Qt Quick 3D documentation is available as a submodule of the Qt Framework's documentation. You can find it in the Qt documentation [online](https://doc-snapshots.qt.io/qt6-dev/qtquick3d-index.html), or you can build it locally from the source code.

## Reporting Issues

If you encounter any issues while using Qt Quick 3D, you can report them on the [Qt bug tracker](https://bugreports.qt.io/). Before submitting a new issue, please check the existing issues to see if the issue has already been reported, and make sure to select the "Quick: 3D" component when creating a new issue.

## Contributing

We welcome contributions to Qt Quick 3D. If you are interested in contributing, please read the [Qt Contribution Guidelines](https://wiki.qt.io/Qt_Contribution_Guidelines) for more details.

## License

Qt Quick 3D is available under the commercial license from The Qt Company. In addition, it is available under the [GNU General Public License, version 3](http://www.gnu.org/licenses/gpl-3.0.html). Further 3rd party Licenses and Attributions can be found in the LICENSES folder, as well as documented [here](https://doc-snapshots.qt.io/qt6-dev/qtquick3d-index.html#licenses-and-attributions).
