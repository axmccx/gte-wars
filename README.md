# GTE Wars

This is a POC of a PS1 game, an exercise in learning about PS1 homebrew development. 
I hope I can eventually make this into a playable arcade type game.
Heavily inspired by spicyjpeg's [ps1-bare-metal](https://github.com/spicyjpeg/ps1-bare-metal) repo, it avoids using any SDK.

## Building

Running an arch-like distribution, I've had trouble using the `mipsel-elf-gcc` and `mipsel-elf-binutils` packages, 
as well as the `mipsel-none-elf-binutils-git` and `mipsel-none-elf-gcc-git` packages from aur. I've had the best success 
compiling them myself, specifically with version 15.2.0 of gcc. [spawn-compiler.sh](https://github.com/grumpycoders/pcsx-redux/blob/e90d74d6fe7b090d7f440a2aa546a3ba4bc3a086/tools/linux-mips/spawn-compiler.sh).

I use python 3.13, required for some tools:
```
python -m venv venv
source venv/bin/activate
pip install -r tools/requirements.txt
```

I use my IDE (CLion) to build, but can also use:
```
cmake --preset debug
cmake --build build
```

I had to do: ` cmake --preset debug -DTOOLCHAIN_PATH=/usr/local/mipsel-none-elf/bin`


## Running compile binaries

...