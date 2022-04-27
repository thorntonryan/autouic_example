## Overview

Sample repository to demonstrate bugs associated with:

* [CMake#16776](https://gitlab.kitware.com/cmake/cmake/-/issues/16776)
* [CMake#22531](https://gitlab.kitware.com/cmake/cmake/-/issues/22531)

## Repository Layout

File / Folder   | Project Name      | Purpose
----------------|-------------------|-------------
/CMakeLists.txt | cmake_issue_16776 | Demonstrates [CMake#16776](https://gitlab.kitware.com/cmake/cmake/-/issues/16776)
/src/CMakeLists | cmake_issue_22531 | Demonstrates [CMake#22531](https://gitlab.kitware.com/cmake/cmake/-/issues/22531) has not been fully addressed <br> _AUTOUIC still triggers rebuild of unrelated compilation units_
/improved/CMakeLists.txt | qt_wrap_methods | Demonstrates notable build improvements using low level `qt_wrap_ui` and `qt_wrap_cpp`


## CMake 16776

<details>
<summary>Steps to reproduce...</summary>


Build the sample project by doing the following:

```cmd
> git clone repo
> cd repo
> cmake -S . --preset Debug
> cmake --build build\cmake_issue_16776 --target all
```

Modify a `*.ui` file (e.g. `widget2.ui`) in some fashion:
```cmd
> cmake -E touch src\widget2.ui
```

Then attempt building:
```cmd
> cmake --build build\cmake_issue_16776 --target all
[1/3] Automatic MOC and UIC for target example
```

Then build again:
```cmd
> cmake --build build\cmake_issue_16776 --target all
[1/2] Building CXX object CMakeFiles\example.dir\src\widget2.cpp.obj
[2/2] Linking CXX executable example.exe
```
</details>

As best we can tell, the problem is related to how the following build rules are generated:

```
#############################################
# Utility command for example_autogen

build example_autogen: phony CMakeFiles\example_autogen example_autogen\include\src\ui_mainwindow.h example_autogen\include\src\ui_widget1.h example_autogen\include\src\ui_widget2.h example_autogen\timestamp example_autogen\mocs_compilation.cpp example_autogen_timestamp_deps
```
*example_autogen target*

```
#############################################
# Phony custom command for CMakeFiles\example_autogen

build CMakeFiles\example_autogen example_autogen\include\src\ui_mainwindow.h example_autogen\include\src\ui_widget1.h example_autogen\include\src\ui_widget2.h | ${cmake_ninja_workdir}CMakeFiles\example_autogen ${cmake_ninja_workdir}example_autogen\include\src\ui_mainwindow.h ${cmake_ninja_workdir}example_autogen\include\src\ui_widget1.h ${cmake_ninja_workdir}example_autogen\include\src\ui_widget2.h: phony example_autogen\timestamp || example_autogen_timestamp_deps

```
*CMakeFiles\example_autogen target*

Note the paths on the generated `ui_*.h` files erroneously include a `src` folder, when they shouldn't.

The paths should instead read:

```diff
-example_autogen\include\src\ui_widget2.h
+example_autogen\include\ui_widget2.h
-${cmake_ninja_workdir}example_autogen\include\src\ui_widget2.h
+${cmake_ninja_workdir}example_autogen\include\ui_widget2.h
```

Fixing the paths allows ninja to build in one pass.

## CMake 22531


<details>
<summary>Steps to reproduce...</summary>


Build the sample project by doing the following:

```cmd
> git clone repo
> cmake -S src --preset Debug
> cmake --build build\cmake_issue_22531 --target all
```

Modify `widget2.cpp` in some fashion:
```cmd
> cmake -E touch src\widget2.cpp
```

And observe everything rebuild:
```cmd
> cmake --build build\cmake_issue_22531 --target all
[1/6] Automatic MOC and UIC for target example
[2/5] Building CXX object CMakeFiles\example.dir\widget1.cpp.obj
[3/5] Building CXX object CMakeFiles\example.dir\mainwindow.cpp.obj
[4/5] Building CXX object CMakeFiles\example.dir\widget2.cpp.obj
[5/5] Linking CXX executable example.exe
```
_Note neither mainwindow.cpp nor main.cpp even include "widget2.h", so the rebuild is doubly confusing_

Asking ninja to explain:
```cmd
> cmake -E touch src\widget2.cpp
> cd build\cmake_issue_22531
> ninja -n -d explain
C:\projects\scratchpad\autouic\build\cmake_issue_22531>ninja -n -d explain
ninja explain: restat of output example_autogen/timestamp older than most recent input C:/projects/scratchpad/autouic/src/widget2.cpp (6727772578205152 vs 6727773996182368)
ninja explain: C:/projects/scratchpad/autouic/build/cmake_issue_22531/example_autogen/mocs_compilation.cpp is dirty
ninja explain: example_autogen/timestamp is dirty
ninja explain: CMakeFiles/example_autogen is dirty
ninja explain: example_autogen/include/ui_mainwindow.h is dirty
ninja explain: example_autogen/include/ui_widget1.h is dirty
ninja explain: example_autogen/include/ui_widget2.h is dirty
ninja explain: example_autogen/timestamp is dirty
ninja explain: example_autogen/mocs_compilation.cpp is dirty
ninja explain: CMakeFiles/example.dir/example_autogen/mocs_compilation.cpp.obj is dirty
ninja explain: example_autogen/include/ui_mainwindow.h is dirty
ninja explain: CMakeFiles/example.dir/mainwindow.cpp.obj is dirty
ninja explain: example_autogen/include/ui_widget1.h is dirty
ninja explain: CMakeFiles/example.dir/widget1.cpp.obj is dirty
ninja explain: example_autogen/include/ui_widget2.h is dirty
ninja explain: CMakeFiles/example.dir/widget2.cpp.obj is dirty
ninja explain: example.exe is dirty
[6/6] Linking CXX executable example.exe
```
</details>


As best we can tell, the problem is related to the fact that the following build rule:

```
#############################################
# Custom command for example_autogen\timestamp

build example_autogen\timestamp example_autogen\mocs_compilation.cpp | ${cmake_ninja_workdir}example_autogen\timestamp ${cmake_ninja_workdir}example_autogen\mocs_compilation.cpp: CUSTOM_COMMAND C$:\Qt\5.15.2\msvc2019\bin\moc.exe C$:\Qt\5.15.2\msvc2019\bin\uic.exe || example_autogen_timestamp_deps
  COMMAND = cmd.exe /C "cd /D C:\projects\scratchpad\autouic\build\cmake_issue_22531 && "C:\Program Files\CMake\bin\cmake.exe" -E cmake_autogen C:/projects/scratchpad/autouic/build/cmake_issue_22531/CMakeFiles/example_autogen.dir/AutogenInfo.json Debug && "C:\Program Files\CMake\bin\cmake.exe" -E touch C:/projects/scratchpad/autouic/build/cmake_issue_22531/example_autogen/timestamp && "C:\Program Files\CMake\bin\cmake.exe" -E cmake_transform_depfile Ninja gccdepfile C:/projects/scratchpad/autouic/src C:/projects/scratchpad/autouic/src C:/projects/scratchpad/autouic/build/cmake_issue_22531 C:/projects/scratchpad/autouic/build/cmake_issue_22531 C:/projects/scratchpad/autouic/build/cmake_issue_22531/example_autogen/deps C:/projects/scratchpad/autouic/build/cmake_issue_22531/CMakeFiles/d/7404f3b5352b712ea9316479718c4fd6bb3f6464fae17b49ca4d2dda9248e200.d"
  DESC = Automatic MOC and UIC for target example
  depfile = C:/projects/scratchpad/autouic/build/cmake_issue_22531/CMakeFiles/d/7404f3b5352b712ea9316479718c4fd6bb3f6464fae17b49ca4d2dda9248e200.d
  restat = 1
```

Doesn't list the `ui_*.h` as direct outputs.

But instead creates a "phony" `CMakeFiles\example_autogen` target.

```
#############################################
# Phony custom command for CMakeFiles\example_autogen

build CMakeFiles\example_autogen example_autogen\include\ui_mainwindow.h example_autogen\include\ui_widget1.h example_autogen\include\ui_widget2.h | ${cmake_ninja_workdir}CMakeFiles\example_autogen ${cmake_ninja_workdir}example_autogen\include\ui_mainwindow.h ${cmake_ninja_workdir}example_autogen\include\ui_widget1.h ${cmake_ninja_workdir}example_autogen\include\ui_widget2.h: phony example_autogen\timestamp || example_autogen_timestamp_deps
```

Ninja doesn't appear to `restat` on the phony targets,
so we don't detect that the other `ui_*.h` files haven't changed
and should be removed from the list of pending build actions.


Listing these files as direct outputs of the `example_autogen\timestamp` target
should resolve the issue.

## Workaround

<details>
<summary>Steps to reproduce...</summary>


Build the sample project by doing the following:

```cmd
> git clone repo
> cmake -S improved --preset Debug
> cmake --build build\qt_wrap_methods --target all
```

Modify `widget2.ui` in some fashion:
```cmd
> cmake -E touch src\widget2.ui
```

And building:
```cmd
> cmake --build build\qt_wrap_methods --target all
[1/3] Generating ui_widget2.h
[2/3] Building CXX object CMakeFiles\example.dir\C_\projects\scratchpad\autouic\src\widget2.cpp.obj
[3/3] Linking CXX executable example.exe
```
_Rebuilds only ui_widget2.h and widget2.cpp.obj_


And similarly, modify `widget2.cpp` in some fashion
```cmd
> cmake -E touch src\widget2.cpp
[1/2] Building CXX object CMakeFiles\example.dir\C_\projects\scratchpad\autouic\src\widget2.cpp.obj
[2/2] Linking CXX executable example.exe
```
_Rebuilds only widget2.cpp.obj_

</details>


Disabling `AUTOUIC` and using the low level `qt_wrap_ui` works around both issues.


