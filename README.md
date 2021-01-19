Corewar2
========

Contents
--------

* [Introduction](#introduction)
* [Corewar](#corewar)
* [Language](#language)
* [Rules](#rules)
* [Examples](#examples)

Introduction
------------

Corewar2 is a very simple Corewar clone, written in C++ with [Qt](http://qt.nokia.com/). The language used was created by myself for this purpose and named CWA (for CoreWar Assembly). It's a simple assembly language, with 7 instructions.

Corewar
-------

The idea of Corewar is simple: programs fight in a virtual memory arena, the last program running wins. You can make other programs crash by making it execute an invalid instruction (by overwriting its code).

Language
--------

The programs are CWA files, which are compiled and placed in memory at the beginning of the round. To keep things simple, the memory is a list of "cells", any of which capable of storing **data** (a number) or **an instruction**. Instructions execute one at a time, one per program per turn. Memory is cyclic, going over one end transparently comes out the other.

CWA files contain one instruction per line, with comments after a semicolon ';'. Each instruction is either like `DAT <val1>`, or `<INSTR> &<val1> &<val2>` where `val1` and `val2` are numbers and `&` is one of the characters `#` (hash sign), `$` (dollar sign) ou `@` (at sign) which correspond to the different addressing modes. There are no registers, all data is in memory.

Arguments can be of three sorts. They are written with a prefix character then a number (positive or negative). The prefixes mean:

* `$`: immediate value. The value to use is the number written. Example: `$-12`
* `#`: address. The value to use is the one found at the address given, relative to the current instruction's position. So `#0` means the current instruction, `#-1` means the previous one, etc.
* `@`: indirect address. The value to use is at the (indirect) address found at the given address. Note that the indirect address is also relative to the current instruction, not its own location. For example, `@1 ; DAT 3 ; DAT -3 ; DAT 4` references the last cell which contains `DAT 4`.

Instructions can take one of two arguments. Not all types are necessarily accepted, for example, `JMP` needs an address (no immediate value), same thing with `MOV`, `ADD`, and `SUB`. The instructions are:

* `DAT <number>`: A numeric value. If a program reaches a cell containing a `DAT`, it gets terminated. Note that the `number` cannot have a prefix.
* `ADD <src> <dst>`: Add `src` to `dst`. `dst` cannot be an immediate value..
* `SUB <src> <dst>`: Subtract `src` from `dst`. `dst` cannot be an immediate value.
* `MOV <src> <dst>`: Copy from `src` to `dst`. This instruction can copy both data and instructions. `dst` cannot be an immediate value.
* `IFE <a> <b>`: Compare `a` with `b`. If `a` is equal to `b`, the next instruction is executed, else it is skipped.
* `IFL <src> <dst>`: Compare `a` with `b`. If `a < b`, the next instruction is executed, else it is skipped.
* `JMP <dst>`: Jump to the address `dst`. `dst` cannot be an immediate value.
* `FORK <dst>`: Start a thread, which executes at the address `dst`, while we continue with the next instruction.

Rules
-----

A program is terminated if it executes an invalid instruction (for example a `DAT`, or an instruction with an invalid argument type). Furthermore, a memory cell is considered to belong to the last program which wrote to it. If a program executes a memory cell that doesn't belong to it, it is *considered to be* the program that wrote it. This means if all remaining programs are executing code from your program, you win.

If the `FORK` instruction is enabled, it allows a program to create multiple threads. However beware that threads are sharing execution time. This means that a program with 3 threads will see each of its thread execute at one third of the speed a of program which never uses the `FORK` instruction.

Examples
--------

### Steamroller

The steamroller is the simplest possible program. It only contains one instruction, which copies itself at the next memory cell. It doesn't crash other programs, once overwritten by the steamroller, they simply start executing the single instruction themselves (which does not cause the steamroller to move faster). Written in CWD, this instruction reads:

```
mov #0 #1 ; steamroller
```

### Bomber

This example, a little more complex, loops to write `DAT 0` over the whole memory following it.

```
add $1 #3 ; increase the counter
mov $0 @2 ; write a DAT 0 to the pointed address
jmp #-2   ; back to start
dat 2     ; counter
```

### Anti-steamroller

This program only counters steamrollers, it doesn't attack. It places a `DAT 1` a little way back and checks for it regularly. Once it is overwritten, it kills the steamroller that we assume overwrote it, by writing a `DAT 0`:

```
mov $1 #-4 ; place the DAT 1
ife $1 #-5 ; checks that it's there
jmp #-1    ; it's still there, loop
mov $0 #-5 ; it was overwritten, write two DAT 0 to kill a steamroller
mov $0 #-6
jmp #-5    ; then loop
```

### Spammer

For this example, the `FORK` instruction needs to be enabled. This program sends steamrollers to many memory locations. It may overwrite itself with a steamroller.

```
add $128 #5 ; increase counters
add $128 #5
mov #5 @3   ; copy the steamroller
fork @3     ; start the steamroller
jmp #-4     ; loop
dat 4
dat 3
mov #0 #1   ; instruction for the steamroller
```
