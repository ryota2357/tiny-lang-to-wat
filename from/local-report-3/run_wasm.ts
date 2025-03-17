import { assert, ensure, is } from "jsr:@core/unknownutil@3.18.1";
import { join as pathJoin } from "jsr:@std/path@0.225.2";

const __dirname = ensure(import.meta.dirname, is.String);
const wasmCode = await Deno.readFile(pathJoin(__dirname, "main.wasm"));

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
