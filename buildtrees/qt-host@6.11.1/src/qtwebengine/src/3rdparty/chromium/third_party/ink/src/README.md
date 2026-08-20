# Ink

The Ink library is a freehand stroke generation library. It produces smoothed,
modeled stroke shapes with brush effect shaders as mesh-based vector graphics.

This library provides the core of the implementation of the Android Jetpack
[Ink module](https://developer.android.com/jetpack/androidx/releases/ink), which
is currently in alpha. While the implementation is well-tested, the developers
of this library are not currently making hard guarantees about interface
stability. We probably will attempt to introduce a versioned release process
after the Jetpack module stable release.

## How to Build and Test

### Bazel

#### Prerequisites:

*   Bazel 7: https://bazel.build/install
*   Android NDK: https://developer.android.com/ndk/downloads
    *   Download and set `ANDROID_NDK_HOME` to point to it.
    *   Alternately use `sdkmanager` or Android Studio:
        https://developer.android.com/studio/projects/install-ndk
*   You may need to install libtinfo for clang
    *   `sudo apt-get install libtinfo5`

Ink can be built and tested from the repo root:

```shell
bazel test --config=linux ink/...
```

## Library Structure

Ink consists of a set of modules that can be used separately. You should only
need to include the parts of the library that you need.

```
 ┌──────────┐ ┌───────┐
 │Rendering │ │Storage│
 └────┬─────┘ └──┬────┘
      │          │
      ▼          │
   ┌───────┐     │
   │Strokes│◄────┘
   └─┬────┬┘
     │    │
     │    ▼
     │ ┌────────┐
     │ │Geometry│
     │ └───┬────┘
     │     │
     ▼     ▼
 ┌─────┐ ┌─────┐
 │Color│ │Types│
 └─────┘ └─────┘
```

*   `color`: color spaces, encoding, and format conversion.
*   `types`: utility types; time, units, constants, small arrays.
*   `geometry`: geometric types (point, segment, triangle, rect, quad), meshes,
    transforms, utility functions, and algorithms (intersection, envelope).
*   `strokes`: the primary `Stroke` data type and `InProgressStroke` builder.
*   `rendering`: rendering utilities for strokes. Currently only has support for
    android.graphics.Mesh based rendering.
*   `storage`: Protobuf serialization utilities for `Stroke` and related types.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for details on sending a PR.

## Contact

Use GitHub Issues to file feedback: https://github.com/google/ink/issues
