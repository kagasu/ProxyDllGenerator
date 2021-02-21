# ProxyDllGenerator

### Features
- Auto generate proxy dll source code(x86, x64)
- Support special character(ex. `@`)  
![](https://user-images.githubusercontent.com/1202244/108622241-ba939b80-747a-11eb-8175-02c13d9fbf2c.png)

- Modify source code template easily.

### Download
https://github.com/kagasu/ProxyDllGenerator/releases

### How to use
```
ProxyDllGenerator.exe --dll test.dll
```

### Build Requirements
- Visual Studio 2019(MSVC v142) Build Tools
- Boost
- fmt

```
vcpkg install boost:x86-windows-static
vcpkg install boost:x64-windows-static
vcpkg install fmt:x86-windows-static
vcpkg install fmt:x64-windows-static
```

# License
[MIT License](https://github.com/kagasu/ProxyDllGenerator/blob/main/LICENSE)

# Third Party License(s)
- Boost ([Boost Software License, Version 1.0](http://www.boost.org/LICENSE_1_0.txt))
- fmt ([MIT License](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst))
