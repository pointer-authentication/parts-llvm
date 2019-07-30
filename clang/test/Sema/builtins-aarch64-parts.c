// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -fsyntax-only -verify %s
#include <stdint.h>

struct foo;
void funcbar(void);

void (*foobar)(void);

uint64_t get_pa_modifier(uint64_t *value) {
  uint64_t res;

  res = __builtin_arm_parts_modifier(value);
  res |= __builtin_arm_parts_modifier(); // expected-error {{too few arguments to function call}}
  res |= __builtin_arm_parts_modifier(value, value); // expected-error {{too many arguments to function call}}
  res |= __builtin_arm_parts_modifier(*value); // expected-error {{address argument to builtin must be a pointer}}
  res |= __builtin_arm_parts_modifier(&value);
  res |= __builtin_arm_parts_modifier((const uint64_t *)value);
  res |= __builtin_arm_parts_modifier((const volatile uint64_t *)value);
  res |= __builtin_arm_parts_modifier((volatile uint64_t *)value);
  res |= __builtin_arm_parts_modifier((char *)value);
  res |= __builtin_arm_parts_modifier((struct foo *)value);
  res |= __builtin_arm_parts_modifier((unsigned long)value); // expected-error {{address argument to builtin must be a pointer}}
  res |= __builtin_arm_parts_modifier(42); // expected-error {{address argument to builtin must be a pointer}}
  res |= __builtin_arm_parts_modifier(funcbar);
  res |= __builtin_arm_parts_modifier(foobar);

  return res;
}
