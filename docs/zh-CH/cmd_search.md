# Tree command

&emsp;&emsp;Celer搜索命令根据指定的模式搜索匹配的端口。

## 命令语法

```shell
celer search [flags]
```

## 用法示例

### 1. 按名称精确搜索

```shell
celer search ffmpeg@5.1.6

--------------------------

[Search result]:
ffmpeg@5.1.6
```

### 2. 按模式搜索

```shell
./celer search open*

-------------------------

[Search result]:
opencv@4.11.0
openssl@1.1.1w
openssl@3.5.0
```

>支持的搜索模式语法：  
>**1.** xxx*: 匹配以xxx开头的任意字符串。  
>**2.** *xxx: 匹配以xxx结尾的任意字符串。  
>**3.** *xxx*: 匹配包含xxx的任意字符串。
