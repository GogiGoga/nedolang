@echo off
path=..\_sdk\

echo ...compiling...
nedolang ../_sdk/emit.c ../_sdk/io.c commands.c
type err.f

echo ...tokenizing...
nedotok compcode.s ../_sdk/emit.asm ../_sdk/emit.var commands.asm commands.var ../_sdk/lib.i ../_sdk/iofast.i ../_sdk/io.asm ../_sdk/io.var ../_sdk/str.i

echo ...assembling...
nedoasm compcode.S_
type asmerr.f

nedodel compcode.asm
nedodel compcode.A_
nedodel emit.asm
nedodel emit.A_
nedodel emit.var
nedodel emit.V_
nedodel commands.asm
nedodel commands.A_
nedodel commands.var
nedodel commands.V_
nedodel lib.I_
nedodel io.I_
nedodel io.asm
nedodel io.A_
nedodel io.var
nedodel io.V_
nedodel err.f
nedodel asmerr.f
movedisk

pause

echo ...compiling...
nedolang ../_sdk/read.c compile.c
type err.f

echo ...tokenizing...
nedotok comp.s ../_sdk/read.asm ../_sdk/read.var compile.asm compile.var

echo ...assembling...
nedoasm comp.S_
type asmerr.f

diff nedolang comp.bin

pause
