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

Despite not using the Arduino software libs, the AVR chip programming depends on Arduino hardware. The predefined programmer settings makes use of the DIP-slot version of Arduino Uno R3 where you can swap or remove the ATmega328P chip. So instead of using a dedicated Atmel programmer to program the chip (and potentially debug the live program, which is a nice-to-have feature...) I chose to use the method described on the Arduino site:

[From Arduino to a Microcontroller on a Breadboard](https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard/)

Note that no μC is fitted to the board DIP slot. The standalone chip is wired to UART TX,RX and RESET pins on the Arduino board with common ground. The detailed I/O wiring diagram (WS281x LED output, TWI/I2C sensor output,..) is WIP.

TODO: Arduino flash method in absense of Arduino IDE works?

## Implementation planning

The microcontroller gathers TWI/I2C sensor data that transforms and displays as an animation on Neopixel LEDs. Handling the animation drawing and WS ledstring "modulation" needs a better approach then a direct "pixel writing", like creating a pixel buffer, that stores the transformed data. Provided that an ISR will transfer the buffered data to Neopixels' input and the input sensor + drawing algorithm needs moderate CPU processing time, there is plenty of CPU time left for other task processing.

However, different peripherals and external devices has different signal timing requirements and thresholds. Doing robust, fault-tolerant timing and minimizing administrative overhead while squeezing out every CPU cycle is hard, but this is a pet project, so I planned on the next three essential layers of I/O subsystem:

* I/O drivers: Simple or state machine-backed interface to μC peripherals

* protocol implementers: Uses various I/O drivers as backend to communicate with other devices, optionally selected by the client. Essentially a frontend

* facade functions: hides certain complexity of inner layers, but this is a microcontroller environment, so usage is limited

The majority of state machine controlled I/O functions uses a (priority based?) queue that stores future operations. An operation consists of a so-called iterator function, which generates and puts output data to a separate buffer queue. The iterator has direct access to the buffer and returns an opcode that signals how urgent and how many times the function wants to be called in the future.

Challenges:

* Data congestion and CPU bottleneck: receiving and sending data to or from other devices simultaneously

* Error handling between multiple layers (e.g. disappearing LCD display controlled via TWI/I2C)

* Minimal, but enough latch delays when interfacing external peripherals

* Minimize administrative, boilerplate and duplicate code

* Balance between CPU and memory usage with caching, scratchpad

## Sources

* https://daniellethurow.com/blog/2021/6/8/programming-an-atmega328p-without-the-arduino-ide

* https://www.tonymitchell.ca/posts/building-avr-projects-with-make/

* Atmel AppNotes

* https://leanpub.com/patternsinc

* C/<pattern> chapters: https://www.cs.yale.edu/homes/aspnes/pinewiki/TitleIndex.html#idx-C
