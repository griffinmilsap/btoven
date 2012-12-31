btoven
======

Realtime Beat Detection Library

Compiling and Installation
--------------------------

This project is split into two separate targets;

- __btoven__ is a static library that performs an analysis on streaming pcm data.
		By itself, it has no dependencies and should compile without issue.
- __btoven_c_example__ is an example project that shows the analysis in action.
		This example project depends on [mpg123](http://www.mpg123.de) and [Portaudio](http://www.portaudio.com).  CMake will try to find these dependencies on your path and use them when creating build files for your compiler.  If you do not have them installed and neatly on your path (likely, if you're running Windows) you'll need to compile these projects separately and put binaries on your system path in order to run this example.  More details can be found in extern/*

The project relies on [CMake](http://www.cmake.org) to generate build files for your platform.  Ensure you have CMake downloaded and installed before attempting to compile.

In order to compile the btoven project, open the root directory in a terminal/command prompt and type:

	cmake .

Use the generated build files (eg. Visual Studio Project) or makefiles to build the targets.  If CMake cannot find the dependencies required for _btoven_c_example_, it will not generate a build target for the example executable.  Build all targets and build the INSTALL target with administrator priviliges.  On Linux:

	make
	sudo make install

At this point, the binaries should be on your system path and you can use btoven in any of your projects.  If you're on windows, you can find your installed library in Program Files (perhaps Program Files (x86)), and you should add this install directory (/lib and /include) to your system path.  Or just forgo installation and use the binaries directly.

Oh, and there's an Android Project.  I might get around to writing documentation for that sometime in the near future.  There's some cool stuff going on in there with OpenSL and network streaming of audio files.
