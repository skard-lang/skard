#ifndef SKARD_DEBUG_H
#define SKARD_DEBUG_H

#include "chunk.h"

void disassemble_chunk(Chunk *chunk, const char *name);
size_t disassemble_instruction(Chunk *chunk, size_t offset);

#endif //SKARD_DEBUG_H
