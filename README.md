# asteroids-emu

SDL2 based asteroids emulator

## Build Environment Prep

Install vs build tools:

```PowerShell
Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vs_BuildTools.exe' -OutFile "$env:TEMP\vs_BuildTools.exe"

& "$env:TEMP\vs_BuildTools.exe" --passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --remove Microsoft.VisualStudio.Component.VC.CMake.Project
```

Install cmake:

```PowerShell
scoop install cmake
```

Install sdl2:

```PowerShell
scoop install sdl2
```

Add [FindSDL2.cmake](https://github.com/aminosbh/sdl2-cmake-modules)

```PowerShell
cd asteroids-emu
git submodule add https://gitlab.com/aminosbh/sdl2-cmake-modules.git cmake/sdl2
git commit -m "Add SDL2 CMake modules"
```

## Generate Build Files

```PowerShell
cmake.exe -S . -B build  -Wno-dev
```

## Build

```PowerShell
cmake.exe --build build --config Debug --parallel
```

## Run

```PowerShell
build\RelWithDebInfo\asteroids-emu.exe
```

## Install

```PowerShell
cmake.exe --install build --config RelWithDebInfo
```

## MAME ROMS

asteroids.rom

Filename|Size|Region|Offset
--------|----|------|------
035145-04e.ef2|2048|maincpu|6800
035144-04e.h2|2048|maincpu|7000
035143-02.j2|2048|maincpu|7800
035127-02.np3|2048|maincpu|5000

034602-01.c8|256|dvg:prom|0

## References

Asteroids:

* [Hardware](https://computerarcheology.com/Arcade/Asteroids/Hardware.html)
* [RAM usage](https://computerarcheology.com/Arcade/Asteroids/RAMUse.html)
* [Disassembly (Nick Mikstas)](https://6502disassembly.com/va-asteroids/Asteroids.html)

Atari Digital Vector Generator (DVG):

* [DVG](https://computerarcheology.com/Arcade/Asteroids/DVG.html)
* [Hitch-Hacker's Guide to the Atari Digital Vector Generator](https://www.philpem.me.uk/elec/vecgen)

Books:

* [Using 6502 Assembly Language](https://archive.org/download/Using_6502_Assembly_Language/Using_6502_Assembly_Language.pdf)
  * This book has some Apple ][ specific information as well.
  * Microprocessor Instructions Reference: A-21 (p. 282)
  * Registers: A-22 (p. 283)
  * Instruction Codes: A-23 - A-27 (p. 284 - 288)
  * Hex Operation Codes: A-28 - A-29 (p. 289 - 290)
* [Obelisk 6502 Guide](https://www.nesdev.org/obelisk-6502-guide/)
  * [Instruction Reference](https://www.nesdev.org/obelisk-6502-guide/reference.html)

SDL:

* [SDL Wiki - API Reference](https://wiki.libsdl.org/SDL2/CategoryAPI)
* [Drawing Lines with SDL2](https://gigi.nullneuron.net/gigilabs/drawing-lines-with-sdl2)

