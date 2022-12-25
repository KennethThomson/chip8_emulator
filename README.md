# CHIP-8 Emulator
An emulator for the CHIP-8 written in C++ using SDL2 to render and receive input in a multi-platform way.

## Installation
Install SDL2
```
$ sudo apt install libsdl2-dev
```

Clone this repository
```
$ git clone https://github.com/KennethThomson/chip8_emulator.git
$ cd chip8_emulator
```

Compile
```
$ make all
```

## ROMs
The file type for a CHIP-8 ROM is *.c8.

You can find tons of ROMs for the CHIP-8 online with a simple search but to those of you to lazy to search for it, here is a [link](https://github.com/dmatlack/chip8/tree/master/roms/games). 

## Execute the program
```
$ ./chip8 10 [delay value] [path to ROM]
```
I find that a delay value of 1 - 4 works well but feel free to adjust the values accordingly.

## Controls
```
Keypad       Keyboard
+-+-+-+-+    +-+-+-+-+
|1|2|3|C|    |1|2|3|4|
+-+-+-+-+    +-+-+-+-+
|4|5|6|D|    |Q|W|E|R|
+-+-+-+-+ => +-+-+-+-+
|7|8|9|E|    |A|S|D|F|
+-+-+-+-+    +-+-+-+-+
|A|0|B|F|    |Z|X|C|V|
+-+-+-+-+    +-+-+-+-+
```

## References
* [Austin Morlan's Guide to Building a CHIP-8 Emulator](https://austinmorlan.com/posts/chip8_emulator/)
* [CHIP-8 - Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
* [Cowgod's Chip-8 Technical Reference v1.0](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
* [Simple DirectMedia Layer (SDL2) Wiki](https://wiki.libsdl.org/CategoryAPI)
