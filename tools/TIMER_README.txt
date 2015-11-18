timer.py is a python script that runs RJIT as a package over every gnur tests benchmark in gnur/tests. 

The script takes a compulsory argument, the name of the output file, and an optional argument the key word "new" which would overwrite the output file if one already existed with the same name.

It requires you to set inside the file the variable testpaths, which is where the gnur/tests directory should be, and the variable tp, which is the directory that you wish to print out the time result.

To generate the RJIT package you will need to run the command: ninja package_install, in your RJIT directory once RJIT is built. 

To have a fresh release of LLVM you will need to rebuild LLVM, which means you will need to run the command: ninja clean, in your LLVM directory. Then run tools/setup.sh -n in your RJIT directory.