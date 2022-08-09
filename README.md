Antiquated Engine
--------------
Old windows C/C++ game framework and legacy DX9 graphics engine

The code contained here was generated over many years as part of various hobby projects largely centered around the game 'The Universal', which was a never-quite-good-enough attempt at making a distributed MMO in the early 2000s.  

It is not well designed, well implemented, well structured or well.. anything but it is what it is.  Considering that most of it was written very late at night, often after a few whiskeys, its always surprising any of it works at all.

This project has mostly been created for the sake of posterity rather than any expectation of practical value.

Overall structure of the repo
-----------------------------
The repo consists of two main blocks of code that reflect different coding periods:
1) '**Pub**' : In this folder are the core 2D/3D DX-wrapper libraries that were used in 'The Universal'. These evolved intermittently (mostly) between 2000-2016 as the game expanded (and contracted, and slowly drifted off). Each library has a primary header file and generates a .lib which is expected to be included in the main project. The 3 main libraries are **Engine** (3d stuff, model format, etc), **Interface** (2d stuff) and **CodeUtil** (Core framework). 
2) **GameCommon** : largely stems from a brief stint around 2016-2017 spent trying to form a set of generic C++ game components that could be used in new projects. The expectation here is that a project will include the code folder containing the blocks it is wanting to use. Most of these components assume the inclusion of the Pub libraries above.

Further descriptions of whats in Pub and whats in GameCommon can be found in the readmes in the subfolders.

In addition to the 2 sets of framework code there is :

3) **Samples** : Some very basic samples of the core Pub libraries in use, including a Hello World
4) **Tools** : Some tools that were created using the libraries, including the **ModelConverter** which is a windows tool for loading, exporting and manipulating the **.atm** model format that is a key component of the **Engine**.


Preconditions
-------------

- Put **[path_to_antiquated_repo]\Pub\Include**  somewhere in your include path otherwise nothing will compile.
- I keep around a version of Visual Studio 2013 express, which is where most of the project & solution files will come from. They should generally be compatable with newer versions (I guess..)
