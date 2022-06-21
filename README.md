C64++ - C64 Emulator written in C++20
=====================================

This is a C64 Emulator written in C++ using Qt6 as part of a practical course at the Karlsruher Institut für Technologie.

Credits:
- Marcel Riedel
- Moritz Behr
- Patrick Jaberg

Compiling in Windows with MinGW
-------------------------------

You need:
- Qt 6 open source
  - Qt -> Qt 6.3.0 -> MinGW 11.2.0 64-bit
  - Qt -> Qt 6.3.0 -> Additional Libraries -> Qt Multimedia
  - QtCreator (should be preselected)

1. Start QtCreator
2. File -> Open -> CMakeLists.txt in this folder
3. Check Desktop Qt 6.3.0 MinGW 64-bit
3. Compile and run with green Triangle (lower left)

The `deploy_windows.ps1` script can be used to deploy to a portable executable or .exe Installer.
For the installer, ![NSIS](https://sourceforge.net/projects/nsis/) is required.

Compiling in Linux
------------------
Just build with cmake:

```bash
mkdir build
cd build
cmake ..
make
```
