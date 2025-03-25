#!/usr/bin/env -S deno run --allow-read --allow-write --allow-run

import { assert, ensure, is } from "jsr:@core/unknownutil@4.3.0";
import { extname } from "jsr:@std/path@1.0.8";

async function main() {
  try {
    let wasmCode: Uint8Array;
    if (Deno.stdin.isTerminal()) {
      const args = Deno.args;
      if (args.length === 0) {
        throw new Error("A single file path argument is required");
      }
      if (args.length > 1) {
        throw new Error("Too many arguments, expected a single file path");
      }
      const filePath = args[0];
      wasmCode = await runFromFile(filePath);
    } else {
      wasmCode = await runFromStdin();
    }
    await runWasm(wasmCode);
  } catch (error) {
    console.error(
      `Error: ${error instanceof Error ? error.message : String(error)}`,
    );
    Deno.exit(1);
  }
}

// returns Uint8Array of wasm code
async function runFromFile(filePath: string): Promise<Uint8Array> {
  const fileExt = extname(filePath).toLowerCase();

  if (fileExt === ".wasm") {
    return await Deno.readFile(filePath);
  } else if (fileExt === ".wat") {
    const watContent = await Deno.readFile(filePath);
    return await wat2wasm(watContent);
  } else {
    throw new Error(
      `Unsupported file extension: ${fileExt}, expected .wasm or .wat`,
    );
  }
}

// returns Uint8Array of wasm code
async function runFromStdin(): Promise<Uint8Array> {
  const INITIAL_BUFFER_SIZE = 1024 * 1024; // 1MB
  const BUFFER_SIZE_LIMIT = 64 * 1024 * 1024; // 64MB

  let buffer = new Uint8Array(INITIAL_BUFFER_SIZE);
  let offset = 0;
  let bytesRead: number | null;
  while (true) {
    bytesRead = await Deno.stdin.read(buffer.subarray(offset, buffer.length));
    if (bytesRead === null) {
      break;
    }
    offset += bytesRead;

    if (offset === buffer.length && bytesRead > 0) {
      const nextSize = buffer.length * 2;
      if (nextSize >= BUFFER_SIZE_LIMIT) {
        throw new Error(
          `Input exceeds maximum size limit of ${
            BUFFER_SIZE_LIMIT / (1024 * 1024)
          }MB`,
        );
      }
      const nextBuffer = new Uint8Array(nextSize);
      nextBuffer.set(buffer);
      buffer = nextBuffer;
    }
  }

  if (offset === 0) {
    throw new Error("Empty input from stdin");
  }

  const watContent = buffer.subarray(0, offset);
  return await wat2wasm(watContent);
}

async function runWasm(wasmCode: Uint8Array) {
  const isWasmExports = is.ObjectOf({
    memory: is.InstanceOf(WebAssembly.Memory),
    main: is.Function,
  });

  let memory: WebAssembly.Memory;
  const { instance } = await WebAssembly.instantiate(wasmCode, {
    imports: {
      write_str(u_strPtr: unknown, u_length: unknown) {
        const strPtr = ensure(u_strPtr, is.Number);
        const length = ensure(u_length, is.Number);
        const buffer = new Uint8Array(memory.buffer, strPtr, length);
        Deno.stdout.writeSync(buffer);
      },
      write_f64(u_value: unknown) {
        const value = ensure(u_value, is.Number);
        const text = `${value}\n`;
        const buffer = new TextEncoder().encode(text);
        Deno.stdout.writeSync(buffer);
      },
      exit(u_code: unknown) {
        const code = ensure(u_code, is.Number);
        console.error(`%cExit with code ${code}`, "color: red;");
        Deno.exit(code);
      },
    },
  });

  assert(instance.exports, isWasmExports);
  memory = instance.exports.memory;

  const { main } = instance.exports;
  main();
}

async function wat2wasm(watContent: Uint8Array): Promise<Uint8Array> {
  const tempFilePath = await Deno.makeTempFile({
    prefix: "wrt-",
    suffix: ".wat",
  });
  await Deno.writeFile(tempFilePath, watContent);
  console.log(`Temp file created: ${tempFilePath}`);

  const cmd = new Deno.Command("wat2wasm", {
    args: ["--output=-", tempFilePath],
    stdin: "piped",
    stdout: "piped",
    stderr: "piped",
  });
  const process = cmd.spawn();
  const { code, stdout, stderr } = await process.output();

  if (code !== 0) {
    const errorMessage = new TextDecoder().decode(stderr);
    throw new Error(`wat2wasm failed with code ${code}: ${errorMessage}`);
  }

  return stdout;
}

await main();
