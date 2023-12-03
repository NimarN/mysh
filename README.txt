CS214 | Project II: Word Occurence Count
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


Pathnames
--------------------


Barenames
--------------------


Built-in commands
--------------------


Wildcards
--------------------


Redirection
--------------------


Pipelines
--------------------


Conditionals
--------------------


execPipe()
--------------------



Mysh.c
--------------------



Wildcard.c
--------------------



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


execPipe()
--------------------




