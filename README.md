# tiny-lang-to-wat

A toy programming language with arithmetic, first-class functions, recursion, and conditional expressions.

## Build

Before building, ensure you have both **clang** (>=19.1.7) and **Nix** (with flake, >=2.24.10) installed.

Enter devShell by `nix develop` or nix-direnv, then run:

```console
$ make
```

For non-Nix users, please prepare the following environment:

- `flex`: 2.6.4
- `bison`: 3.8.2
- `deno`: 2.2.4

## Language Features

- Values
  - Numeric: All numbers are represented as `double`.
  - Functions:
    - First-class object.
    - **Do not** capture their environment.
    - **Not** automatically curried.
    - A new variable scope is created within function bodies.
    - Recursion is supported.
- Conditional Expressions:
  - Supports `if` expressions.
    - A single expression is evaluated as a statement (syntax: item := expr), so the if expression can be seen as an if statement as well.
    - `else if` is not supported; use `else { if { .. } }` instead.
- Scoped Variables: Variables are declared with `let` and scopes are defined at the function level.
- Operators
  - Arithmetic operator `+`, `-`, `*`, `/` and logical operator `&&`, `||`  are supported.
  - Logical operators (&&, ||) perform short-circuit evaluation.
- Line Comments: Single-line comments start with `//`.

## Usage

The build process produces two main executables: bin/eval and bin/compile.

- `bin/eval`: An interpreter for running your source code.
- `bin/compile`: A compiler that converts your source code into WebAssembly Text Format (WAT).

### Interpreting Code

You can run your code directly using the interpreter:

```console
# From a file
$ bin/eval path/to/code.tl2w

# From standard input
$ cat path/to/code.tl2w | bin/eval
```

### Compiling Code to WAT

To compile your code to WAT, use the compiler executable:

```console
# From a file
$ bin/compile path/to/code.tl2w > output.wat

# From standard input
$ cat path/to/code.tl2w | bin/compile > output.wat
```

### Executing Compiled Code

There is an executable script, [./wrt.ts](./wrt.ts), with proper execution permissions.

It can execute the compiled WAT code by converting it internally to WebAssembly, or it can run precompiled WASM files.
The following usages are supported:

```console
# Directly executing compiled WAT from the compiler
$ bin/compile path/to/code.tl2w | ./wrt.ts

# Executing a WAT file
$ ./wrt.ts path/to/output.wat

# Executing a WASM file
$ ./wrt.ts path/to/output.wasm
```

Note: The WAT and WASM files must be generated using `bin/compile` from valid source code.

## Example

For more examples, see [./examples](./examples) directory.

```
// Assignment
let a = 1
print(a) // 1

// Function (one line)
let sum a b = a + b
print(sum(10, 20))  // 30

// Functions creates new scopes, so 'a' is the same as the assignment above
// i.e. it is not overwritten by the `sum` function.
print(a)  // 1

// Recursive function
let fib n = {
    if n < 3 {
        1
    } else {
        fib(n - 1) + fib(n - 2)
    }
}
print(fib(15))  // 610

// Functions with 0 arguments need () in their definition
let print_123456789 () = print(123456789)
print_123456789()

// if expression
let x = if 1 < 2 { 7 } else { 13 }
print(x)  // 7

// Short-circuit evaluation of logical operators
let not_call () = print(1029384756)
let _ = 0 && not_call()
let _ = 1 || not_call()
let should_call () = print(0)
let _ = 1 && should_call() // 0
let _ = 0 || should_call() // 0

// Arithmetic operations with decimals
let x = 3
let y = -1.2 * 2
print(x / 2 + y * 3)  // ≈ -5.7

// Function returning a function
let fnfn () = {
    let res () = {
        print_123456789()
    }
    res
}
fnfn()()

// Reference to undefined variable
print(undef)  // error: `undef` is not defined
```
