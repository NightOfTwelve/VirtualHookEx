# VirtualHookEx
VirtualHookEx基于以下项目修改而来:
- [VirtualHook](https://github.com/rk700/VirtualHook). A tool for hooking application without root permission.

主要修改:
- 使用[AndHook](https://github.com/rrrfff/andhook)替换[YAHFA](https://github.com/rk700/YAHFA)，从而实现对Dalvik虚拟机的支持
- [lib\src\main\java\com\lody\virtual\client\VClientImpl.java](https://github.com/rrrfff/VirtualHookEx/blob/master/lib/src/main/java/com/lody/virtual/client/VClientImpl.java)，修改插件加载逻辑，方便分别在程序加载前、后注入代码
- [lib\src\main\jni\Foundation\VMPatch.cpp](https://github.com/rrrfff/VirtualHookEx/blob/master/lib/src/main/jni/Foundation/VMPatch.cpp)，修复在android 4.x上复现的一些bug，包括因现有hook方式与旧版本libhoudini不兼容导致的模拟器crash、相机组件无法使用等

插件编写:
- 准备 AndHook 支持, 此步按需操作一次即可
	- 将 AndHook 项目 src/*.class 导出为jar包: AndHook.jar
	- 将 AndHook 项目导出为apk(可不签名; 因apk文件容易被误清理故改后缀zip)得到: AndHook.zip
	- adb push AndHook.zip /sdcard/io.virtualhook
- 在项目中引用 AndHook.jar, 注意该jar不需要参与Build
- 按需实现 io.virtualhook.PreHook 和 io.virtualhook.PostHook 类, 具体可参考示例[examples\VirtualPhone](https://github.com/rrrfff/VirtualHookEx/tree/master/examples/VirtualPhone)
- 导出生成apk并push到/sdcard/io.virtualhook下

插件加载流程:
- VClientImpl.java: bindApplicationNoCheck
- 加载/sdcard/io.virtualhook/AndHook.zip作为插件的共享库
- 加载/sdcard/io.virtualhook/下其它插件(.apk/.zip/.jar), 并加载 io.virtualhook.PreHook 和 io.virtualhook.PostHook 类(如果有)
- 在程序 OnCreate 前调用 io.virtualhook.PreHook.Init(ClassLoader patchClassLoader, Context context, String processName)
- 在程序 OnCreate 后调用 io.virtualhook.PostHook.Init(ClassLoader patchClassLoader, Application app, String processName), 注意此时可能会产生线程安全问题
