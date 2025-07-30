问题    | 状态
-------| -----
不是所有的编译工具都是必填的，需要支持读field上的自定义tag(改成了判空，空则不校验，但CC和CXX是默认必须校验的)  | ✔
通过获取资源文件的大小和本地文件大小对比，来判断是否需要重新下载覆盖  | ✔
platform添加native的支持  | ✔
提供DevMode cli参数，用于在toolchain调用celer时候不输出过程信息  | ✔
cli未提供create_platform的支持  | ✔
go内部拼装PATH环境路径，估计是有问题的，可能不该这么用  | ✔
toolchain里定义CC和CXX时候后面拼上 "--sysroot=xxx"  | ✔
三方库如果编译后产生pkg-config文件，需要加入系统变量（ffmpeg在通过pkg-config寻找libx265）  | ✔
path不存在，应该先尝试用下载后的文件解压，而不是直接重复下载，重新下载的前提是md5等校验没通过  | ✔
每个installed的库添加platform目录，目的是为了支持不同平台的库共存  | ✔
执行setup打印所有已经准备好的tool和已经安装的port  | ✔
环境变量用os.PathListSeparator拼接  | ✔
将download的资源统一解压到内部的tools目录  | ✔
拓展project，将packages的配置文件放到project目录下  | ✔
menu cli的实现可以考虑用面向对象思维简化  | ✔
git在下载代码时候没有过程log  | ✔
添加-install参数，用于指定三方库的编译  | ✔
--sysroot和--cross-prefix自动设置  | ✔
git 同步代码需要优化  | ✔
预编译好的三方库需要支持remove  | ✔
支持remove功能, 同时支持recursive 模式  | ✔
makefile的安装路径和依赖寻找路径应该自动管理 | ✔
install 三方库的时候，如果已经配置到project里了，无需指定版本  | ✔
cmd/cli缺少创建和选择project的功能  | ✔
一个项目配置同名不同版本的port是禁止的  | ✔
支持编译库为native的  | ✔
usage 里的颜色需要优化  | ✔
有的toolchain或者tool不是绿色版，不能托管到celer里，需要绝对路径指向  | ✔
在project中支持配置cmake变量和C++宏  | ✔
makefile编译前不支持配置环境变量，例如：export CFLAGS="-mfpu=neon"  | ✔
三方库以目录方式维护，内部放不同版本的配置  | ✔
终端输出实现需要再简化  | ✔
当tool不存在，在执行install的时候不会触发下载  | ✔
支持打patch  | ✔
第一次使用交互需要优化  | ✔
cmake_config的配置独立于version文件之外  | ✔
有些pc文件产生做share目录，而不是lib目录，需要统一移动到lib目录（libz）  | ✔
通过一个中间临时目录来实现收集install的文件清单  | ✔
支持通过命令创建tool和port  | ✔
支持编译缓存共享  | ✔
内部出现同一个库的不同版本依赖情况给与报错提示 | ✔
支持clone时候连同submodule一起clone  | ✔
支持meson  |  ✔
支持ninja  |  ✔
将buildtype抽象到各个buildsystem里 | ✔
支持autotools  | ✔
支持编译三方库作为dev | ✔
用-purge代替-uninstall -purge | ✔
完善cmd描述，并告知候选参数是什么 | ✔
支持在project里覆盖默认port的配置  | ✔
支持tgz解压 | ✔
arguments改为options |  ✔
下载过程中的文件名不能直接是目标名，先作为临时文件，下载完成后再重命名 | ✔
package名字里的build type统一小写 | ✔
增加update功能，可以指定glog@1.2.3, 如果不指定则update所有仓库 | ✔
对于有些三方库，部分系统平台需要，可以通过"libiconv@1.14@windows"来指定 | ✔
支持${PYTHON3_PATH}表达式解析 | ✔
在celer.toml | ✔
多个executable执行，之间的log空行过大 | ✔
通过cpp_standard=17来指定跨平台的c++标准 | ✔
将所有的tools配置到一个清单里，然后自动跟编译工具关联，不再统一在一个platform里绑定 | ✔
支持windows下工作  | ✔
支持vswhere自动识别msvc的编译环境 | ✔
在源码里存放的.patched文件里添加patch文件名，便于精确查询patch是否已经打过 | ✔
在windows下，devDep还没适配好 | ✔
perl makefiles需要和普通的makefiles区分 | ✔
判断python库是否已经安装，而不是每次都安装 | ✔
windows支持自动MSVC识别(vswhere.exe -legacy -prerelease -format json) | ✔
动态生成的cmake config文件（windows还没测试）| ✔
如何pattern没有值，意思是全平台支持 | ✔
remove命令缺少命令提示 | ✔
cmake_config支持自动识别，无需平台绑定hardcode | ✔
在windows里cmake在configure期间通过vswhere识别安装的msvc的名字，然后自动设置 | ✔
命令提示不支持--build-type, --dev这种tag的提示 | ✔
windows platform的host = "x86_64-w64-mingw32"需要重新考虑 | ✔
通过post_install_windows的方式支持平台化的配置 | ✔
install命令允许指定--force或者-f | ✔
windows支持将celer路径写到系统的PATH里 | ✔
检查所有packages生成静态库文件的库 | ✔
build -r -f 支持递归强制编译 | ✔
用build_shared = "--enable-shared|--disable-shared" 方式支持库类型编译 | ✔
支持clean xx | ✔
让凡是dev目录里的bin都无需设置LD_LIBRARY_PATH | ✔
让dev中的bin默认以相对路径寻找lib里的库 | ✔
增加autoremove删除未依赖的库 | ✔
meson默认支持-Dlibdir=lib | ✔
编译期间将明确依赖的库拷贝到tmp/deps里，然后指向这里寻找依赖的库 | ✔
autoconf, libtools, m4都推荐通过源码即时安装 | ✔
clean 命令执行能同时清理代码 | ✔
autoremove 同时删除无需的dev库 | ✔
给makefile项目默认提供options = ["--host=${HOST}"] | ✔
解压依赖库到tmp/dep时候考虑去重 | ✔
支持命令celer tree ffmpeg@3.4.11 查看指定库的内部子依赖 | ✔
windows下执行多条命令用"&&"连接, 无效 | ✔
celer clean xxx 支持-f递归清理，支持--dev 清理 | ✔
一键删除非当前平台的编译缓存目录和log文件 | ✔
执行tree命令，遇到循环依赖时候没有提示而是进入了死循环 |  ✔
编译器路径用绝对路径配置，否之会命中不对 | ✔
支持 celer -upgrade 升级  | ✘
下载的库暂不支持生成cmake config文件  | ✘
如果发现资源包size跟最新不匹配，即便已经解压了也要重新下载 | ✘
支持export导出所有编译资源功能 | ✘
支持在project里定义CMAKE_CXX_FLAGS和CMAKE_C_FLAGS，以及LDFLAGS | ✘
校验是否真的installed还需要判断文件是否存在 | ✘
支持offline模式 | ✘
支持download缓存，目录区别与库 | ✘
binary库添加-L和-Wl,-rpath-link | ✘
固定终端第一行显示当前在进行的工作事项 | ✘
build_tools下载过程呈现相信信息 | ✘
在windows里考虑不显示默认environment里的值，只显示传入msys2里的环境变量的值 | ✘
全局指定不同的build_type对应的-g, -O, -O2, -Os等参数 | ✘
checkBuildTools考虑每次append是否会重复添加 | ✘
汇总所有的buildtools一次性下载？ | ✘
支持在windows下交叉编译Linux的库 | ✘
下载的三库支持指定额外的includeDirs和libDirs | ✘
如果以dev编译，得在编译前检查安装gcc等 | ✘
支持compile_options定义 | ✘
支持 build_static = "--" 方式屏蔽编译静态库 | ✘
支持./celer refer xxx查询依赖xxx的库 | ✘
支持./celer search open*** 查询匹配的库 | ✘
packages下再分子目录，结构跟cache_dir类似，为了方便直观的看到 | ✘
支持格式化命令参数输出 | ✘