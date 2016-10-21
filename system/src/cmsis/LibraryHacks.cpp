/*
 * LibraryHacks.cpp
 *
 *  Created on: 23 Jan 2011
 *      Author: Andy
 */

#include <cstdlib>
#include <sys/types.h>


/*
 * Implement C++ new/delete operators using the heap
 */

void *operator new(size_t size) {
  return malloc(size);
}

void *operator new(size_t,void *ptr) {
  return ptr;
}

void *operator new[](size_t size) {
  return malloc(size);
}

void *operator new[](size_t,void *ptr) {
  return ptr;
}

void operator delete(void *p) {
  free(p);
}

void operator delete[](void *p) {
  free(p);
}


/*
 * EABI builds can generate reams of stack unwind code for system generated exceptions
 * e.g. (divide-by-zero). Since we don't support exceptions we'll wrap out these
 * symbols and save a lot of flash space.
 */

extern "C" void __wrap___aeabi_unwind_cpp_pr0() {}
extern "C" void __wrap___aeabi_unwind_cpp_pr1() {}
extern "C" void __wrap___aeabi_unwind_cpp_pr2() {}
