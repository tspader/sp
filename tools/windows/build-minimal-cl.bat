@echo off
setlocal

if not exist "build" mkdir "build"
if not exist "build\debug" mkdir "build\debug"
if not exist "build\debug\store" mkdir "build\debug\store"
if not exist "build\debug\store\bin" mkdir "build\debug\store\bin"
if not exist "build\debug\store\obj" mkdir "build\debug\store\obj"

cl /nologo /TC /Zi /Od /W4 /we4715 /I. /Iinclude /Itest\tools /Itest\tools\process /Isp /DSP_IMPLEMENTATION /DSP_TEST_IMPLEMENTATION /Febuild\debug\store\bin\minimal.exe /Fobuild\debug\store\obj\ test\amalg.c
