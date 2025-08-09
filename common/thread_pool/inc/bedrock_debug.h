// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cutio::bedrock::debug {

#ifdef TRACER
inline void AbortingCall(int result) {
  if (result != 0) {
    fprintf(stderr, "Function call failed with errno = %d(%s). Aborting.\n",
            result, strerror(result));
    abort();
  }
}
#else  // TRACER
inline void AbortingCall(int result) {}
#endif  // TRACER

}  // namespace cutio::bedrock::debug
