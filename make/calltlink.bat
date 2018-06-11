@echo off

setlocal enabledelayedexpansion

set PATH=%~1
cd %~2

set objs=
set libs=

set /a count=0
for %%a in (*.obj) do (
	ren "%%a" !count!.obj 2> nul
	set objs=!objs! !count!
	set /a count+=1
)
set /a count=0
for %%a in (*.lib) do (
	ren "%%a" !count!.lib 2> nul
	set libs=!libs! !count!
	set /a count+=1
)

call tlink %~3 %objs:~1%,%~4,,%libs:~1%