Spirit
========
**Modular Numerical Optimizations Spin Code**<br />
The code is released under [MIT License](../master/LICENSE.txt).<br />
If you intend to *present and/or publish* scientific results for which you used Spirit,
please read the [REFERENCE.md](../master/REFERENCE.md)

For contributions and affiliations, see [CONTRIBUTIONS.md](../master/CONTRIBUTIONS.md)

Wiki Page: https://iffwiki.fz-juelich.de/index.php/Spirit

Please note that the Spirit Web interface is hosted at the Research Centre Jülich:
http://iffwww.iff.kfa-juelich.de/pub/spiritdemo/

<!--
![nur ein Beispiel](https://commons.wikimedia.org/wiki/File:Example_de.jpg "Beispielbild")
-->

Contents
--------
1. [Introduction](#Introduction)
2. [User Interfaces](#UserInterfaces)
3. [Branches](#Branches)
4. [Code Dependencies](#Dependencies)
5. [Installation Instructions](#Installation)
5. [Contributing](#Contributing)


&nbsp;
 

Introduction <a name="Introduction"></a>
----------------------------------------

**Platform-independent** code with optional visualization, written in C++11.
The build process is platform-independent as well, using CMake. 
This code has been developed as a flexible solution to various use-cases, including:
* **Spin Dynamics simulations** obeying the
  [Landau-Lifschitz-Gilbert equation](https://en.wikipedia.org/wiki/Landau%E2%80%93Lifshitz%E2%80%93Gilbert_equation "Titel, der beim Überfahren mit der Maus angezeigt wird")
* Direct **Energy minimisation** of a spin system
* **Minimum Energy Path calculations** for transitions between different
  spin configurations, using the GNEB method
* Energy Landscape **Saddlepoint searches** using the MMF method

More details may be found in the [Wiki](https://iffwiki.fz-juelich.de/index.php/Spirit "Click me...").

----------------------------------------

&nbsp;
   

User Interfaces <a name="UserInterfaces"></a>
---------------------------------------------
The code is separated into several folders, representing the 'core' physics code
and the various user interfaces you may build.
* **core**:        Core Physics code
* **ui-console**:  Console version
* **ui-qt**:       OpenGL visualisation inside QT
* **ui-web**:      WebGL visualisation inside a website
* **gl**:          Reusable OpenGL code
* **thirdparty**:  Third party libraries the code uses

Due to this modularisation, arbitrary user interfaces may be placed on top of the core physics code.
The console version may be useful to be run on clusters, whereas the GUI-versions are useful to
provide live visualisations of the simulations to the user.
Note that the current web interface is primarily meant for educational purposes. Since the core needs to be
transpiled to JavaScript (which does not support threads) and is executed user-side, it is necessarily slower
than the regular code.

----------------------------------------

&nbsp;


Branches <a name="Branches"></a>
--------------------------------
We aim to adhere to the "git flow" branching model: http://nvie.com/posts/a-successful-git-branching-model/

>Release (`master` branch) versions are tagged `x.x.x`, starting at `1.0.0`

Download the latest stable version from https://github.com/spirit-code/spirit/releases

The develop branch contains the latest updates, but is generally less consistently tested than the releases.

----------------------------------------

&nbsp;


Code Dependencies <a name="Dependencies"></a>
---------------------------------------------

The core library does not have dependencies, except for C++11.
Due to the modular CMake Project structure, when building only a specific library or UI,
one does thus not need any libraries on which other projects may depend.
Most *external* dependencies are included in the thirdparty folder. 

The following lists all *external* dependencies which are not included: 

### Core
* gcc >= 4.8.1 (C++11 stdlib)
* cmake >= 2.8.12

### GL
* OpenGL Drivers >= 3.3

Necessary OpenGL drivers *should* be available through the regular drivers for any remotely modern graphics card.

### UI-QT
* QT >= 5.5

In order to build with QT as a dependency, you need to have `path/to/qt/qtbase/bin` in your PATH variable.

Note that building QT can be a big pain, but usually it should work if you simply use their installers.

### UI-Python
* Python

We have not tested how far backwards the Python UI is compatible.
It should not matter if you use Python 2 or 3.

### UI-Web
* emscripten

In order to build the core.js JavaScript library, you need emscripten.
Note we have not tested this process on different machines.

----------------------------------------

&nbsp;



Installation Instructions <a name="Installation"></a>
-----------------------------------------------------

>The following assumes you are in the Spirit root directory.

Please be aware that our CMake scripts are written for our use cases and
you may need to adapt some paths and options in the Root CMakeLists.txt, specifically:

The **Options** you can set under *### Build Flags ###* are:
* BUILD_UI_WEB - build the web interface instead of others
* BUILD_UI_PYTHON - build the python library
* BUILD_UI_CXX - build the C++ interfaces (console or QT) instead of others
* UI_CXX_USE_QT - build qt user interface instead of console version
* OSX_BUNDLE_APP - not yet functional
* PRINT_SOURCES - print all source files (for debugging)
* USER_PATHS_IFF - use default IFF (FZJ) cluster paths

The **Paths** you can set under *### User Paths ###* (just uncomment the corresponding line) are:
* USER_COMPILER_C and USER_COMPILER_CXX for the compiler name you wish to use
* USER_PATH_COMPILER for the directory your compiler is located in
* USER_PATH_QT for the path to your CMake installation
Otherwise, the developers' defaults will be used or CMake will try to use it's defaults.
  
Clear the build directory using

	./clean.sh
	or
	rm -rf build && mkdir build
	
### Generate Build Files
`./cmake.sh` lets cmake generate makefiles for your system inside a 'build' folder.
Simply call

	./cmake.sh
	or
	cd build && cmake .. && cd ..
	
When on pure **Windows** (no MSys etc), you can simply use the git bash to do this.
When using MSys etc., CMake will create corresponding MSys makefiles.

### Building the Projects
`./make.sh` executes the build and linking of the executable. Simply call

	./make.sh
	or
	cd build && make && cd ..

If building any C++ UI, the executable `spirit` should now be in the root folder

When on pure **Windows** (no MSys etc), instead of using `make` or `./make.sh`,
you need to open the generated Solution in Visual Studio and build it there.
The execution folder should be 'build' and file paths at runtime will be
relative to this folder.

----------------------------------------

&nbsp;
   

Contributing <a name="Contributing"></a>
-----------------------------------------

1. Fork this repository
2. Check out the develop branch: `git checkout develop`
3. Create your feature branch: `git checkout -b feature-something`
4. Commit your changes: `git commit -am 'Add some feature'`
5. Push to the branch: `git push origin feature-something`
6. Submit a pull request

Please keep your pull requests feature-specific and limit yourself
to one feature per feature branch.
Remember to pull updates from this repository before opening a new
feature branch.

If you are unsure where to add you feature into the code, please
do not hesitate to contact us.

There is no strict coding guideline, but please try to match your
code style to the code you edited or to the style in the respective
module.
