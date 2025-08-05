# 如何全局集成

Celer 包管理器提供了一套强大的智能提示系统，能够帮助用户快速完成命令输入，减少记忆负担并提高使用效率。该系统支持多层次的自动补全和智能提示功能。

```
./celer integrate           

[integrate] celer --> /home/phil/.local/bin/celer

[✔] ======== celer is integrated. ========
```

## 功能详解

1. 主命令自动补全
当用户输入部分主命令时，按下 [tab] 键可自动补全完整命令

示例：
```
$ cel[tab] → $ celer
```

2. 子命令提示输入主命令后，按下 [tab] 键可显示所有可用的子命令列表，可用子命令：

```
$ celer [tab]
about       clean       create      help        install     remove  update 
autoremove  configure   deploy      init        integrate   tree
```

3. 子命令参数补全

对于特定子命令，系统会提示可用的参数选项。  
示例（configure 命令）：

```
$ celer configure [tab]
celer configure --platform/--project
```

4. 基于文件系统的智能提示

对于需要文件/包名作为参数的子命令，系统会扫描相关目录并提供智能提示：

install/remove/update/clean/tree 命令：自动扫描可用的包仓库，提供包名补全

示例：

```
$ celer install [tab]
celer install xxx/yyy/zzz
```