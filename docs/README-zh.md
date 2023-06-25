HCore               {#mainpage}
==========

## 介绍

这是一个高性能得C接口库。

决定编写这个接口库的起因是因为在Nginx开发过程中，爱上了其优秀的编码规范、API和数据结构。比如：`ngx_pool_t`, `ngx_str_t`、`ngx_queue_t`等。这些接口复用率高而且有很好的性能优势。但遗憾的是，它们只能在Nginx的项目中使用，有较强的绑定关系。为此，我决定着手将它们从Nginx中分离出来，从而诞生了`HCore`的原型。目前该接口仅支持`Linux x86_64`系统。

我期望`HCore`是一个能承担高性能，并称为优秀C项目的基础库。

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

用于记录程序日志的日志接口。支持常见的8个日志级别，且能够用`_HCORE_DEBUG`去控制调试日志是否编译到程序中，以区分调试版本和正式版本。

#### 日志级别

- HCORE_LOG_STDERR 0
- HCORE_LOG_EMERG  1
- HCORE_LOG_ALERT  2
- HCORE_LOG_CRIT   3
- HCORE_LOG_ERR    4
- HCORE_LOG_WARN   5
- HCORE_LOG_NOTICE 6
- HCORE_LOG_INFO   7
- HCORE_LOG_DEBUG  8
- HCORE_LOG_UNSET  ((hcore_uint_t)-1)

#### 示例

待完善