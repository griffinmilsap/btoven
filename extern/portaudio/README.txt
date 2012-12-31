This is where you can put your local portaudio external library if
you really don't want to install the library on your machine.

Honestly, you might want to consider directly installing the library
because you won't be able to run the resulting example executable
unless you have portaudio_x86.dll or libportaudio.so somewhere on your path...

That said, if you really want to, drop the files in here, build them
	* HINT: Come here in a command prompt/terminal and type, "cmake ."
		and use your generated project files/makefiles to build portaudio
and cmake/modules/FindPortaudio.cmake SHOULD find your build binaries.

Hopefully.

