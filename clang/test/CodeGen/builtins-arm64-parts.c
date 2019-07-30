// RUN: %clang_cc1 -triple arm64-unknown-linux -disable-O0-optnone -DPAUTH_ARMV8_3_A -emit-llvm -o - %s | opt -S -mem2reg | FileCheck %s
#include <stdint.h>

uint64_t get_pa_modifier(uint64_t *value)
{
  return __builtin_arm_parts_modifier(value);
// CHECK: call i64 @llvm.pa.modifier.p0i64(i64* %value)
}

uint64_t get_pa_modifier8(char *value)
{
  return __builtin_arm_parts_modifier(value);
// CHECK: call i64 @llvm.pa.modifier.p0i8(i8* %value)
}

struct foo;

uint64_t get_pa_modifier_struct(struct foo *value)
{
  return __builtin_arm_parts_modifier(value);
// CHECK: call i64 @llvm.pa.modifier.p0s_struct.foos(%struct.foo* %value)
}

uint64_t get_pa_modifier_func(void (*value)(void))
{
  return __builtin_arm_parts_modifier(value);
// CHECK: call i64 @llvm.pa.modifier.p0f_isVoidf(void ()* %value)
}
