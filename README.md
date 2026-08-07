# µprint
Deferred, tokenized logging for embedded C/C++ built as a GCC compiler plugin.

`µprint` brings the zero-overhead logging approach popular in Rust (defmt) to C and C++ environments. It strips format strings at compile time, minimizes binary footprint, and keeps runtime overhead on resource-constrained microcontrollers to a minimum.

## The Idea
Traditional printf calls on microcontrollers come with high costs:
- Format strings clog up Flash memory.
- Formatting logic takes up execution time and stack space.
- Transmitting long text strings bogs down UART/SWO interfaces.

`µprint` shifts the heavy lifting from the target MCU to the development host:
1. **Compile Time**: A custom GCC plugin intercepts logging calls, extracts format strings into an ELF section, and replaces the call with a compact ID.
2. **Runtime**: The MCU transmits only the ID and raw binary arguments over the serial wire.
3. **Host Side**: A host tool parses the target's ELF file, maps incoming IDs back to their format strings, and prints the formatted log.

This transformation happens directly during compilation by adding a single flag to GCC.

## Existing Tools & Why `µprint` Exists
Tokenized logging isn't a new concept, but current C/C++ implementations usually add friction to the development workflow:
- [Google Pigweed](https://pigweed.dev/) (`pw_tokenizer`): Powerful, but heavily tied to the broader Pigweed ecosystem and complex build setups.
- [Trice](https://github.com/rokath/trice): Fast, but requires running external pre-build scripts (trice insert) or using verbose macros that clutter the code.
- Zephyr [Dictionary Logging](https://github.com/zephyrproject-rtos/zephyr) Works well out of the box, but locks you into the Zephyr RTOS.
- [cdefmt](https://github.com/RisinT96/cdefmt): A C port of defmt concepts, but relies on static metadata structs and linker script tricks rather than compiler-level transformations.
- [Postform](https://github.com/Javier-varez/Postform): Direct C++20 port of defmt ideas, but restricted to modern C++20 toolchains and specific host decoding tools.
- Rust [defmt](https://defmt.ferrous-systems.com/): Great developer experience, but restricted to Rust projects.

`µprint` moves the logic into a GCC plugin (via AST/GIMPLE passes) so you don't have to change your workflow:
- **Architecture-Agnostic:** Operates at the compiler IR level, making it completely independent of the target architecture (ARM, RISC-V, Xtensa, AVR, x86, etc.), register width (8/16/32/64-bit), and operating system.
- **No External Pre-processing:** No code-generation scripts, extra build phases, or source file modifications.
- **Clean Codebase:** No macro bloat or custom linker script overrides required.
- **Drop-in Integration:** Works with any standard C/C++ build system by passing a single `-fplugin` flag to GCC.

## Building & Usage

### Prerequisites
To build the plugin and run the host decoder, you need `gcc`, `g++`, and `gcc-plugin-dev` (version 12 or newer recommended) and `makefile`

On Ubuntu/Debian, install the host build dependencies with:
```bash
sudo apt update
sudo apt install build-essential gcc-12-plugin-dev
```
### 1. Building the Plugin
Clone the repository and build the C++ plugin shared library (`libuprint_plugin.so`):

```bash
git clone https://github.com/mahdy-asady/uprint.git
cd uprint/plugin
make
```
The compiled plugin will be located at `plugin/bin/uprint.so`.

## Project Status
`µprint` is being developed as a Bachelor's Thesis project exploring compiler-assisted optimization techniques for low-level embedded systems.

## License
Apache License 2.0 - see LICENSE for details.