# Lötyi virtual bottle

Powered by ATmega328P, this WIP project aims to represent a virtual bottle with some LED string animation.

The codebase is developed without any Arduino-related libs, so smaller footprint can be achieved and more control over μC resources is possible.

## Compiling

The code is developed in VS Code and is ready to compile on Linux platforms after installing the neccessary compiler toolchain, then setting the proper Debug target task.

The `Makefile` contains all logic to compile and flash the firmware onto the ATmega, the correct settings (fCLK, TTY & flasher BAUD rate,..) are need to be set here. The flashing sequence (with compiling if required) can be initiated from VS Code by activating Debug mode (F5). The `Build/Flash` combined task can be set as default from 'Run and Debug' (`Ctrl+Shift+D`).

On Debian (& some derivatives), the AVR toolchain is contained in these packages:

* gcc-avr (compiler)

* avr-libc (headers & other useful stuff)

## Physical wiring and flashing

Despite the absense of Arduino software lib usage, the solution used for programming the AVR chip depends on Arduino hardware. The predefined programmer settings makes use of the DIP-slot version of Arduino Uno R3 where you can swap or remove the ATmega328P chip. So instead of using a dedicated Atmel programmer to program the chip (and potentially debug the live program, which is a nice-to-have feature...) I chose to use the method described on the Arduino site:

[From Arduino to a Microcontroller on a Breadboard](https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard/)

The standalone μC is connects to the Arduino board on UART TX,RX and RESET pins just like on the link above, with common ground connected between the microcontroller and Arduino board. The detailed I/O wiring diagram is WIP.

TODO: Arduino flash method in absense of Arduino IDE works?

## Links

* https://daniellethurow.com/blog/2021/6/8/programming-an-atmega328p-without-the-arduino-ide

* https://www.tonymitchell.ca/posts/building-avr-projects-with-make/
