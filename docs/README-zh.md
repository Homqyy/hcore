HCore               {#mainpage}
==========

## 介绍

这是一个高性能得C接口库。

决定编写这个接口库的起因是因为在Nginx开发过程中，爱上了其优秀的编码规范、API和数据结构。比如：`ngx_pool_t`, `ngx_str_t`、`ngx_queue_t`等。这些接口复用率高而且有很好的性能优势。但遗憾的是，它们只能在Nginx的项目中使用，有较强的绑定关系。为此，我决定着手将它们从Nginx中分离出来，从而诞生了`HCore`的原型。目前该接口仅支持`Linux x86_64`系统。

我期望`HCore`能成为各优秀C项目的基础库，加速C开发的过程。

---

## 开发

### 初始化

获取源代码后，用以下两个命令获取子模块代码：

```bash
git submodule init
git submodule update
```

### 开发环境

在拥有容器环境的主机上：

- 方法一（推荐）：支持通过`VSCode`的`Remote - Containers`插件进行容器式开发以这种方式可以免除搭建开发环境的困恼
    - 通过`Remote - Containers`插件运行开发环境后，执行`./build.sh`进行构建即可。
- 方法二：执行命令`./build.sh -d`进行构建，该方法会构建镜像并运行容器，在容器中进行编译等操作

在非容器环境的主机上：

1. 安装依赖
    - cmake：v3.0.0以上
    - gcc
2. 构建：`./build.sh`

---

## 目录结构介绍

```sh
hcore
├── build.sh                    # 构建脚本
├── CMakeLists.txt.in           # 用来生成 CMakeLists.txt 的模板文件
├── Config.cmake.in             # 用来生成 配置信息（Config.cmake） 的模板文件
├── CPackConfig-debug.cmake     # Debug版本的打包配置文件
├── CPackConfig-release.cmake   # Release版本的打包配置文件
├── Dockerfile
├── docs                        # 文档目录
├── Doxyfile                    # Doxygen 配置文件
├── googletest
├── hcore_config.h.in
├── include                     # 头文件目录
├── Jenkinsfile
├── LICENSE                     # License文件
├── README.md -> ./docs/README-zh.md
├── src                         # 源码目录
├── tests                       # 单元测试目录
├── tools-dev                   # 开发工具
```

---

## API概述

### 日志(hcore_log)

日志文件：[hcore_log.h](./include/hcore_log.h)

用于记录程序日志的日志接口。支持常见的8个日志级别，且能够用`_HCORE_DEBUG`去控制调试日志是否编译到程序中，以区分调试版本和正式版本。

支持的日志级别从高到低分别为：

- 标准错误（Standard Error）
- 严重（Emergency）
- 警报（Alert）
- 危险的（Critical）
- 错误（Error）
- 警告（Warning）
- 通知（Notice）
- 信息（Informational）
- 调试（Debug）

#### 示例

[example/e_log.c](./example/hcore_log.c)

运行后可以在`/var/log/myqpp.log`中看到与下方类似的日志内容：

```text
2023/06/28 10:29:22 -0000 UTC [ debug] 12417 unknown: (0) This is a debug message.
2023/06/28 10:29:22 -0000 UTC [  warn] 12417 unknown: (0) This is a warning message.
2023/06/28 10:29:22 -0000 UTC [ error] 12417 unknown: (0) This is an error message.
2023/06/28 10:29:22 -0000 UTC [ debug] 12417 unknown: (0) run cleanup: 0000000001DA8390
```
