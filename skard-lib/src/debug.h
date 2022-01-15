#ifndef SKARD_DEBUG_H
#define SKARD_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name);
size_t disassembleInstruction(Chunk *chunk, size_t offset);

#endif //SKARD_DEBUG_H
