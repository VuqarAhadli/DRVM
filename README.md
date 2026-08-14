<img align="right" src="extracted/splash" width="200" alt="Diamond Rush Splash">

# Diamond Rush Emulator

A custom Java Virtual Machine (JVM) implementation for reverse engineering the iconic J2ME game **Diamond Rush**.

## Future Plans

Once the VM is fully functional on Linux and macOS (hopefully), I plan to port it and its graphics backend to a variety of popular microcontroller platforms, such as the ESP32 and RP2040/RP2350.

# To build the project:
``` bash
mkdir build
cd build
cmake ..
make
```
> Note: The project does not use any external libraries yet.

## Constant pool dump
<br>
<p align="center">
  <img src="drvm_photos/Constantpool.png" width="500" alt="Constant Pool">
</p>

## Methods dump
<p align="center">
  <img src="drvm_photos/Methods.png" width="500" alt="Methods">
</p>
