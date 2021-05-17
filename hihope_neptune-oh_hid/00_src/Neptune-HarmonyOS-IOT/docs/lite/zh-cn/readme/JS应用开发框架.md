# JS应用开发框架<a name="ZH-CN_TOPIC_0000001078402300"></a>

-   [简介](#section11660541593)
-   [目录](#section1464106163817)
-   [约束](#section1718733212019)
-   [使用targets](#section1460013282612)
-   [使用runtime-core](#section1460223932718)
-   [相关仓](#section11703194974217)

## 简介<a name="section11660541593"></a>

JS应用开发框架，提供了一套跨平台的类web应用开发框架，通过Toolkit将开发者编写的HML、CSS和JS文件编译打包成JS Bundle，解析运行JS Bundle，生成native UI View组件树并进行渲染显示。通过支持三方开发者使用声明式的API进行应用开发，以数据驱动视图变化，避免大量的视图操作，大大降低应用开发难度，提升开发者开发体验。

JS应用框架模块组成如下图所示：

**图 1**  JS应用开发框架<a name="fig1771713591545"></a>  
![](figures/JS应用开发框架.png "JS应用开发框架")

## 目录<a name="section1464106163817"></a>

JS应用开发框架源代码在/foundation/ace/ace\_engine\_lite下，目录结构如下图所示：

```
/foundation/ace/ace_engine_lite
├── frameworks      # 框架代码目录
│   ├── examples    # 示例代码目录
│   ├── include     # 头文件目录
│   ├── packages    # 框架JS实现存放目录
│   ├── src         # 源代码存放目录
│   ├── targets     # 各目标设备配置文件存放目录
│   └── tools       # 工具代码存放目录
├── interfaces      # 对外接口存放目录
│   └── innerkits   # 对内部子系统暴露的头文件存放目录
│       └── builtin # JS应用框架对外暴露JS三方module API接口存放目录
└── test            # 测试用例目录
```

## 约束<a name="section1718733212019"></a>

-   语言版本：
    -   C++11版本或以上
    -   JavaScript ES5.1


-   框架运行内存通常分为如下组成部分：
    -   JS引擎运行时内存：可调，取决于具体设备应用复杂度，通常建议64K\~512K
    -   框架本身native内存：在百K级的内存设备上，建议预分配一个与native UI共用的内存池，用于native内存的管理

-   框架针对不同的芯片平台和底层OS能力，规格有所区别：
    -   Cortex-M RAM/ROM
        -   JS引擎内存池: 建议大于48K
        -   RAM：建议与native UI共用内存池，大于80K
        -   ROM:  \> 300K （包含JS应用框架，以及native UI和JS引擎等强相关子系统）

    -   Cortex-A RAM/ROM
        -   JS引擎内存池: 建议大于128K
        -   RAM：建议大于512K
        -   ROM：\> 2M （包含JS应用框架，以及native UI和JS引擎等强相关子系统）



## 使用targets<a name="section1460013282612"></a>

JS应用框架实现主要包含两部分：

-   native部分：使用C++进行编写，是框架主体实现；
-   JavaScript部分：提供JS应用框架对用户JS文件的运行时支持，并通过向引擎暴露一些全局方法和对象，支撑JS运行时与native框架之间的交互。

JS应用框架通过一些特性宏来定制不同平台上参与编译的功能代码，该部分特性宏定义在 foundation/ace/ace\_engine\_lite/frameworks/targets 目录下头文件内，目录结构如下：

```
/foundation/ace/ace_engine_lite/frameworks/targets
├── default/
│   └── acelite_config.h
├── linux/                # linux环境配置文件目录
│   └── acelite_config.h
├── liteos_a/             # LiteOS A核环境配置文件目录
│   └── acelite_config.h
├── liteos_m/             # LiteOS M核环境配置文件目录
│   └── acelite_config.h
├── platform_adapter.cpp
├── platform_adapter.h
└── simulator/            # 模拟器环境配置文件目录
    ├── acelite_config.h
    └── BUILD.gn
```

注：目前开源部分比较完整的是LiteOS A核的真机目标编译，使用ninja\(BUILD.gn\)构建，其他如 simulator \(CMake+MingW\) ，LiteOS M核\(IAR\)目标，还未完全开源，在适配完成后会逐渐放出。以下仅通过示例说明targets目录在构建不同目标时的作用。

在编译不同的平台目标时，需要使用对应平台目录下的acelite\_config.h头文件，这可以通过配置编译时的搜索路径来进行，以下以ninja和CMake构建工具为例进行示例：

-   ninja：

    ```
      if (ohos_kernel_type == "liteos_a" || ohos_kernel_type== "liteos_m" ||
          ohos_kernel_type == "liteos_riscv") { # 通过目标内核平台选择不同的头文件搜索路径
        include_dirs += [ "targets/liteos_a" ]
      } else if (ohos_kernel_type == "linux") {
        include_dirs += [ "targets/linux" ]
      }
    ```


-   CMake：

    ```
    ......
    set(ACE_LITE_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/targets/simulator") # 模拟器编译搜索路径使用targets/simulator
    set(ACE_LITE_INNERKITS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../interfaces/innerkits/builtin")
    set(JSFWK_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
    set(JSFWK_INNERKITS_BUILTIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../foundation/ace/ace_engine_lite/interfaces/innerkits/builtin")
    set(JSFWK_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/core")
    set(UIKIT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../foundation/graphic/lite")
    set(THIRTY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../third_party")
    set(JSFWK_SIMULATOR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../tools/developer_tools_lite/graphic_tool/simulator")
    set(AAFWK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../aafwk")
    set(UTILS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../utils/native/lite")
    
    # header files
    include_directories(
        ${ACE_LITE_CONFIG_PATH}
        ${JSFWK_INCLUDE_PATH}/async
        ${JSFWK_INCLUDE_PATH}/base
        ${JSFWK_INCLUDE_PATH}/context
        ${JSFWK_INCLUDE_PATH}/jsi
        ${JSFWK_SOURCE_PATH}
        ......
    ```


acelite\_config.h主要用于对应平台的特性宏开关，也可用来进行一些屏蔽平台差异的定义。如不同平台由于使用的文件系统不一致，可能存在一些固定目录路径名不一样的情况，这些有区别的常量可以放在这里进行定义，如下：

-   liteos\_a/acelite\_config.h

    ```
    #define JS_FRAMEWORK_PATH "//system/ace/bin/"
    ```


-   simulator/acelite\_config.h

    ```
    #define JS_FRAMEWORK_PATH "..\\..\\..\\jsfwk\\packages\\runtime-core\\build"
    ```


## 使用runtime-core<a name="section1460223932718"></a>

为了实现单向数据绑定机制，JS应用框架使用JavaScript语言实现了一套简单的数据劫持框架，称之为runtime-core，目录结构如下所示：

```
/foundation/ace/ace_engine_lite/frameworks/packages
└── runtime-core
    ├── .babelrc          # babel配置文件
    ├── contribution.md
    ├── .editorconfig     # IDE配置文件
    ├── .eslintignore     # ESLint配置文件，可以设置不进行ESLint扫描的目录或文件
    ├── .eslintrc.js      # ESLint配置文件，可以配置扫描规则
    ├── .gitignore
    ├── package.json      # NPM包管理文件
    ├── package-lock.json # NPM依赖版本锁定文件
    ├── .prettierrc       # 代码格式化规则配置文件
    ├── scripts           # 编译脚本存放目录
    │   ├── build.js      # 编译脚本
    │   └── configs.js    # Rollup配置文件
    ├── .size-snapshot.json
    └── src               # 源代码
        ├── core          # ViewModel核心实现目录
        │   └── index.js
        ├── index.js
        ├── observer      # 数据劫持部分代码实现目录
        │   ├── index.js
        │   ├── observer.js
        │   ├── subject.js
        │   └── utils.js
        ├── profiler      # profiler目录
        │   └── index.js
        └── __test__      # 测试用例目录
            └── index.test.js
```

支持的NPM命令有：

-   npm run build

    JS应用框架所集成的JS引擎仅支持ES5.1语法，runtime-core源代码是使用ES6语法书写的。因此选择使用rollup做为打包工具，配合babel实现对语法进行降级处理。命令行中执行npm run build，会在build目录下输出打包结果，输出结果如下所示：

    ```
    build/
    ├── framework-dev.js     // 开发环境使用的框架代码（未压缩混淆）
    ├── framework-dev.min.js // 开发环境使用的框架代码（已压缩混淆）
    ├── framework.js         // 生产环境使用的框架代码（未压缩混淆）
    └── framework.min.js     // 生产环境使用的框架代码（已压缩混淆）
    ```

-   npm run test

    runtime-core使用jest进行单元测试，在命令行中执行npm run test即可触发。


## 相关仓<a name="section11703194974217"></a>

[ace\_engine\_lite](https://gitee.com/openharmony/ace_engine_lite/blob/master/README_zh.md)

