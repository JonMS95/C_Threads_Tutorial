# C_Threads_Tutorial: a series of lessons on C language's threads 🧵
This project focuses on threading, one of the most powerful features of the C language.


## Table of contents 🗂️
* [**Introduction** 📑](#introduction)
* [**Prerequisites** 🧱](#prerequisites)
* [**Usage** 🖱️](#usage)
* [**To do** ☑️](#to-do)
* [**Related documents** 🗄️](#related-documents)


## Introduction <a id="introduction"></a> 📑
Although I have used threads during my professional career as well as at university, I thought that it was a nice idea to have some deep insight on
how threads work in C. Some of the ideas covered in this tutorial are pretty basic and commonly used (such as thread creation, mutexes, or thread
detachment, among others) while others can be a bit more challenging and unusual, like timed mutex locks, condition variables and TLS (which stands
for _Thread Local Storage_ in this context, it has nothing to do with communications security).

Building this project has involved a deep understanding of:
* C/C++ programming
* Parallel-running processes
* Tests
* Linux OS operation


## Prerequisites <a id="prerequisites"></a> 🧱
By now, the application has only been tested in POSIX-compliant Linux distros. Many of the dependencies below may already be installed in the OS.
In the following list, the minimum versions required (if any) by the library are listed.

| Dependency                   | Purpose                                 | Minimum version |
| :--------------------------- | :-------------------------------------- |:-------------: |
| [gcc][gcc-link]              | Compile                                 |11.4            |
| [git][git-link]              | Download GitHub dependencies            |2.34.1          |
| [glibc][glibc-link]          | Interact with the OS                    |2.35            |


[gcc-link]:        https://gcc.gnu.org/
[git-link]:        https://git-scm.com/
[glibc-link]:      https://www.gnu.org/software/libc/

## Usage 🖱️
First, enter the local repo path and create a directory to store executable files (let's say it's called _exe_).
The local repository directory should be left as follows:

Again, the one below is the path to the generated executable file:
- **/path/to/repos/C_Threads_Tutorial**
  - **exe**
  - **src**

Now simply compile the source files, all of them at once:

```bash
gcc -lpthread src/* -o exe/main
```

Depending on the **_glibc_** version being used, the compiler may be unable to find any function referencing timed mutex locks. If that's the case, add the **-D_XOPEN_SOURCE=700** flag:

```bash
gcc -lpthread -D_XOPEN_SOURCE=700 src/* -o exe/main
```

Once it's done, reading each lesson's summary before executing the resulting file is strongly encouraged, so it's easier to grasp all the nuances. Enjoy!

## To do <a id="to-do"></a> ☑️
- [ ] Add practical examples (such as multi-threaded matrix multiplication or KNN algorithm implementation).

## Related Documents <a id="related-documents"></a> 🗄️
* [LICENSE](LICENSE)
* [CONTRIBUTING.md](Docs/CONTRIBUTING.md)
* [CHANGELOG.md](Docs/CHANGELOG.md)

