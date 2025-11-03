#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "../config/config.h"

void* info(const char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");

  return NULL;
}

void* debug(const char* fmt, ...) {
  if (strcmp(ctx->cEnv, "development") != 0) {
    return NULL;
  }

  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "debug: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");

  return NULL;
}
