Welcome to Linux File Explorer

A terminal based file explorer for linux based operating systems

How to compile project
g++ main.cpp

How to Run project
./a.out

1. Normal Mode:

1.1 Read and display list of files and directories in the current folder

    File explorer show each file in the directory (one entry per line). The following attributes are visible for each file
        File Name
        File size
        Ownership (User & Group) & Permissions
        Last modified

    The File explorer also handle scrolling (vertical overflow) in case the directory has a lot of files.

    The file explorer also show the entries “.” & “..” for current and parent directory respectively.

    User is able to navigate up & down the file list using corresponding arrow keys.
    
1.2 Open files & directories

    When enter is pressed
        Directory - It will Clear the screen and Navigate into the directory and shows the files & directories inside it as specified in point 1
        Files - It will open files using the corresponding default application.
        
Command Mode:

1. Type 'quit' + Press ENTER anytime to go out of command mode to normal mode	
2. User can perform any of the following commands in the given format

Command: copy

Syntax: copy <source_file(s)> <destination_directory>

Description: Copies multiple files into the destination directory. The operation is done recursively on directories

Command: copy_dir

Syntax: copy_dir <source_file(s)> <destination_directory>

Description: Copies multiple directories into the destination directory. The operation is done recursively on directories

Command: move

Syntax: move <source_file(s)> <destination_directory>

Description: Copies multiple files into the destination directory. The operation is done recursively on directories

Examples:

Command: move_dir

Syntax: move_dir <source_file(s)> <destination_directory>

Description: Copies multiple files into the destination directory. The operation is done recursively on directories

Command: rename

Syntax: rename <old_name> <new_name>

Description: Renames a file or directory with the new name given in command

Command: create_file

Syntax: create_file <file_name> <destination_directory>

Description: Creates a new file in the directory mentioned in <destination_directory>. Please note that "." in destination directory means current directory

Command: create_dir

Syntax: create_dir <directory_name> <destination_directory>

Description: Creates a new directory in the directory mentioned in <destination_directory>. Please note that "." in destination directory means current directory

Command: delete_file

Syntax: delete_file <file_name>

Description: Deletes the given file

Command: delete_dir

Syntax: delete_dir <dir_name>

Description: Deletes the given directory

Command: goto

Syntax: goto <dir_name>

Description: Traverses to the given directory

Command: search

Syntax: search <file/dir_name>

Description: Searches for the given files from the current directory recursively
