#pragma once

#define ASMFUNC __attribute__((cdecl))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))