# FileManagerCpp

FileManagerCpp is a lightweight file manager written in C++ using the Qt5 framework.

# Screenshots

| Screenshot 1 | Screenshot 2 |
|--------------|--------------|
| ![](images/screenshot1.png) | ![](images/screenshot2.png) |

# Change Log

• Reworked the style handling system: no more style-loading errors when the binary is moved. If the styles directory is located next to the binary or in `/usr/share/filemanager/`, everything works correctly (automatic style installation is planned for future updates).

• You can now launch the file manager with a specific directory:
```bash
fileManager <path>
```

or

```bash
./fileManager <path>
```

Depending on whether the binary is included in `$PATH`.

New features of this update:

```bash
fileManager .
```
• Opens the current terminal directory.

```bash
fileManager ~
```
• Opens the user’s home directory (`/root` for root, `/home/$USER` for regular users).

# How to use?

# Linux

First, install the package `qt5-base`

Then clone the repository:
```bash
git clone https://github.com/ArchLinuxUserIgor/FileManagerCpp.git
```

Enter the project directory:
```bash
cd FileManagerCpp
```

Buld the app:
```bash
qmake
make -j4;
```

After a successful build, run:
```bash
./fileManager
```

You can also place the `fileManager` binary somewhere in your $PATH to run it without `./`.

Done! You can use my file manager

# Windows

Not supported at the moment.

Windows support will be provided in a separate repository soon.
