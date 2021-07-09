There are 2 build scripts and generators in the more interesting call_tests
The first makes chains of functions with 50 xorq instructions in each function (bulking up the functions)
The second comments out the xorq block in the generator
The new build script (mostly) use parallel make files after the generators make huge numbers of source files
This randomizes the link order in the binary.
I recommend invoking the build scripts like this example 
./build_att_ran.sh 10 10 1000 > log1.log 2>&1 &
this will create a binary that executes 100 call stacks of 1000 functions

One could relink the FOO*.o files with “gcc -O0 -g FOO_main.o FOO_0*.o -o FOO_static” after the makefile creates a randomly ordered linked binary
This command will then create binaries with the functions in lexical order…this changes the results even if the functions are invoked in random order within the call stack  (be careful not to overwrite the random binary)
You need to invoke “ulimit -s 65536” in your shell to remove the large number (100k) of FOO*.0 and FOO*.c

most directories have scripts like run_st2.sh shown below which pins the binary to core2 and executes all the call stacks 10,000 times.
