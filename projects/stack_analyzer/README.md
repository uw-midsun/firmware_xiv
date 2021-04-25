# stack_analyzer

This is an **experimental** port of the [Chromium OS Embedded Controller (EC) team][1]'s open source
[stack size analyzer][2] project, which aims to prevent stack overflows by calculating a project's
maximum possible stack usage through statically analyzing its disassembly. Their version is
available along with the rest of the EC source code under a permissive licence [here][3].

_**Please read [their documentation][2]!!**_ This README will attempt to give a Midnight Sun-specific
overview of the stack analyzer, but the Chromium documentation is authoritative on how it works.

## Usage
```
make analyzestack PROJECT=[project]
```
This will clean your build files, rebuild the project for STM32 with debug symbols enabled (i.e.
`STDLIB_DEBUG=true`, see platform/stm32f0xx/newlib-debug/README.md), then run `stack_analyzer.py` on
the build output. It will print a ton of output like this:
```
Worst-case stack usage (each interrupt fires when last ISR is at largest stack use):
Entry: main, Max size: 1068
Call Trace:
    main (144) [projects/power_distribution/src/main.c:109] 8004c08
        -> main[projects/power_distribution/src/main.c:129] 8004d22
    output_init (96) [projects/power_distribution/src/output.c:147] 8004620
        -> output_init[projects/power_distribution/src/output.c:183] 80047a8
           - prv_init_bts7200[projects/power_distribution/src/output.c:125]
    bts7200_init_pca9539r (24) [libraries/ms-drivers/src/bts7200_load_switch.c:83] 800764c
        -> bts7200_init_pca9539r[libraries/ms-drivers/src/bts7200_load_switch.c:126] 80076d6
        -> bts7200_init_pca9539r[libraries/ms-drivers/src/bts7200_load_switch.c:127] 80076e2
    bts7xxx_init_pin (8) [libraries/ms-drivers/src/bts7xxx_common.c:17] 8007984
        -> bts7xxx_init_pin[libraries/ms-drivers/src/bts7xxx_common.c:21] 800799a
    pca9539r_gpio_init_pin (16) [libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:59] 8007c0c
        -> pca9539r_gpio_init_pin[libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:69] 8007c50
        -> pca9539r_gpio_init_pin[libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:72] 8007c6a
    prv_set_reg_bit (40) [libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:45] 8007b94
        -> prv_set_reg_bit[libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:48] 8007bba
    prv_read_reg.constprop.0 (16) [libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:30] 8007b70
        -> prv_read_reg[libraries/ms-drivers/src/stm32f0xx/pca9539r_gpio_expander.c:33] 8007b88
    i2c_read_reg (56) [libraries/ms-common/src/stm32f0xx/i2c.c:160] 8006d28
        -> i2c_read_reg[libraries/ms-common/src/stm32f0xx/i2c.c:168] 8006d80
        -> i2c_read_reg[libraries/ms-common/src/stm32f0xx/i2c.c:169] 8006d9a
<snip>
Nested interrupt: TIM2_IRQHandler, Max size: 952 (924 + 28 context-switching overhead)
Call Trace:
    TIM2_IRQHandler (24) [libraries/ms-common/src/stm32f0xx/soft_timer.c:249] 80068f8
        -> [annotation]
    prv_measure_currents (32) [projects/power_distribution/src/current_measurement.c:17] 8004460
        -> prv_measure_currents[projects/power_distribution/src/current_measurement.c:25] 80044b2
    output_read_current (32) [projects/power_distribution/src/output.c:254] 8004904
        -> output_read_current[projects/power_distribution/src/output.c:268] 8004994
           - prv_read_current_bts7200[projects/power_distribution/src/output.c:250]
    bts7200_get_measurement (32) [libraries/ms-drivers/src/bts7200_load_switch.c:171] 80076f4
        -> bts7200_get_measurement[libraries/ms-drivers/src/bts7200_load_switch.c:192] 800775a
    bts7xxx_handle_fault_pin (8) [libraries/ms-drivers/src/bts7xxx_common_impl.c:15] 8007d58
        -> bts7xxx_handle_fault_pin[libraries/ms-drivers/src/bts7xxx_common_impl.c:18] 8007d70
    bts7xxx_disable_pin (8) [libraries/ms-drivers/src/bts7xxx_common.c:37] 80079cc
        -> bts7xxx_disable_pin[libraries/ms-drivers/src/bts7xxx_common.c:41] 80079e0
<snip>
Overall worst-case stack usage: 3996 bytes
Unresolved indirect callsites:
    In function prv_raise_blink_event_callback:
        -> prv_raise_blink_event_callback[libraries/ms-common/src/blink_event_generator.c:46] 800524a
    In function prv_fsm_state_none:
        -> prv_fsm_state_none[libraries/ms-common/src/lights_signal_fsm.c:37] 80054a0
<snip>
```
**The important thing to pay attention to** is the `Overall worst-case stack usage: 3996 bytes`
line. If this number is less than the number of bytes of static memory left for stack space that
`make build` gives you (it should be right above the `stack_analyzer.py` output) *and* there are
no `Unresolved indirect callsites`, then it's guaranteed that the project can never stack overflow.

