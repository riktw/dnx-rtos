A simple LED blinky and hello world example to use with the elf loader.
Compile via "make" and copy the app-striped.elf to a sd card.
On a dnx-rtos system with the elf loader app, the .elf can be executed with "run app-striped.elf"
As the symbols table is created after compiling dnx-rtos and then added to romfs, dnx-rtos needs to be compiled twice. Once to compile and create the symbols table, once to add it to romfs.
