# 如何全局集成

celer 只是一个独立的可执行二进制文件。但是为了方便全局调用以及命令智能提示，我们支持将 celer安装到系统目录并同时生成bash/powershell的配置文件，这样你就可以在任何地方运行celer了, 执行如下命令：

```
./celer integrate           

SUCCESS: Specified value was saved.
[integrate] celer.exe --> C:\Users\xxx\AppData\Local\celer\celer.exe

[✔] ======== celer is integrated. ========
```