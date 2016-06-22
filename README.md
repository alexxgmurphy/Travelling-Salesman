# Travelling-Salesman
Assignment Description

In this lab, you will implement a genetic algorithm for determining the best tour of the landmarks
in LA, starting and ending at LAX. You will want to review the week 13, lecture 2 notes for this
lab, particularly the supplemental notes that go over our specific formulation of the problem.

For our testing scripts, it is very important you follow the command line parameter syntax
exactly as described in this section. We will invoke your program as follows:
[program] inputfile popsize generations mutationchance seed

For example:

tsp-windows.exe locations.txt 8 5 10 1337

means that the input file is locations.txt, there is a population size of 8, there should be 5
generations, the mutation chance is 10%, and the starting seed value is 1337.
You may assume that your program will not be passed invalid command line parameters.

Note that there may be alternate locations files, of approximately the same number of locations,
that we’ll test your program against. Don’t hard code the locations.

If you are trying for the extra credit, as outlined in the extra credit section, you should only run
your extra credit code if the program receives a sixth parameter “extra.” For example:

tsp-windows.exe locations.txt 8 5 10 1337 extra

would have the same parameters as before, but now also uses whatever extra credit code you’ve
written. If the extra parameter is not passed, your program should still run to specification.

Output File

Your program should write its output to a single file called log.txt. Do not use any other file
name. We will ignore any console output, so feel free to write whatever you want to console. But
your log file must follow the format in the instructions.

I’ve provided several sample output files in the logs sub-folder. Because of platform differences,
each file has two versions, one for Windows and one for Mac. You should verify that the files on
your designated platform match the output you get.

All of these sample output files use the provided “locations.txt” file, and the remainder of the file
name tells you which command line parameters were used (with underscores instead of spaces).
