# Daisy Toolchain Library Notes

Purpose: capture Daisy firmware link/runtime differences that can create
surprising bugs even when the C++ code compiles.

## Local Evidence

Checked on 2026-06-07 in this checkout:

- Compiler: `C:\Program Files\DaisyToolchain\bin\arm-none-eabi-gcc.exe`
- Version: `GNU Arm Embedded Toolchain 10-2020-q4-major`, GCC `10.2.1`
- Shared link rule: `libDaisy/core/Makefile` adds:

```make
--specs=nano.specs --specs=nosys.specs
```

`nano.specs` resolves inside `C:\Program Files\DaisyToolchain` and changes the
normal link to nano variants:

- `-lc` becomes `-lc_nano`
- `-lstdc++` becomes `-lstdc++_nano`
- `-lsupc++` becomes `-lsupc++_nano`
- `=/include/newlib-nano` is added to the include search path

`nosys.specs` adds `-lnosys`, the no-operating-system syscall stub library.

The `Field_Template_June` map confirms these libraries in a real build:

- `libc_nano.a`
- `libstdc++_nano.a`
- `libnosys.a`
- `libm.a`
- `libdaisy.a`
- `libdaisysp.a`

## What This Means

Daisy Make firmware is not linked like a desktop C++ program. It is a bare-metal
STM32H7 firmware using size-optimized C/C++ runtime libraries and no OS syscall
backend. The code can compile while text formatting, heap allocation, or file I/O
silently behave differently than expected.

## Issues To Check First

| Surface | What can jump out | Default action |
|---------|-------------------|----------------|
| `printf`, `snprintf`, `PrintLine` float formats | `%f`, `%e`, `%g` can render blank, `?`, or missing under `newlib-nano` unless `_printf_float` is deliberately linked | Prefer integer formatting: `%d Hz`, `%d ms`, `%d%%`, or manual tenths |
| `scanf`, `sscanf`, `fscanf` float formats | Float input parsing also needs a deliberate nano float-scan choice; serial command parsers can accept malformed values silently | Parse integers or fixed-point text manually |
| `_printf_float` / `_scanf_float` flags | They increase flash usage and can push 128 KB internal-flash builds over the limit | Use only for debug or explicitly budgeted builds; record memory usage |
| `std::iostream`, `std::cout`, `std::cin` | Pulls larger C++ runtime and buffering assumptions into firmware | Use `hw.seed.PrintLine`, bounded `snprintf`, or libDaisy logger |
| `std::vector`, `std::string`, `std::function`, `new`, `malloc` | Pulls heap allocation through `libc_nano`; allocation may fragment or fail on long-running audio firmware | Prefer static arrays and fixed-capacity structs |
| `free`, `delete`, destructors, `atexit` | The nano C++ runtime may include destructor/atexit support, but firmware should not rely on desktop shutdown semantics | Do not design around program exit or teardown |
| POSIX/stdio file calls | `nosys` supplies syscall stubs for `_read`, `_write`, `_lseek`, `_close`, `_fstat`, `_isatty`, `_sbrk`; generic file I/O is not a real storage layer | Use FatFs/libDaisy storage APIs when storage is required |
| `printf` through libDaisy logger | Logger uses `vsnprintf` into a fixed 128-byte buffer and marks overflow with `$$` | Keep logs short and tagged; never log from audio callback |
| `libm` functions | `powf`, `expf`, `logf`, `tanhf`, `sinf`, `cosf` pull code and can be expensive in hot paths | Precompute, approximate, or move out of the audio callback when possible |
| Map file interpretation | Archive symbols can appear in cross-reference output even when not intentionally requested as features | Confirm with link flags and `arm-none-eabi-nm`, not map text alone |

## Practical Triage Order

1. Inspect the project `Makefile` for extra `LDFLAGS`, especially
   `_printf_float`, `_scanf_float`, custom `--specs`, or missing libDaisy core
   include.
2. Inspect source for float text formatting before changing OLED, serial, or
   control-routing code.
3. Inspect source for heap allocation and iostream usage before blaming random
   runtime instability.
4. Build and read the memory summary. Near-full internal-flash builds should not
   gain float printf unless there is a strong reason.
5. Check the `.map` and `arm-none-eabi-nm` output for unexpected pulls:
   `malloc`, `free`, `operator new`, `operator delete`, `powf`, `expf`, `logf`,
   `tanhf`, `iostream`, or full float formatting.

## Commands

```powershell
arm-none-eabi-gcc --version
arm-none-eabi-gcc "-print-file-name=nano.specs"
arm-none-eabi-gcc "-print-file-name=nosys.specs"
arm-none-eabi-gcc "-print-file-name=libc_nano.a"
arm-none-eabi-gcc "-print-file-name=libstdc++_nano.a"
arm-none-eabi-size -A build/<target>.elf
arm-none-eabi-nm -A build/<target>.elf | rg "malloc|free|new|delete|_printf_float|_scanf_float|powf|expf|logf|tanhf"
```

## Validator Coverage

`validate_daisy_code.py` now checks these library-related risks:

- `FLOAT-PRINTF-NANO`
- `FLOAT-SCANF-NANO`
- `FLOAT-PRINTF-FLAG`
- `FLOAT-SCANF-FLAG`
- `DYNAMIC-ALLOC`
- `IOSTREAM-USAGE`

These checks do not replace hardware testing. They catch the common cases early
so debugging does not drift toward OLED layout, control polling, or MIDI logic
when the real cause is the firmware runtime library boundary.
