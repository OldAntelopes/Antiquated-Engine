Antiquated Engine
--------------
Old windows C/C++ game framework and legacy DX9 graphics engine

The code contained here was generated over many years as part of various hobby projects largely centered around the game 'The Universal' (or 'A tractor' if you prefer), which was/is a never-quite-good-enough attempt at making a distributed MMO in the early 2000s.  

It is not well designed, well implemented, well structured or well.. anything .. it just is what it is. Sorry, Adrian. Considering that most of it was written in a rush very late at night, often after a few whiskeys, its always surprising any of it works at all. (Which it probably doesnt)

This github project has mostly been created for the sake of posterity rather than any expectation of practical value, although the code is still in active use in the old game. 

Overall structure of the repo
-----------------------------
The repo consists of two main blocks of code that reflect different coding periods:
1) **Pub** : In this folder are the core 2D/3D DX-wrapper libraries that are used in 'The Universal'. These evolved intermittently from 2000 onwards as the game expanded (and contracted then slowly drifted off, occasionally flickering briefly into life for a while whenever i'd stored up enough cash and will to quit my job). Each library has a primary header file and generates a .lib which is expected to be included in the main project. The 3 main libraries are **Engine** (3d stuff, atm model format, etc), **Interface** (2d stuff) and **CodeUtil** (Core framework). 
3) **GameCommon** : largely stems from a brief stint mostly circa 2016-2017 spent trying to form a set of generic C++ game components that could be used in new projects. The expectation here being that a project would include the code folder(s) containing the blocks it wanted to use. Most of these components assume the inclusion of the Pub libraries above. There may even be some half-decent or usable code in some of this, but its unlikely.

Further descriptions of whats in Pub and whats in GameCommon can be found in the readmes in the subfolders.

In addition to the 2 sets of framework code there is:

3) **Samples** : Some very basic samples of the core Pub libraries in use, including a Hello World
4) **Tools** : Some tools that were created using the libraries, including the **ModelConverter** which is a windows tool for loading, exporting and manipulating the **.atm** model format that is a key component of the **Engine**.
5) **Exe** : Contains some necessary data files and dlls required to run any of the generated programs.

Preconditions
-------------

- For obscure reasons relating to the functioning of edit and continue (i.e. its inability to do what i need it to do in later versions), I keep around a version of Visual Studio 2013 express, which is where most of the project & solution files will come from. They should generally be compatable with newer versions (I guess..) though you may have some work to do to use anything other than the V120 toolchain
- Executables will get placed in the Exe folder which contains some required data files. When debugging you'll need to set your working directory there for things to be happy.
- If you were to try to use any parts of this in an external project, you'd need to put **[path_to_antiquated_repo]\Pub\Include**  somewhere in your project's include path to keep things happy.

Disclaimer
--------------
There's a bit in the MIT license about this all being provided AS IS and don't blame me for any of it. This **really** applies here. Even looking at this stuff is probably bad for your eyes, don't do it.

Do not hold me to account for terrible coding practices contained within. Most of the stuff here was made purely for fun and never intended to be made public. Some of it was written rapidly really really late at night and in places I've broken every good practice or sensible coding rule. There is much stuff that was done as a necessity for maintaining a lil indie game that has run publicly for over 20 years with rarely more than 1 very part-time coder involved, but which shouldn't have been done that way then, and absolutely shouldn't be done that way now.

It is not actively maintained. Most of it doesnt work. Those bits that do work are pretty much useless, and in every case there's alternative, modern and better ways of doing stuff than repeating whats done here. 

Also, it is not a reflection of my current coding practices or skills. This is historical shit, treat it like medicinal leeches or 1970s tv entertainers. Just point and laugh and perhaps wonder why anybody used to do things like that. Ok?

Shared Code
-------------
The engine includes and makes use of versions of:
- Bullet physics ( https://github.com/bulletphysics )
- curl ( https://curl.se/ )
- cjson ( https://github.com/DaveGamble/cJSON )
- zlib ( https://github.com/madler/zlib )
- libjpeg ( https://github.com/kornelski/libjpeg ) 

