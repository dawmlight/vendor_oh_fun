# 项目介绍<a name="ZH-CN_TOPIC_0000001124444687"></a>

-   [简介](#section11660541593)
-   [目录](#section1464106163817)
-   [约束](#section13591154411)

## 简介<a name="section11660541593"></a>

Wi-Fi Aware模块提供了进场通信能力，该模块可被上层应用所使用。

## 目录<a name="section1464106163817"></a>

```
foundation/communication/wifi_aware/         # 模块目录
├── frameworks
│  └── source                            # 模块源文件
├── hals                                  # 硬件抽象层头文件
└── interfaces
    └── kits                              # 模块对外提供接口文件
```

## 约束<a name="section13591154411"></a>

WiFi Aware模块使用C语言编写，目前仅支持Hi3861开发板。如果您想接入其它芯片，那么仅需要适配实现鸿蒙的集成接口即可，将实现放在device下。

