Xuan(James) Zhai - CS 7343 - Project 3

1: How to compile the code:
1.1: If you are in Linux, you can do g++ -pthread *.cpp
1.2: If you are in Windows or others, since jrand is not a function in the standard library, you may need to use some flags in the command or just open it with some IDE.

2: How to run the code:
There are two command line arguments that the program will read in.
The first one is required which is the input file name.
The second one is optional, if you want to test it the first credit task, you just make "1" as the second argument.
(i.e. input.txt 1)

3: How does the extra part works (The one which is optional to undergrads).
If a writer is trying to get a resource but found out that it's currently holding by readers,
It will mark that resource "blocked" so that no others can get that resource.
The writer will keep trying to get that mutex, once it gets that, it will remove that "blocked" mark.
For a reader who is trying to get that blocked resource, its request will be denied.