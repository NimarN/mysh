CS214 | Project III: My Shell
Rutgers University | Fall 2023

Collaborators
-------------
Nimar Nasir - nn286
Bhumit Patel - bdp70


Overall Implementation
--------------------
mysh.c writes a greeting and then either starts in batch mode or interactive mode depending on the number of arguments.
It then takes input from either source and splits the input into tokens using the function acceptArgs().
It then sends those dynamically allocated arguments to processArgs(), which will systematically determine how to run the command.
It checks if it was given a filename, and then runs the THEN or ELSE conditional logic if neccessary. It exits if neccessary.
It then runs its wildcard expansion logic. This is followed by running the change directory (cd) BuiltIn if necessary.
It then checks for Pipes and sets up argument lists to implement that functionality if need be.
It then checks for Input and Output Redirection and sets up input and output files to prepare for execution.
During these step, it will truncate arguments that should no longer be the argument list.
It then checks if it needs to execute a local program or barenames, and takes the appropriate action.
During the barename execution, if the command to run is PWD or WHICH, it uses our BuiltIn instead of any external command.
During execution Child processes would have their input and output sources changed if neccessary, and would exit, returning
stdinput and stdoutput back to their original states.
The process then repeats until the user runs exit or the process is terminated with ctrl+c.


Test Plan
--------------------
Our testing stategy was constantly and rigorously testing different possible scenarios after implementing each seperate part of 
mysh. 
We had test programs to make sure our spereate components worked before integrating them into the main mysh.c file. 
We also created test files and test directories which we would use to test different commands of our shell. 
As our shell got more complex we made sure to use test inputs that would comptehensively test a multitude of different functions together at once. 
We made sure simple applications of our features worked before moving on to testing combinations of those features. 
An example of this would be testing wildcards in conjunction with multiple redirects and pipes, ensuring that in the most complex cases, our shell worked.
Throughout this process, we also made sure to eliminate memory leaks, segmentation faults, address misuse, and all other error types that may have occured.

Test Cases:

Scenarios:


Pathnames
--------------------


Barenames
--------------------


Built-in commands
--------------------
We implemented the CD command in the parent process and the PWD and WHICH commands in the child process. This allowed us to 
redirect the output of PWD and WHICH if necessary, which we tested. CD does not have output and only accepts 1 argument, so we
left it in the parent proecss. For all 3 BuiltIns, we created error checks for when they failed or if they were used incorrectly
when the user ran the command. 

Wildcards
--------------------
To implement wildcard expansion, we searched the appropriate directory for entries that contained a matching prefix and suffix
(which are the characters around the wildcard) for any arguments containing a wildcard. These were then input into the argument list
where the wildcard was found, maintaining argument order. If there are no matches to the wildcard, it is returned as normal with
no replacements.


Redirection
--------------------
For redirection, we parsed through the argument list and when we found a < or > symbol with an argument after it, we removed both
the redirect symbol and the filename after it from the argument list, storing the filename as either an output or input file.
If multiple redirects of the same type were found, we replaced old input/outfiles with new ones. These were fed as parameters
to our function which executed commands, so the child process could accurately change stdinput or stdoutput to be those files.
These changed file descriptors were then either reset by execv or manually by mysh.c for the builtins. 

Pipelines
--------------------


Conditionals
--------------------



Mysh.c
--------------------
This is the main code file which contains all methods used by our shell program, which also contains a header file to import our
wildcard implementation functions. It takes input, parses it, tokenizes it, and does all things described within "Our Implementation"


Wildcard.c
--------------------
This C file contains the functions to execute wildcard expansion. It searches the required directory for entries matching the expansion,
and correctly inserts them into an arrayList struct given by mysh.c.


///**We Lowkey dont need to elaborate on all of these functions below but they are here***////

execPipe()
--------------------


execProg()
--------------------


checkBareNames()
--------------------


processArgs()
--------------------


acceptArgs
--------------------





