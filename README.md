<h1 align="center">Operating System Input Output Library in C</h1>

<p align="center"> The so_stdio.h header defines three variable types, several macros, and various functions for performing buffered input and output operations with files.
    <br> 
</p>

## 📝 Table of Contents
- [About](#about)
- [Getting Started](#getting_started)
- [Usage](#usage)
- [Built Using](#built_using)
- [Contributing](../CONTRIBUTING.md)

## 🧐 About <a name = "about"></a>
 The following function signatures are exported by the so_stdio.h library:
```C
SO_FILE *so_fopen(const char *pathname, const char *mode);
int so_fclose(SO_FILE *stream);
int so_fileno(SO_FILE *stream);
int so_fflush(SO_FILE *stream);
int so_fputc(int c, SO_FILE *stream);
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream);
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream);
int so_fseek(SO_FILE *stream, long offset, int whence);
long so_ftell(SO_FILE *stream);
```
To use this functions you must add the so_stdio.h header in your source and specify at compile time where the library can be found
## 🏁 Getting Started <a name = "getting_started"></a>
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites
To use the so_stdio.h library on your projects you must have:

* for UNIX based operating systems:
  * gcc - is a tool from the GNU Compiler Collection used to compile and link C programs

### Installing
This is a step by step series of examples that tell you how to get a development env running.

* Linux:
  * start by updating the packages list
    ```bash
    $sudo apt-get update
    ```
  * install the build-essential package(a package of new packages including gcc, g++ and make) by typing:
    ```bash
    $sudo apt-get install build-essential 
    ```
## 🔧 Running the tests <a name = "tests"></a>
If you want to run the automated tests for Linux system you must follow the following steps:
* clone the repository by copping the following command in your terminal:
  ```
  git clone https://github.com/the-sergiu/Standard-Input-Output-Library-in-C
  ```
* go into the project director and run the following command:
  ```bash
  make test
  ```
* the results of the tests will be in the newly created file result.out

## 🎈 Usage <a name="usage"></a>
If you want to use the ***so_stdio.so*** library in your projects then you must add the ***so_stdio.h*** header in the desired source file and specify at the compile time the path to the so_stdio.so library.

* running the following command in the project director will generate the so_stdio.so:
  ```bash
  make build
  ```


## ⛏️ Built Using <a name = "built_using"></a>
- [Visual Studio Code](https://code.visualstudio.com/) - code editor
- [GCC](https://gcc.gnu.org/) - used to compile the library on my Linux machine

More details regarding assignment (Romanian):
https://ocw.cs.pub.ro/courses/so/teme/tema-2
