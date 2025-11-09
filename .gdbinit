define target remote
target extended-remote $arg0
symbol-file ./cmake-build-debug/main.elf
monitor reset shellhalt
load ./cmake-build-debug/main.elf
end