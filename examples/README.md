# Payment provider example

This directory contains a standalone CMake project for building the example
Praxis payment provider as a loadable STTrader module.

## Requirements

- Linux x86-64 matching the target server environment;
- CMake 3.16 or newer;
- a C++20-compatible GCC or Clang toolchain;
- a compatible `libstdc++` ABI with the server.

The payment ABI currently passes C++ standard-library types across the module
boundary. Build production modules with the same compiler family and compatible
standard library used for the server. Do not enable the legacy
`_GLIBCXX_USE_CXX11_ABI=0` mode unless the server was built with it as well.

## Release build

Run these commands from `sdk/payments/examples`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The resulting module is:

```text
build/payment_praxis.so
```

The CMake project applies these relevant settings:

- `-std=c++20`;
- position-independent code (`-fPIC`, selected by CMake for the module);
- `-O2 -DNDEBUG` for `Release`;
- `-fvisibility=hidden`, while required ABI functions are explicitly exported;
- `-Wall -Wextra -Wpedantic`;
- LTO/IPO when supported by the selected compiler.

To disable LTO, for example when diagnosing toolchain or loader issues:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPAYMENT_EXAMPLES_ENABLE_LTO=OFF
cmake --build build --parallel
```

Do not add `-march=native` to distributable modules: it can generate
instructions unsupported by another server CPU.

## Debug-symbol build

For an optimized module with debug symbols:

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DPAYMENT_EXAMPLES_ENABLE_LTO=OFF
cmake --build build-debug --parallel
```

This produces `build-debug/payment_praxis.so` with `-O2 -g -DNDEBUG`.

## Deployment

Copy the module into the server's `payments` directory using the exact output
name:

```text
payments/payment_praxis.so
```

The path is resolved relative to the STTrader process working directory. On
startup, Cashier scans that directory and should report:

```text
[CASHIER] Payment module available: provider=praxis module=payment_praxis api_version=100
```

The server then copies enabled modules into its private runtime directory. Do
not deploy modules directly into `var/sttrader/payments` because that directory
is managed and cleaned by the server.

## Export verification

The module must expose the following C symbols:

```text
GetPaymentApiVersion
GetPaymentProviderDescriptor
CreatePaymentProvider
DestroyPaymentProvider
```

`ValidatePaymentProviderConfig` is optional but recommended. It lets the
server return a precise configuration error instead of treating failed
provider creation as an unavailable service.

The exported symbols can be inspected without loading the module:

```bash
nm -D --defined-only build/payment_praxis.so
```

## Configuration

The Praxis example requires these keys inside `config_json`:

- `merchant_id`;
- `application_key`;
- `secret_key`.

The optional validation export reports missing field names but never returns
or logs secret values.
