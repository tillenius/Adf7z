@echo off

cl /O2 /c 7z/C/Alloc.c
cl /O2 /c 7z/CPP/Common/Wildcard.cpp
cl /O2 /c 7z/CPP/Common/MyString.cpp
cl /O2 /c 7z/CPP/Common/IntToString.cpp
cl /O2 /c 7z/CPP/Common/StringConvert.cpp
cl /O2 /c 7z/CPP/Windows/PropVariant.cpp
cl /O2 /c 7z/CPP/7zip/Common/CWrappers.cpp
cl /O2 /c 7z/CPP/7zip/Common/ProgressUtils.cpp
cl /O2 /c 7z/CPP/7zip/Common/StreamUtils.cpp
cl /O2 /c 7z/CPP/7zip/Common/PropId.cpp
cl /O2 /c 7z/CPP/7zip/Archive/ArchiveExports.cpp /EHsc
cl /O2 /c 7z/CPP/7zip/Archive/DllExports.cpp

cl /LD /O2 /EHsc /DEF 7z/CPP/7zip/Archive/Archive.def ADFHandler.cpp Alloc.obj MyString.obj Wildcard.obj IntToString.obj StringConvert.obj PropVariant.obj CWrappers.obj ProgressUtils.obj StreamUtils.obj PropId.obj ArchiveExports.obj DllExports.obj user32.lib oleaut32.lib
