# Deploy 命令

&emsp;&emsp;**Deploy**命令执行所有必需的第三方库的完整构建和部署周期，根据选择的平台和项目配置。它同时生成一个**toolchain_file.cmake**文件，用于与基于CMake的项目无缝集成。

## 命令语法

```shell
celer deploy
```

当部署成功后，**toolchain_file.cmake**文件将在项目根目录生成，您可以使用它来开发你的项目，并且可以选择任何支持CMake的IDE来开发，例如Visual Studio、CLion、Qt Creator、Visual Studio Code等。
