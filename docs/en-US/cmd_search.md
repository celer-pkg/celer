# Tree command

&emsp;&emsp;The search command searches ports that match the specified name with pattern.

## Command Syntax

```shell
celer search [flags]
```

## Usage Examples

**1. Search with exactly name**

```shell
celer search ffmpeg@5.1.6

--------------------------

[Search result]:
ffmpeg@5.1.6
```

**2. Search with pattern**

```shell
./celer search open*

-------------------------

[Search result]:
opencv@4.11.0
openssl@1.1.1w
openssl@3.5.0
```

>The supported pattern syntax are:  
>**1.** xxx*: Matches any string that starts with xxx.  
>**2.** *xxx: Matches any string that ends with xxx.  
>**3.** *xxx*: Matches any string that contains xxx.
