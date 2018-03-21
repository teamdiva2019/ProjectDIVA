Project DIVA

Description coming soon.

Please download the tmax.nc file or mslp.2002.nc into the root directory to run this project. You can toggle between them by
commenting/uncommenting the block of code in Source/DynamicTextureSample/MyStaticMeshActor.cpp

# SET UP INSTRUCTIONS

System Requirements:
1) Microsoft Visual Studio 2015 installed with all C++ libraries
2) Set Up git
3) Unreal v4.14 installed
4) Python 3.6.4 64-bit installed

Steps:
1) Clone Repository
2) Download latest netcdf4 library for c here: https://www.unidata.ucar.edu/software/netcdf/docs/winbin.html (without DAP)
3) Copy entire folder into Thirdparty/netCDF
4) Download mslp.2002.nc https://drive.google.com/drive/folders/0B_Folj4rOnAVYmt5OTNoX1FSMjA
5) Move mslp.2002.nc to project root directory
6) Right-click Unreal Engine Project (DynamicTextureSample.uproject) --> Generate Visual Studio project files
7) Open Microsoft Visual Studio Solution (DynamicTextureSample.sln)
8) Change FILE_NAME to appropriate file path in MyStaticMeshActor.cpp
9) Start Debugger
10) Unreal Engine will start automatically


Settings in Visual Studio:
* VC++ Include and library directories contain /include and /libs for Python3.6
* Mode: Development Editor
* Make sure our project is set as a start up project (right click -> set startup project)
* Build order: UE4 then our project, our project depends on UE4

# COMMON ERRORS

## Can't include precompiled file Python.h
* Delete the line `#include <Python.h>`
* Rebuild and let it break
* Put the line back
* Build again, et voila
