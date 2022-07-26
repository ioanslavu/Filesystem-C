# File system wrote in C using generic linked list and tree data structers
Commands:
- touch <filename> [filecontent], creates a file in the current
directory and adds it to the current directory's list.
- ls [arg] list files from current directory or from the
specified directory iterating through the directory content.
- mkdir <dirname> creates a directory in the current directory
and adds it to the current directory's list.
- cd <path> changes the current directory to the specified one.
- tree [path] prints the directory tree from the current directory
or from the specified one.
- pwd prints the current directory.
- rmdir <dirname> removes the specified directory from the current
directory's list.
- rm <filename> removes the specified file from the current
directory's list.
- rmrec <resourcename> removes the specified folder and all its
content from the current directory's list.
- cp <source_path> <destination_path>
copies the specified file or directory to the specified destination.
- mv <source_path> <destination_path> 
moves the specified file or directory to the specified destination,
unlink the source file or directory from source parent directory 
and adds it to the destination parent directory.

For any command, if the path exists, it will navigate through all the folders
to find the file or directory you are looking for. Same for the tree command.
