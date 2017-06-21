# VirtualHookEx
VirtualHookEx基于以下项目修改而来:
- [VirtualHook](https://github.com/rk700/VirtualHook). A tool for hooking application without root permission.

主要修改:
- 使用[AndHook](https://github.com/rrrfff/andhook)替换[YAHFA](https://github.com/rk700/YAHFA)，从而实现对Dalvik虚拟机的支持
- [lib\src\main\java\com\lody\virtual\client\VClientImpl.java](https://github.com/rrrfff/VirtualHookEx/blob/master/lib/src/main/java/com/lody/virtual/client/VClientImpl.java)，修改插件加载逻辑，方便分别在程序加载前、后注入代码
- [lib\src\main\jni\Foundation\VMPatch.cpp](https://github.com/rrrfff/VirtualHookEx/blob/master/lib/src/main/jni/Foundation/VMPatch.cpp)，修复在android 4.x上复现的一些bug，包括因现有hook方式与旧版本libhoudini不兼容导致的模拟器crash

