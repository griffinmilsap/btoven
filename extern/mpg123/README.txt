This is where you can put your local mpg123 external library if
you really don't want to install the library on your machine.

Honestly, you might want to consider directly installing the library
because you won't be able to run the resulting example executable
unless you have libmpg123.dll or libmpg123.so somewhere on your path...

That said, if you really want to, drop the files in here, build them
	* HINT: Check ports/MSVC++/ for Visual Studio
	* HINT: Check ports/Xcode/ for Xcode
	* HINT: Use your package manager for any Linux distribution
and cmake/modules/FindMPG123.cmake SHOULD find your build binaries.

Hopefully.

