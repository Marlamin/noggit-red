# LICENSE #
This software is open source software licensed under GPL3, as found in
the COPYING file.

# BUILDING #
This project requires CMake to be built. 

It also requires the
following libraries:

* OpenGL
* StormLib (by Ladislav Zezula)
* CascLib (by Ladislav Zezula)
* Qt5
* Lua5.x

On Windows you only need to install Qt5 yourself, the rest of the dependencies are pulled through FetchContent automatically.
Supporting for Linux and Mac for this feature is coming in the future.
In case FetchContent is not available (e.g. no internet connection), the find scripts will look for system installed libraries.

Further following libraries are required for MySQL GUID Storage builds:

* LibMySQL
* MySQLCPPConn
See below for detailed instructions

## Windows ##
Text in `<brackets>` below are up to your choice but shall be replaced
with the same choice every time the same text is contained.

### MSVC++ ###
Any recent version of Microsoft Visual C++ should work. Be sure to
remember which version you chose as later on you will have to pick
corresponding versions for other dependencies.

### CMake ###
Any recent CMake 3.x version should work. Just take the latest.

### Qt5 ###
Install Qt5 to `<Qt-install>`, downloading a pre-built package from
https://www.qt.io/download-open-source/#section-2.

Note that during installation you only need **one** version of Qt and
also only **one** compiler version. If download size is noticably large
(more than a few hundred MB), you're probably downloading way too much.

### StormLib ###
This step is only required if pulling the dependency from FetchContent is not available.
Download StormLib from https://github.com/ladislav-zezula/StormLib (any
recent version).

* open CMake GUI
* set `CMAKE_INSTALL_PREFIX` (path) to `<Stormlib-install>` (folder should
  not yet exist). No other things should need to be configured.
* open solution with visual studio
* build ALL_BUILD
* build INSTALL
* Repeat for both release and debug.

### MySQL (Optional) ###
Optional, required for MySQL GUID Storage builds.
download MySQL server https://dev.mysql.com/downloads/installer/
and MySQL C++ Connector https://dev.mysql.com/downloads/connector/cpp/
* open CMake GUI
* enable `USE_SQL`
* set `MYSQL_LIBRARY` (path) to `libmysql.lib` from your MYSQL server install.
e.g `"C:/Program Files/MySQL/MySQL Server 8.0/lib/libmysql.lib"`
* set `MYSQLCPPCONN_INCLUDE` (path) to the folder containing `cppconn/driver.h` from your MYSQL Connector C++ install.
e.g `"C:/Program Files/MySQL/Connector C++ 8.0/include/jdbc"`
* set `MYSQLCPPCONN_LIBRARY` (path) to `mysqlcppconn.lib` from your MYSQL Connector C++ install.
e.g `"C:/Program Files/MySQL/Connector C++ 8.0/lib64/vs14/mysqlcppconn.lib"`
* Don't forget to set your SQL settings and enable the feature in the noggit settings menu to use it.

### Noggit ###
* open CMake GUI
* set `CMAKE_PREFIX_PATH` (path) to `"<Qt-install>;<Stormlib-install>"`,
  e.g. `"C:/Qt/5.6/msvc2015;D:/StormLib/install"`
* set `BOOST_ROOT` (path) to `<boost-install>`, e.g. `"C:/local/boost_1_60_0"`
* (**unlikely to be required:**) move the libraries of Boost from where
  they are into `BOOST_ROOT/lib` so that CMake finds them automatically or
  set `BOOST_LIBRARYDIR` to where your lib are (.dll and .lib). Again, this
  is **highly** unlikely to be required.
* set `CMAKE_INSTALL_PREFIX` (path) to an empty destination, e.g. 
  `"C:/Users/blurb/Documents/noggitinstall`
* configure, generate
* open solution with visual studio
* build ALL_BUILD
* build INSTALL
 
To launch noggit you will need the following DLLs from Qt loadable. Install
them in the system, or copy them from `C:/Qt/X.X/msvcXXXX/bin` into the
directory containing noggit.exe, i.e. `CMAKE_INSTALL_PREFIX` configured.

* release: Qt5Core, Qt5OpenGL, Qt5Widgets, Qt5Gui
* debug: Qt5Cored, Qt5OpenGLd, Qt5Widgetsd, Qt5Guid 

## Linux ##
On **Ubuntu** you can install the building requirements using:

```bash
sudo apt install freeglut3-dev libboost-all-dev qt5-default libstorm-dev
```

Compile and build using:

