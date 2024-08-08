# Lötyi virtual bottle

Powered by ATmega328P, this WIP project aims to represent a virtual bottle with some LED string animation.

The codebase is developed without any Arduino-related libs, so smaller footprint can be achieved and more control over μC resources is possible.

## Compiling

The code is developed in VS Code and is ready to compile on Linux platforms after installing the neccessary compiler toolkit, then setting the proper Debug target task.

The `Makefile` contains all logic to compile and flash the firmware onto the ATmega, the correct settings (fCLK, TTY & flasher BAUD rate,..) are need to be set here. The flashing sequence (with compiling if required) can be initiated from VS Code by activating Debug mode (F5). The `Build/Flash` combined task can be set as default from 'Run and Debug' (`Ctrl+Shift+D`).

On Debian (& some derivatives), the AVR toolkit is contained in these packages:

* gcc-avr (compiler)

* avr-libc (headers & other useful stuff)

## Links

* https://daniellethurow.com/blog/2021/6/8/programming-an-atmega328p-without-the-arduino-ide
