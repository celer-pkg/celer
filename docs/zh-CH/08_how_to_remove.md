# 如何删除第三方库

删除第三方库有四种方式：

1. **celer remove xxx**: celer将删除所有属于xxx的已安装文件，但是xxx的包仍然存在。如果再次安装xxx，安装速度将非常快，因为celer将尝试从`packages`文件夹恢复到`installed`文件夹。

2. **celer remove --recursive xxx**: 类似于`./celer remove xxx`，并且还将删除xxx的子依赖项文件。如果再次安装xxx，安装速度也将非常快。
3. **celer remove --purge xxx**: 类似于`./celer remove xxx`，并且还将删除xxx的包文件夹。如果再次安装xxx，celer将从源代码配置，构建和安装它。
4. **celer remove --recursive --purge xxx**: 这将从`installed`文件夹中删除xxx的文件，删除其包，以及其子依赖项。如果再次安装xxx，celer将从源代码配置，构建，并且安装它，以及其子依赖项。