```bash
mkdir build
cd build
cmake ..
make -j $(nproc)
```

Instead of `make -j $(nproc)` you may want to pick a bigger number than
`$(nproc)`, e.g. the number of `CPU cores * 1.5`.

If the build pass correctly without errors, you can go into build/bin/
and run noggit. Note that `make install` will probably work but is not
tested, and nobody has built distributable packages in years.

# SUBMODULES #

To pull the latest version of submodules use the following command at the root directory.

```bash
git submodule update --recursive --remote
```

# CODING GUIDELINES #
File naming rules:

```.hpp``` - is used for header files (C++ language).

```.h``` - is used **only** for header files or modules written in C language.

```.c``` - is used **only** for implementation files or modules written in C language.

```.cpp``` - is used for project implementation files. 

```.inl``` - is used for include files providing template instantiations.

```.ui``` - is used for QT UI definitions (output of QtDesigner/QtCreator).

### Project structure: ###

```/src/Noggit``` - is the main directory hosting .cpp, .hpp, .inl, .ui files of the project.

Within this directory the subdirs should correspond to namespace names (case sensitive). 

File names should use PascalCase (e.g. ```FooBan.hpp```) and either correspond to the type defined in the file,
or represent sematics of the module.

```/src/External``` - is the directory of hosting included libraries and subprojects. This is external or modified
external code, so no rules from Noggit project apply to its content.

```/src/Glsl``` - is the directory to store .glsl shaders for the OpenGL renderer. It is not recommended, 
but not strictly prohibited to inline shader code as strings to ```.cpp``` implementation files.


### Code style ###

Following is an example for file `src/Noggit/Ui/FooBan.hpp`. 

```cpp
#ifndef INCLUDE_GUARD_BASED_ON_FILENAME
#define INCLUDE_GUARD_BASED_ON_FILENAME
// We do not use #pragma once in headers as it is technically not cross-platform.
// Use include guards instead. For example, CLion IDE creates them automatically on .hpp file creation.

// <> are prefered for includes.
// Local imports go here
#include <SomeLocalFile.hpp>

// Lib imports go here
#include <external/SomeLibCode.hpp

// STL imports go here
#include <string>
#include <mutex>
#include <vector> // etc

// Forward declarations in headers are encouraged. That prevents type leaking into bigger scopes
// Also reduces compile time
namespace Parent::SomeOtherChild
{
  class ForwardDeclaredClass;
}

// Namespaces are defined as PascalCase names. Namespace concatenation for nested namespaces
// is adviced, but not strictly enforced.
namespace Parent::Child
{
  // types are name in PascalCase,
  class Test : public TestBase
  {
    public:
      Test();
     
      int x; // public fields like that are discourged, but occur here and there through the project. 
      // Subject to refactoring.
      
      // methods are named in camelCase.
      // trivial getter methods are declared in the header file.
      int somePrivateMember() { return _some_private_member; } const;

      // trivial setters are declared in the header file. Preceded by "set" prefix.
      void setSomePrivateMember(int a) { _some_private_member = a; };
    
    // private members are snake lower case, separated by underscore, preceded by underscore to indicate they're private.
    private:
      int _some_private_member;
      ForwardDeclaredClass* _some_other_private_member_using_forward_decl;
      std::mutex _mutex;

    // static methods

    private:
      static void someStaticMethod();
    
  };
}

#endif
```

Following is an example for file `src/Noggit/Ui/FooBan.cpp`.

```cpp
// the header of this .cpp comes first
// <> are prefered for includes.
#include <Noggit/Ui/FooBan.hpp>

// same order of includes as in header.

using namespace Parent::Child;

Test::Test()
: TestBase("some_arg")
, _some_private_member(0)
, _some_other_private_member_using_forward_decl(new ForwardDeclaredClass()) // do not forget to import ForwardDeclaredClass in .cpp
{
// body of ctor
}

void Test::someStaticMethod()
{
// local variables are named in snake_case, no preceding underscore.
int local_var = 0;

// preceding underscore is used on variables that are used for RAII patterns, such as scoped stuff (e.g. a scoped mutex)
std::lock_guard<std::mutex> _lock (_mutex); // _lock is never accessed later, it just needs to live as long as the scope lives.
// So, it has an underscore prefix.

someFunc(local_var); // free floating functions use the same naming rules as methods
}
```

Additional examples:

```cpp

constexpr unsigned SOME_CONSTANT = 10; // constants are named in SCREAMING_CASE
#define SOME_MACRO // macro definitions are named in SCREAMING_CASE

```
