# DFX<a name="EN-US_TOPIC_0000001075771948"></a>

-   [Basic Concepts](#section5635178134811)

[Design for X](https://en.wikipedia.org/wiki/Design_for_X)  \(DFX\) refers to the software design that aims to improve the quality attributes in OpenHarmony. It mainly consists of two parts: design for reliability \(DFR\) and design for testability \(DFT\).

The DFX subsystem provides the following functions:

-   HiLog: Implements the logging function. It is applicable to Mini-System Devices \(reference memory ≥ 128 KB\), Small-System Devices \(reference memory ≥ 1 MB\), and Large-System Devices \(reference memory ≥ 1 GB\).


## Basic Concepts<a name="section5635178134811"></a>

**Logging**

Logging means to record the log information generated during system running so you can understand the running process and status of the system or applications.

**Distributed call chain tracing**

In a distributed system, a service may traverse multiple software modules, with control commands and data transmitted over intra-process, inter-process, and inter-device communication interfaces. To help you understand such complex communication processes and locate service faults efficiently, the DFX subsystem provides a distributed call chain tracing framework.

**Thread suspension detection**

If a thread is trapped in an infinite loop or the kernel state \(for example, Uninterruptable Sleep, Traced, Zombie, or synchronous wait\) when it is running, the thread cannot respond to normal service requests and cannot detect and recover from faults by itself. To detect and locate this type of faults, the DFX subsystem provides a simple watchdog mechanism by inserting detection probes to the process nodes that are prone to suspension. This ensures that suspension faults can be detected and logs can be collected.

