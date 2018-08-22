function assert(a, msg) { if(!a) throw new Error(msg) }
function equal (a, b) {
  if (a.length !== b.length) return false
  for (var i = a.length; i; i--) {
    if (a[i] !== b[i]) return false
  }

  return true
}

fetch('../out/main.wasm').then(response =>
  response.arrayBuffer()
).then(bytes => WebAssembly.instantiate(bytes)).then(results => {
  instance = results.instance;
  var buf = new Uint8Array([
    1 |       // fin
    1 << 1 |  // rsv1
    1 << 2 |  // rsv2
    1 << 3 |  // rsv3
    0xf << 4, // opcode
    1 |       // mask
    127 << 1, // payload_len
    0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff
  ]);

  var mem = new Uint8Array(instance.exports.memory.buffer)
  var ptr = instance.exports.__heap_base
  mem.set(buf, ptr)

  var maskingKeyPtr = ptr + buf.byteLength
  var maskingKey = new Uint8Array(mem.buffer, maskingKeyPtr, 4)

  assert(instance.exports.fin(ptr) === 1)
  assert(instance.exports.rsv1(ptr) === 1)
  assert(instance.exports.rsv2(ptr) === 1)
  assert(instance.exports.rsv3(ptr) === 1)
  assert(instance.exports.opcode(ptr) === 0xf, 'opcode')
  assert(instance.exports.mask(ptr) === 1, 'mask')
  assert(instance.exports.masking_key(ptr, maskingKeyPtr) === 0, 'masking_key')
  assert(equal(maskingKey, [0xff, 0xff, 0xff, 0xff]), 'maskingKey data')
  console.log(instance.exports.payload_length53(ptr))
  assert(instance.exports.payload_length53(ptr) === Number.MAX_SAFE_INTEGER, 'length')
}).catch(console.error);
