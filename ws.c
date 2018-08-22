#include <stdint.h>

#define WASM_EXPORT __attribute__((visibility("default"))) extern
#define PACK __attribute__((__packed__))

#define WS_MASKING_KEY_BYTES 4

#define WS_OPCODE_CONT 0x0
#define WS_OPCODE_TEXT 0x1
#define WS_OPCODE_BINARY 0x2
#define WS_OPCODE_CLOSE 0x8
#define WS_OPCODE_PING 0x9
#define WS_OPCODE_PONG 0xa

typedef struct PACK ws_raw_header_s {
  uint8_t fin: 1;
  uint8_t rsv1: 1;
  uint8_t rsv2: 1;
  uint8_t rsv3: 1;
  uint8_t opcode: 4;
  uint8_t mask: 1;
  uint8_t payload_len: 7;
  unsigned char tail[2 + 6 + 4];
  // uint16_t payload_len_medium: 16;
  // uint64_t payload_len_long: 48;
  // uint32_t masking_key: 32;
} ws_raw_header_t;

WASM_EXPORT
inline uint8_t ws_fin(const ws_raw_header_t* header) {
  return header->fin;
}

WASM_EXPORT
inline uint8_t ws_rsv1(const ws_raw_header_t* header) {
  return header->rsv1;
}

WASM_EXPORT
inline uint8_t ws_rsv2(const ws_raw_header_t* header) {
  return header->rsv2;
}

WASM_EXPORT
inline uint8_t ws_rsv3(const ws_raw_header_t* header) {
  return header->rsv3;
}

WASM_EXPORT
inline uint8_t ws_opcode(const ws_raw_header_t* header) {
  return header->opcode;
}

WASM_EXPORT
inline int ws_mask(const ws_raw_header_t* header) {
  return header->mask;
}

WASM_EXPORT
inline int ws_masking_key(const ws_raw_header_t* header, unsigned char masking_key[WS_MASKING_KEY_BYTES]) {
  if (!mask(header)) return 1;

  unsigned char *base_ptr;
  if (header->payload_len < 126) base_ptr = (unsigned char *) header->tail;
  if (header->payload_len == 126) base_ptr = (unsigned char *) header->tail + 2;
  if (header->payload_len == 127) base_ptr = (unsigned char *) header->tail + 8;
  
  for (int i = 0; i < WS_MASKING_KEY_BYTES; i++) masking_key[i] = base_ptr[i];

  return 0;
}

WASM_EXPORT
inline double ws_payload_length53 (const ws_raw_header_t* header) {
  if (header->payload_len < 126) return header->payload_len;
  if (header->payload_len == 126) return header->tail[1] << 8 | header->tail[0];
  if (header->payload_len == 127) return (double) ((header->tail[6] & 0x1f) << 48 | header->tail[5] << 40 | header->tail[4] << 32 | header->tail[3] << 24 | header->tail[2] << 16 | header->tail[1] << 8 | header->tail[0]);
}


inline uint64_t ws_payload_length(const ws_raw_header_t* header) {
  if (header->payload_len < 126) return header->payload_len;
  if (header->payload_len == 126) return header->tail[1] << 8 | header->tail[0];
  if (header->payload_len == 127) return header->tail[7] << 56 | header->tail[6] << 48 | header->tail[5] << 40 | header->tail[4] << 32 | header->tail[3] << 24 | header->tail[2] << 16 | header->tail[1] << 8 | header->tail[0];
}


inline int ws_process_client_msg(const ws_raw_header_t* header) {
  if (ws_mask(header) == 0) return 1; // clients must mask 
}