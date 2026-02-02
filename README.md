# nexa

how to run this file is first install any linux based OS because LLVM is not supported in Win32 or Win64 versions
Then install clang and set up the OS along with LLVM installation 
To install Clang  - sudo apt install clang # check by clang --version
to install gpp - sudo apt install gcc # check by gpp --version
To install llvm - sudo apt install llvm # check by llvm-config --version

Install this project in the machine 
and follow the commands as:-
┌──(kali㉿kali)-[~/nexa]
└─$ make
[100%] Built target nexa
                                                                                             
┌──(kali㉿kali)-[~/nexa]
└─$ ./build/nexa tests/matrix.nx -o matrix.ll
Compiled successfully → matrix.ll
                                                                                             
┌──(kali㉿kali)-[~/nexa]
└─$ clang matrix.ll -o matrix_test
warning: overriding the module target triple with x86_64-pc-linux-gnu [-Woverride-module]
1 warning generated.
                                                                                             
┌──(kali㉿kali)-[~/nexa]
└─$ ./matrix_test
                                                                                             
┌──(kali㉿kali)-[~/nexa]
└─$ echo $?
