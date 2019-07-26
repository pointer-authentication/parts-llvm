// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -fsyntax-only -verify %s
#include <stdint.h>

uint64_t get_pa_modifier(uint64_t *value) {
  uint64_t res;
  res = __builtin_arm_parts_modifier(value);
  res |= __builtin_arm_parts_modifier(); // expected-error {{too few arguments to function call}}
  res |= __builtin_arm_parts_modifier(value, value); // expected-error {{too many arguments to function call}}
  return res;
}
