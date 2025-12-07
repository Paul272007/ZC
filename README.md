# ZC

A utility to compile C easily and faster

## Dependencies

- GCC
- Binutils
- Python 3

## Installation :

- Clone the repository, go into ZC folder and run `./install.sh` with sudo privileges to install ZC

## Usage :

- #### Arguments :
  - one or more files (.c or .o)
  - Without options, the program just compiles the program and executes it without you having to add some options. It also deletes the created executable so your folder isn't filled with random execuatble files everywhere. If a filename is given without extension, the program will try to complete it with .c to compile it, and will raise an error if the file still isn't found.
- #### Options :
  - -h, --help : show help message.
  - -v, --version : show current version.
  - -c : same as gcc : create an object file instead of an executable. Also doesn't destroy the created file.
  - -k, --keep : keep the created binary instead of deleting it after it being executed.
  - -n, --newheader : genereate a header based on the given c file(s)
  - -a, --addlib : with a header file and a static library file, install the library by simply moving them into the right directories on the system.
  - -l, --newlib : create and install a static library entirely based on the given c file(s)
  - -r, --removelib : remove (delete files) an installed library on the system (works for libraries installed via ZC)
  - -f, --force : force installation or file creation even if it already exists (don't ask confirmation)
- #### Exit codes :
  - 0 : compilation succeeded.
  - 1 : compilation error from GCC. Executable wasn't generated.
  - 2 : file not found error.
  - 3 : no c file was given so the program couldn't compile.
  - 4 : .h or .a file is missing, so the library couldn't be installed.
  - 5 : the file names are not corresponding (header and library file).
  - 6 : the configuration file couldn't be loaded as it was not found.
  - 7 : the temporary file containing an object (.o) was unsuccessfully removed.
  - 8 : the temporary object file wasn't successfully compiled into a library file.
  - 9 : an error occured while moving the library file into its destination directory.
  - 10 : an error occured while moving the header file into its destination directory.
  - 11 : the .c file extension was expected but not given.
  - 12 : the static library the program is looking for doesn't exist.
  - 13 : the header file the program is looking for doesn't exist.
  - 14 : file was unsuccessfully deleted.
  - 15 : incorrect arguments number.
  - 16 : the given file(s) was/were unsuccessfully compiled into an object.

## Uninstalling :

- Simply run `./uninstall.sh` with sudo privileges to uninstall ZC and its configuration
