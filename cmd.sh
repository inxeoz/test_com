rm -rf build
mkdir "build"
cp code.ved ./build
cd build || exit
cmake ..
make

### getting input and saving ir to llvm file
./test_compiler code.ved > output.ll

###### start of dependent process for native (linux i guess)
ls
cat output.ll
llc -filetype=obj output.ll -o output.o
ls
clang output.o -o test_program -no-pie
ls
./test_program
#### end of dependent process


####for windows
#sudo apt install clang llvm mingw-w64 ##to install mingw to work with windows system


###### start of dependent process for windows
ls
cat output.ll
llc -filetype=obj -mtriple=x86_64-w64-mingw32 output.ll -o output.o
ls
x86_64-w64-mingw32-g++ -o test_program.exe output.o -static
ls
#### end of dependent process



