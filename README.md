# ProxyDllGenerator

### Features
- Auto generate proxy dll source code(x86, x64)

### Download

### How to use
```
ProxyDllGenerator --dll test.dll
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

# Third Party License(s)
- Boost ([Boost Software License, Version 1.0](http://www.boost.org/LICENSE_1_0.txt))
- fmt ([MIT License](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst))
