cmake_minimum_required(VERSION 3.16)
project(test_compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)


add_executable(test_compiler main.cpp ast.h)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Use llvm-config to link required LLVM libraries
execute_process(COMMAND llvm-config --libs interpreter executionengine core target irreader analysis
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND llvm-config --system-libs
        OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND llvm-config --ldflags
        OUTPUT_VARIABLE LLVM_LDFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set the linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LDFLAGS}")

# Link all required libraries
target_link_libraries(test_compiler ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS})














#
#
#
#
#
#
#inxeoz@192:~/CLionProjects/test_compiler/build$ echo "LLVM Version:" && llvm-config --version && \
#echo "Installed Components:" && llvm-config --components && \
#echo "Available Targets:" && llvm-config --targets-built && \
#echo "Include Directory:" && llvm-config --includedir && \
#echo "Library Directory:" && llvm-config --libdir && \
#echo "System Libraries:" && llvm-config --system-libs && \
#echo "Libraries to Link:" && llvm-config --libs all && \
#echo "CXX Compiler Flags:" && llvm-config --cxxflags && \
#echo "Linker Flags:" && llvm-config --ldflags && \
#echo "Full Configuration Summary:" && llvm-config --cxxflags --ldflags --system-libs --libs all
#LLVM Version:
#14.0.6
#Installed Components:
#aarch64 aarch64asmparser aarch64codegen aarch64desc aarch64disassembler aarch64info aarch64utils aggressiveinstcombine all all-targets amdgpu amdgpuasmparser amdgpucodegen amdgpudesc amdgpudisassembler amdgpuinfo amdgputargetmca amdgpuutils analysis arm armasmparser armcodegen armdesc armdisassembler arminfo armutils asmparser asmprinter avr avrasmparser avrcodegen avrdesc avrdisassembler avrinfo binaryformat bitreader bitstreamreader bitwriter bpf bpfasmparser bpfcodegen bpfdesc bpfdisassembler bpfinfo cfguard codegen core coroutines coverage debuginfocodeview debuginfodwarf debuginfogsym debuginfomsf debuginfopdb demangle dlltooldriver dwarflinker dwp engine executionengine extensions filecheck frontendopenacc frontendopenmp fuzzmutate globalisel hexagon hexagonasmparser hexagoncodegen hexagondesc hexagondisassembler hexagoninfo instcombine instrumentation interfacestub interpreter ipo irreader jitlink lanai lanaiasmparser lanaicodegen lanaidesc lanaidisassembler lanaiinfo libdriver lineeditor linker lto m68k m68kasmparser m68kcodegen m68kdesc m68kdisassembler m68kinfo mc mca mcdisassembler mcjit mcparser mips mipsasmparser mipscodegen mipsdesc mipsdisassembler mipsinfo mirparser msp430 msp430asmparser msp430codegen msp430desc msp430disassembler msp430info native nativecodegen nvptx nvptxcodegen nvptxdesc nvptxinfo objcarcopts object objectyaml option orcjit orcshared orctargetprocess passes perfjitevents powerpc powerpcasmparser powerpccodegen powerpcdesc powerpcdisassembler powerpcinfo profiledata remarks riscv riscvasmparser riscvcodegen riscvdesc riscvdisassembler riscvinfo runtimedyld scalaropts selectiondag sparc sparcasmparser sparccodegen sparcdesc sparcdisassembler sparcinfo support symbolize systemz systemzasmparser systemzcodegen systemzdesc systemzdisassembler systemzinfo tablegen target textapi transformutils ve veasmparser vecodegen vectorize vedesc vedisassembler veinfo webassembly webassemblyasmparser webassemblycodegen webassemblydesc webassemblydisassembler webassemblyinfo webassemblyutils windowsmanifest x86 x86asmparser x86codegen x86desc x86disassembler x86info x86targetmca xcore xcorecodegen xcoredesc xcoredisassembler xcoreinfo xray
#Available Targets:
#AArch64 AMDGPU ARM AVR BPF Hexagon Lanai Mips MSP430 NVPTX PowerPC RISCV Sparc SystemZ VE WebAssembly X86 XCore M68k
#Include Directory:
#/usr/lib/llvm-14/include
#Library Directory:
#/usr/lib/llvm-14/lib
#System Libraries:
#
#Libraries to Link:
#-lLLVM-14
#CXX Compiler Flags:
#-I/usr/lib/llvm-14/include -std=c++14   -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
#Linker Flags:
#-L/usr/lib/llvm-14/lib
#Full Configuration Summary:
#-I/usr/lib/llvm-14/include -std=c++14   -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
#-L/usr/lib/llvm-14/lib
#-lLLVM-14
#
#inxeoz@192:~/CLionProjects/test_compiler/build$
