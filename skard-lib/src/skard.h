#ifndef SKARD_SKARD_H
#define SKARD_SKARD_H

#define SKARD_STR(x) #x
#define SKARD_XSTR(x) SKARD_STR(x)

#define SKARD_VERSION_MAJOR 0
#define SKARD_VERSION_MINOR 2
#define SKARD_VERSION_PATCH 1

#define SKARD_VERSION (SKARD_XSTR(SKARD_VERSION_MAJOR) "." SKARD_XSTR(SKARD_VERSION_MINOR) "." SKARD_XSTR(SKARD_VERSION_PATCH))

#include "chunk.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"


#endif //SKARD_SKARD_H
