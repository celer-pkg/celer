# Introduce project

&emsp;&emsp;Each project has its own features, such as the dependencies of the project, the global cmake variables, the environment variables, the C/C++ macros, and the compile options. Celer recommends defining a respective configuration file for each project to describe the features of the project.

## 1. Project toml file

Let's take a look at the example project configure file, `project_003.toml`:

```toml
ports = [
    "x264@stable",
    "sqlite3@3.49.0",
    "ffmpeg@3.4.13",
    "zlib@1.3.1",
    "opencv@4.5.1"
]

vars = [
    "CMAKE_VAR1=value1",
    "CMAKE_VAR2=value2"
]

envs = [
    "ENV_VAR1=/home/ubuntu/ccache"
]

micros = [
    "MICRO_VAR1=111",
    "MICRO_VAR2"
]

compile_options = [
    "-Wall",
    "-O2"
]
```

The following are fields and their descriptions:

| Field | Description |
| --- | --- |
| ports | Define the third-party libraries that the current project depends on. |
| vars | Define the global cmake variables that the current project required. |
| envs | Define the global environment variables that the current project required. |
| micros | Define the C/C++ macros that the current project required. |
| compile_options | Define the compile options that the current project required. |
