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
This seems to increase the binary size by ~5KB. Running on STM32 with the debug stdlib seems to work
fine, but in production we should use the normal stdlib, if only to optimize for space.

## How was it built?

Like so. Note that these steps are *entirely* untested, proceed at your own risk.
```bash
cd ~/shared
git clone git://sourceware.org/git/newlib-cygwin.git
mkdir newlib-build
cd newlib-build

# these options taken from https://stackoverflow.com/a/50348732
../newlib-build/configure --target=arm-none-eabi --enable-newlib-reent-small --disable-newlib-fvwrite-in-streamio --disable-newlib-fseek-optimization --disable-newlib-wide-orient --enable-newlib-nano-malloc --disable-newlib-unbuf-stream-opt --enable-lite-exit --enable-newlib-global-atexit --enable-newlib-nano-formatted-io --disable-nls

make
```
Libraries taken from the `arm-none-eabi/thumb/newlib` directory, because our MCUs run the Thumb
variant of ARM. Note that this actually builds newlib *nano*, which is a variant optimized for code
size and which gcc-arm-embedded uses too.

[1]: https://sourceware.org/newlib/
