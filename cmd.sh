rm -rf build
mkdir "build"
cp code.ved ./build
cd build || exit
cmake ..
make
./test_compiler code.ved > output.ll
ls
cat output.ll
llc -filetype=obj output.ll -o output.o
clang output.o -o test_program -no-pie
ls
./test_program


