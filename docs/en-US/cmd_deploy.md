# Deploy command

&emsp;&emsp;The deploy command performs a complete build and deployment cycle for all required third-party libraries based on the selected platform and project configuration. It simultaneously generates a **toolchain_file.cmake** for seamless integration with CMake-based projects.

## Command Syntax

```shell
celer deploy
```

After deploy successfully, the **toolchain_file.cmake** file will be generated in the project root directory, then you can use it to develop your project with any CMake-based IDE, such as Visual Studio, CLion, Qt Creator, Visual Studio Code, etc.