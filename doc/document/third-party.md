# third party libraries

You are not required to install these 3rd party libraries for Helium to run,
but Helium will of course fail to compile the projects using a missing library call or type.

You can add third party library header file and optional compile flag in `headers.conf`.
Helium will check if the headers exist, and if so include it for every building.

To install the default libraries, refer to `headers.conf`.