## What does all the output mean?

In a tree of function calls, each call uses some stack space. The maximum possible stack usage is
achieved at the bottom of the tree when the total stack size used by all the functions in a call
chain is the maximum possible. So this output:
```
Entry: main, Max size: 1068
Call Trace:
    main (144) [projects/power_distribution/src/main.c:109] 8004c08
        -> main[projects/power_distribution/src/main.c:129] 8004d22
    output_init (96) [projects/power_distribution/src/output.c:147] 8004620
        -> output_init[projects/power_distribution/src/output.c:183] 80047a8
           - prv_init_bts7200[projects/power_distribution/src/output.c:125]
    bts7200_init_pca9539r (24) [libraries/ms-drivers/src/bts7200_load_switch.c:83] 800764c
    <snip>
```
means that the maximum stack usage from `main` is 1068 bytes, and what follows is the exact call
stack. `main` uses 144 bytes by itself, then calls `output_init` at line 129, which uses 96 bytes
and calls `bts7200_init_pca9539r` through an inlined function `prv_init_bts7200` (which doesn't use
additional stack space since it's inlined), and so on.

The hex like `8004c08` is the address in the binary where the functions/function calls are found.
Any unfamiliar functions with `newlib` in the path trace are standard library functions from
[newlib][4] (read about it in platforms/stm32f0xx/newlib-debug/README.md).

### Interrupts

But wait, there's more! Not all code is invoked from `main`'s call stack: the CPU also calls
*interrupt service routines* (ISRs) whenever an interrupt happens. There are 38 of these on the
STM32F072 MCU we use (see section 11.1.3 of the STM32F0xx manual) with names like `TIM2_IRQHandler`,
and they all have different priorities. Interrupts with higher priorities can interrupt those with
lower priorities, so the contrived scenario with the maximum possible total stack usage is as
follows:
1. We enter `main` and reach its call stack's maximum stack usage.
2. Then the lowest priority interrupt (`USB_IRQHandler`) fires, and its ISR reaches its maximum
   stack usage.
3. Then the *next* lowest priority interrupt fires, and its ISR reaches its maximum stack usage.
4. And so on, 36 more times...

So the maximum total stack usage is the sum of `main`'s total stack usage and all the ISRs' maximum
stack usages. This scenario is what's simulated in the output. For example:
```
Nested interrupt: TIM2_IRQHandler, Max size: 952 (924 + 28 context-switching overhead)
Call Trace:
    TIM2_IRQHandler (24) [libraries/ms-common/src/stm32f0xx/soft_timer.c:249] 80068f8
        -> [annotation]
    prv_measure_currents (32) [projects/power_distribution/src/current_measurement.c:17] 8004460
        -> prv_measure_currents[projects/power_distribution/src/current_measurement.c:25] 80044b2
    output_read_current (32) [projects/power_distribution/src/output.c:254] 8004904
        -> output_read_current[projects/power_distribution/src/output.c:268] 8004994
           - prv_read_current_bts7200[projects/power_distribution/src/output.c:250]
    <snip>
```
This output means that the `TIM2_IRQHandler` ISR (which handles soft timer interrupts) has a maximum
stack size of 952 bytes with the given call stack. The `+ 28 context-switching overhead` is because
[the CPU pushes 28 bytes on the stack][5] before calling an ISR to support saving and restoring its
place in the interrupted code.

## Annotations

The stack analyzer can easily follow direct function calls, but it can't automatically determine
where a call to a function pointer (an *indirect call*) could go. To help it out, we can write
`analyzestack.yaml` annotation files which add functions that could be called at each callsite.
The stack analyzer will report any unannotated indirect callsites under `Unresolved indirect
callsites:` in the output. Annotations can also be added to add missed call paths or remove false
positive call paths---the most useful application of this is removing any recursive call paths,
which will be reported under `There are cycles in the following function sets:`.

Here's an example annotation file:
```yaml
add:
  # annotate an indirect function call - you need the filename:line to eliminate an indirect call
  prv_call_a_callback[projects/my_project/main.c:80]:
    - prv_my_callback[projects/my_project/main.c]
  # a MidSun extension: annotate function call sites using an alias (see next section)
  # this adds a soft timer interrupt
  soft_timers:
    - prv_my_soft_timer_callback[projects/my_project/main.c]
  # annotate a call in the standard library (code at https://sourceware.org/newlib)
  bsearch[../newlib-cygwin/newlib/libc/search/bsearch.c:79]:
    - prv_handler_comp[libraries/ms-common/src/can_rx.c]
  # just add a call path that was missed: prv_one_function calls prv_another_function
  # paths can be used if there are multiple functions with the same name, or just for clarity
  prv_one_function:
    - prv_another_function
remove:
  # remove an invalid path: adc_read_converted can only recurse at most once
  - [adc_read_converted, adc_read_converted, adc_read_converted]
  # remove all paths pointing to prv_my_function
  - prv_my_function
```
See the [Chromium stack analyzer docs][6] for a more comprehensive example.

### Aliases

As a Midnight Sun extension, we support *annotation aliases* so you don't have to type out the
internal function + file + line number for aliases, which is prone to getting out of date quick.

Use the `ANALYZESTACK_ALIAS` macro from `analyzestack.h` (in libcore) in your STM32F0xx firmware:
```c
ANALYZESTACK_ALIAS("my_callback_call")
foo->bar();
```
Then you can use the alias in an `add` annotation to add candidates to the indirect call:
```yaml
add:
  # prv_my_callback could be called at the aliased site
  my_callback_call:
    - prv_my_callback
```

Here's a possibly-incomplete list of currently implemented aliases:
- `soft_timers`: for soft timer callbacks
- `gpio_interrupts`: for gpio interrupt callbacks

Feel free to add more! (Please follow the general naming scheme.)

### Annotation file resolution

`make analyzestack` will use and merge all annotation files from the given project's library
dependency tree as defined by `$(T)_DEPS` in `rules.mk` files. In practice, this means you can
provide `analyzestack.yaml` files in any library for any code in that library that needs to be
annotated, and it should all just work.

## Why do we rebuild with `STDLIB_DEBUG=true`?

By rebuilding with `STDLIB_DEBUG=true`, we get a C standard library ([newlib][4]) included in our
binary with debug symbols enabled, which lets us trace paths through files in the standard library
and annotate calls there. See platform/stm32f0xx/newlib-debug/README.md for more information.

[1]: https://chromium.googlesource.com/chromiumos/platform/ec/+/HEAD/README.md
[2]: https://www.chromium.org/chromium-os/ec-development/stack-size-analyzer
[3]: https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/main/extra/stack_analyzer/
[4]: https://sourceware.org/newlib
[5]: https://developer.arm.com/documentation/dui0497/a/the-cortex-m0-processor/exception-model/exception-entry-and-return
[6]: https://www.chromium.org/chromium-os/ec-development/stack-size-analyzer#TOC-Annotation1
