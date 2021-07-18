# newlib-debug: Newlib binaries with debug symbols

This directory contains a prebuilt version of [newlib][1], our C standard library on STM32, with
debug symbols.

## Wait, what?

The default newlib binaries shipped with gcc-arm-embedded (our compiler for STM32) have debug
information stripped. Debug symbols are useful for tools like GDB or the stack analyzer
(see projects/stack_analyzer) to give names to parts of the code, but they also make the binary
larger.

Use these binaries to help debugging in the standard library or when using the stack analyzer.

The sources referred to in the debugging information are available [here][1]. Paths will look like
`../newlib-cygwin/newlib/libc/stdio/printf.c`, because that's what the directory structure was when
it was built.

## How do I use it?

Use the `STDLIB_DEBUG` option when building for STM32:
```bash
make build PROJECT=centre_console STDLIB_DEBUG=true
```
Running on STM32 with the debug stdlib seems to work fine, but in production we should use the
normal stdlib, if only to optimize for space.

## How was it built?

Like so. Note that these steps are *entirely* untested, proceed at your own risk.
```bash
cd ~/shared
git clone git://sourceware.org/git/newlib-cygwin.git
mkdir newlib-build
cd newlib-build

# these options taken from https://stackoverflow.com/a/50348732
../newlib-cygwin/configure --target=arm-none-eabi --enable-newlib-reent-small --disable-newlib-fvwrite-in-streamio --disable-newlib-fseek-optimization --disable-newlib-wide-orient --enable-newlib-nano-malloc --disable-newlib-unbuf-stream-opt --enable-lite-exit --enable-newlib-global-atexit --enable-newlib-nano-formatted-io --disable-nls

make
```
The libraries were taken from the `arm-none-eabi/thumb/v6-m/newlib` directory, because our MCU, the
STM32F072, has an [ARM Cortex-M0][2] processor core, which supports the [ARMv6-M][3] instruction set
architecture (ISA) in [Thumb][4] mode (an ARM variant which takes half the bytes per instruction).
Note that this actually builds newlib *nano*, which is a newlib variant optimized for code size;
it's what gcc-arm-embedded uses too.

### What if we change from the STM32F072?

Then these binaries (probably) won't work! To change to a different MCU:
1. Figure out what processor core it uses ([Wikipedia has a good list][5]).
2. Build newlib from source with the instructions above.
3. Copy `lib[cgm].a` from the `newlib-build/arm-none-eabi/thumb/${arch}/newlib` directory, where
   `${arch}` corresponds to the precise architecture name without `ARM`, e.g. `v6-m` for ARMv6-M.

Note: there's also a generic `arm-none-eabi/thumb/newlib` directory which has binaries that work on
all ARM versions. This *wasn't* used because it uses a [really awkward function returning convention][6]
for compatibility where it pops the return address to some non-`lr` register like `r1` then calls
`bx r1`, which the stack analyzer was falsely detecting as an indirect function call.

[1]: https://sourceware.org/newlib/
[2]: https://en.wikipedia.org/wiki/ARM_Cortex-M#Cortex-M0
[3]: https://en.wikipedia.org/wiki/ARM_Cortex-M#ARMv6-M
[4]: https://en.wikipedia.org/wiki/ARM_architecture#Thumb
[5]: https://en.wikipedia.org/wiki/STM32
[6]: https://stackoverflow.com/a/38770325
