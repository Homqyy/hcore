# HCore

## 介绍

- 这是一个C接口库。决定编写这个接口库的起因是因为个人在Nginx开发过程中，爱上了其优秀的编码规范、API和数据结构。比如：`ngx_str_t`、`ngx_queue_t`等。这些接口不复用率高，而且有很好的性能优势。但遗憾的是，它们只能在Nginx的项目中使用，有较强的绑定关系。为此，我决定着手将它们从Nginx中分离出来，从而诞生了`HCore`的原型。目前该接口仅支持`Linux x86_64`系统。

- 我期望`HCore`是一个能承担高复用和高性能的职责的接口库。

---

## 开发

- 开发环境
    - 在拥有容器环境的主机上：
        - 方法一（推荐）：支持通过`VSCode`的`Remote - Containers`插件进行容器式开发以这种方式可以免除搭建开发环境的困恼
            - 通过`Remote - Containers`插件运行开发环境后，执行`./build.sh`进行构建即可。
        - 方法二：执行命令`./build_on_docker.sh build`进行构建
    - 在非容器环境的主机上：
        1. 安装依赖
            - cmake：v3.0.0以上
            - gcc
        2. 构建：`./build.sh`

---

## 目录结构介绍

```bash
hcore
├── build_on_docker.sh      # 在容器中构建代码的脚本
├── build.sh                # 构建脚本
├── CMakeLists.txt.in       # 用来生成 CMakeLists.txt 的模板文件
├── Config.cmake.in         # 用来生成 配置信息 的模板文件
├── Dockerfile              # 用来构建项目容器的Dockerfile
├── googletest              # 单元测试框架，采用googletest
├── hcore_config.h.in       # 用来生成 hcore_config.h 的模板文件
├── include                 # 头文件目录
│   ├── hcore_array.h
│   ├── hcore_astring.h
│   ├── hcore_base.h
│   ├── hcore_buf.h
│   ├── hcore_connection.h
│   ├── hcore_constant.h
│   ├── hcore_count.h
│   ├── hcore_crc32.h
│   ├── hcore_debug.h
│   ├── hcore_event.h
│   ├── hcore_hash.h
│   ├── hcore_htb.h
│   ├── hcore_inet.h
│   ├── hcore_io.h
│   ├── hcore_ipc.h
│   ├── hcore_lib.h
│   ├── hcore_list.h
│   ├── hcore_log.h
│   ├── hcore_map.h
│   ├── hcore_pack.h
│   ├── hcore_path.h
│   ├── hcore_pid.h
│   ├── hcore_pool.h
│   ├── hcore_queue.h
│   ├── hcore_rbtree.h
│   ├── hcore_shmtx.h
│   ├── hcore_shpool.h
│   ├── hcore_string.h
│   ├── hcore_time.h
│   └── hcore_types.h
├── LICENSE                 # License文件
├── MultiCPackConfig.cmake  # 用来打包的配置文件
├── README.md
├── src                     # 源码目录
│   ├── hcore_array.c
│   ├── hcore_astring.c
│   ├── hcore_base.c
│   ├── hcore_buf.c
│   ├── hcore_connection.c
│   ├── hcore_count.c
│   ├── hcore_crc32.c
│   ├── hcore_debug.c
│   ├── hcore_event.c
│   ├── hcore_hash.c
│   ├── hcore_htb.c
│   ├── hcore_inet.c
│   ├── hcore_io.c
│   ├── hcore_ipc.c
│   ├── hcore_lib.c
│   ├── hcore_list.c
│   ├── hcore_log.c
│   ├── hcore_map.c
│   ├── hcore_pack.c
│   ├── hcore_path.c
│   ├── hcore_pid.c
│   ├── hcore_pool.c
│   ├── hcore_queue.c
│   ├── hcore_rbtree.c
│   ├── hcore_shmtx.c
│   ├── hcore_shpool.c
│   ├── hcore_string.c
│   └── hcore_time.c
├── tests                   # 单元测试目录
│   ├── test_astring.cpp
│   ├── test_shmtx.cpp
│   └── test_shpool.cpp
├── tools-dev               # 开发工具
└── upload.sh               # 上传包的脚本

5 directories, 71 files
```

---

## 单元测试

- 待完善

---

## API导航

- 待完善

### 基础（base）

- 待完善

### 时间（time）

- 待完善

### 字符串(string)

- 待完善

### 动态字符串(astring)

- 待完善

### 位图(map)

- 待完善

### 日志(log)

- 待完善

### 调试(debug)

- 待完善

### 缓冲区(buf)

- 待完善

### 队列(queue)

- 待完善

### 哈希表-定址法(htb)

- 待完善

### 动态数组(array)

- 待完善

### 内存池(pool)

- 待完善

### 共享内存池(shpool)

- 待完善

### 进程间的自旋锁(shmtx)

- 待完善
