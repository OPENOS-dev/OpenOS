// RUN: clspv %target  %s -o %t.spv
// RUN: spirv-dis -o %t2.spvasm %t.spv
// RUN: FileCheck %s < %t2.spvasm
// RUN: spirv-val --target-env vulkan1.0 %t.spv

// CHECK: OpCapability Float16
// CHECK: [[half:%[^ ]+]] = OpTypeFloat 16
// CHECK: [[float:%[^ ]+]] = OpTypeFloat 32
// CHECK: [[a:%[^ ]+]] = OpLoad [[half]]
// CHECK: [[b:%[^ ]+]] = OpLoad [[half]]
// CHECK: [[c:%[^ ]+]] = OpLoad [[half]]
// CHECK: [[a32:%[^ ]+]] = OpFConvert [[float]] [[a]]
// CHECK: [[b32:%[^ ]+]] = OpFConvert [[float]] [[b]]
// CHECK: [[c32:%[^ ]+]] = OpFConvert [[float]] [[c]]
// CHECK: [[fma32:%[^ ]+]] = OpExtInst [[float]] {{.*}} Fma [[a32]] [[b32]] [[c32]]
// CHECK: [[fma:%[^ ]+]] = OpFConvert [[half]] [[fma32]]
// CHECK: OpStore {{.*}} [[fma]]
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

kernel void foo(global half *dst, global half *srcA, global half *srcB, global half *srcC) {
    int gid = get_global_id(0);
    dst[gid] = srcA[gid] * srcB[gid] + srcC[gid];
}

