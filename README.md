Project DIVA

Description coming soon.

Please download the tmax.nc file or mslp.2002.nc into the root directory to run this project. You can toggle between them by
commenting/uncommenting the block of code in Source/DynamicTextureSample/MyStaticMeshActor.cpp

# SET UP INSTRUCTIONS

## System Requirements:
1) Microsoft Visual Studio (2015 or 2017) with all C++ libraries
2) Git
3) Unreal v4.19
4) Python 3.6.4 64-bit
5) Windows SDK v8.1 (https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)

## Steps:
1) Clone Repository
2) Download latest netcdf4 library for c here: https://drive.google.com/drive/folders/1CQn-5Ej8iMF-Iit61bFJ_QtGMvo2s-2W
3) Copy entire folder into Thirdparty/netCDF
4) Download mslp.2002.nc (https://drive.google.com/open?id=1RqXCXicaGLiIxn3OmC15g8GA5G51wTa0) and move mslp.2002.nc to project root directory
5) Download and extract chesapeake and florida raw data (https://drive.google.com/drive/folders/1RXNpJHktfyJaZ5AatHntpm2fX_enN-jS)
6) Right-click Unreal Engine Project (DynamicTextureSample.uproject) --> Generate Visual Studio project files. If option is not showing up:
   - Navigate to C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64, copy the UnrealVersionSelector.exe file
   - Navigate to C:\Program Files\Epic Games\UE_4.19\Engine\Binaries\Win64, paste UnrealVersionSelector.exe
   - Double click on UnrealVersionSelector.exe, select Yes to register directory as Unreal Engine installation
7) Open DynamicTextureSample.sln with Visual Studio
8) Update paths
   - MyStaticMeshActor.cpp PROG_NAME and FILE_NAME
   - MyStaticMeshActor2.cpp FILE_NAME and FIRST_FILE_NAME
9) Start Debugger
10) Unreal Engine will start automatically

## Settings in Visual Studio:
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
