# Linux-Shell

This repository contains a custom Linux Shell that I had created as a mini project. Its operation is based on the *fork-join* model. The main process spawns children processes that execute the given commands.

My shell supports:
- Single Linux valid commands
- Multiple Linux commands separated by `;` or `&&`
- Redirections of input/output
- Interactive mode
- Detached mode

In *interactive mode* the commands are given as input in the terminal. In *detatched mode* the commands are read from a input file. My program parses the syntax of every command and executes it if it is valid. Pipes are not supported.


