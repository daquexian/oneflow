/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef _ONEFLOW_USER_KERNELS_ACTIVATION_KERNELS_H_
#define _ONEFLOW_USER_KERNELS_ACTIVATION_KERNELS_H_
#include "oneflow/core/ep/include/primitive/binary_op.h"
#include "oneflow/core/ep/include/primitive/broadcast_elementwise_binary.h"
#include "oneflow/user/kernels/elementwise_xpu_kernel.h"

namespace oneflow {

namespace {
auto UnaryPrimitiveExists(ep::primitive::UnaryOp op, const std::string& output_name,
                          const std::string& input_name) {
  return hob::make_custom(
      "ElementwiseUnaryPrimitiveExists", [=](const user_op::KernelRegContext& ctx) {
        const user_op::TensorDesc* src = ctx.TensorDesc4ArgNameAndIndex(input_name, 0);
        const user_op::TensorDesc* dst = ctx.TensorDesc4ArgNameAndIndex(output_name, 0);
        auto primitive = ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(
            ctx.device_type(), op, src->data_type(), dst->data_type());
        return primitive.operator bool();
      });
}

auto BinaryPrimitiveExists(ep::primitive::BinaryOp op, const std::string& output_name,
                           const std::string& input_a_name) {
  return hob::make_custom(
      "BroadcastElementwiseBinaryPrimitiveExists", [=](const user_op::KernelRegContext& ctx) {
        const user_op::TensorDesc* src0 = ctx.TensorDesc4ArgNameAndIndex(input_a_name, 0);
        const user_op::TensorDesc* dst = ctx.TensorDesc4ArgNameAndIndex(output_name, 0);
        auto primitive =
            ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(
                ctx.device_type(), op, src0->data_type(), dst->data_type(), 1 /*max_num_dims*/);
        return primitive.operator bool();
      });
}
}  // namespace

#define REGISTER_SOFTSHRINK_FORWARD_KERNEL()                                                 \
  REGISTER_USER_KERNEL("softshrink")                                                         \
      .SetCreateFn([]() {                                                                    \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                   \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                            \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);     \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);    \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(    \
                  ctx->device_type(), ep::primitive::UnaryOp::kSoftShrink, src->data_type(), \
                  dst->data_type(), ctx->Attr<double>("alpha"));                             \
            });                                                                              \
      })                                                                                     \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kSoftShrink, "out", "in"));

#define REGISTER_SOFTSHRINK_BACKWARD_KERNEL()                                                      \
  REGISTER_USER_KERNEL("softshrink_grad")                                                          \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "y", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("y", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kSoftshrinkBackwardWithDyY,             \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/, ctx->Attr<double>("alpha")); \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kSoftshrinkBackwardWithDyY, "dx", "y"));

#define REGISTER_ELU_FORWARD_KERNEL()                                                     \
  REGISTER_USER_KERNEL("elu")                                                             \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                         \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);  \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0); \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kElu, src->data_type(),     \
                  dst->data_type(), ctx->Attr<double>("alpha"));                          \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kElu, "out", "in"));

#define REGISTER_ELU_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("elu_grad")                                                                \
      .SetCreateFn([]() {                                                                         \
        return user_op::NewOpKernel<                                                              \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {      \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);               \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);              \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(   \
              ctx->device_type(), ep::primitive::BinaryOp::kEluBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/, ctx->Attr<double>("alpha"));                  \
        });                                                                                       \
      })                                                                                          \
      .SetIsMatchedHob(                                                                           \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kEluBackwardWithDyX, "dx", "x"));

#define REGISTER_GELU_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("gelu")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                         \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);  \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0); \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kGelu, src->data_type(),    \
                  dst->data_type());                                                      \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kGelu, "out", "in"));

#define REGISTER_GELU_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("gelu_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kGeluBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/);                                               \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kGeluBackwardWithDyX, "dx", "x"));

#define REGISTER_LEAKYRELU_FORWARD_KERNEL()                                                 \
  REGISTER_USER_KERNEL("leaky_relu")                                                        \
      .SetCreateFn([]() {                                                                   \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                  \
            "y", "x", [](user_op::KernelComputeContext* ctx) {                              \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);     \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("y", 0);     \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(   \
                  ctx->device_type(), ep::primitive::UnaryOp::kLeakyRelu, src->data_type(), \
                  dst->data_type(), ctx->Attr<float>("alpha"));                             \
            });                                                                             \
      })                                                                                    \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kLeakyRelu, "y", "x"));

#define REGISTER_LEAKYRELU_BACKWARD_KERNEL()                                                      \
  REGISTER_USER_KERNEL("leaky_relu_grad")                                                         \
      .SetCreateFn([]() {                                                                         \
        return user_op::NewOpKernel<                                                              \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {      \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);               \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);              \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(   \
              ctx->device_type(), ep::primitive::BinaryOp::kLeakyReluBackwardWithDyX,             \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/, ctx->Attr<float>("alpha")); \
        });                                                                                       \
      })                                                                                          \
      .SetIsMatchedHob(                                                                           \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kLeakyReluBackwardWithDyX, "dx", "x"));

#define REGISTER_CELU_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("celu")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                         \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);  \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0); \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kCelu, src->data_type(),    \
                  dst->data_type(), ctx->Attr<double>("alpha"));                          \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kCelu, "out", "in"));

#define REGISTER_CELU_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("celu_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kCeluBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/, ctx->Attr<double>("alpha"));                   \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kCeluBackwardWithDyX, "dx", "x"));

#define REGISTER_HARDSWISH_FORWARD_KERNEL()                                                 \
  REGISTER_USER_KERNEL("hardswish")                                                         \
      .SetCreateFn([]() {                                                                   \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                  \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                           \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);    \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);   \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(   \
                  ctx->device_type(), ep::primitive::UnaryOp::kHardSwish, src->data_type(), \
                  dst->data_type());                                                        \
            });                                                                             \
      })                                                                                    \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kHardSwish, "out", "in"));

#define REGISTER_HARDSWISH_BACKWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("hardswish_grad")                                                        \
      .SetCreateFn([]() {                                                                       \
        return user_op::NewOpKernel<                                                            \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {    \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);             \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);            \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>( \
              ctx->device_type(), ep::primitive::BinaryOp::kHardswishBackwardWithDyX,           \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/);                          \
        });                                                                                     \
      })                                                                                        \
      .SetIsMatchedHob(                                                                         \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kHardswishBackwardWithDyX, "dx", "x"));

#define REGISTER_HARDSIGMOID_FORWARD_KERNEL()                                                 \
  REGISTER_USER_KERNEL("hardsigmoid")                                                         \
      .SetCreateFn([]() {                                                                     \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                    \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                             \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);      \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);     \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(     \
                  ctx->device_type(), ep::primitive::UnaryOp::kHardSigmoid, src->data_type(), \
                  dst->data_type());                                                          \
            });                                                                               \
      })                                                                                      \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kHardSigmoid, "out", "in"));

#define REGISTER_HARDSIGMOID_BACKWARD_KERNEL()                                                  \
  REGISTER_USER_KERNEL("hardsigmoid_grad")                                                      \
      .SetCreateFn([]() {                                                                       \
        return user_op::NewOpKernel<                                                            \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {    \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);             \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);            \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>( \
              ctx->device_type(), ep::primitive::BinaryOp::kHardsigmoidBackwardWithDyX,         \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/);                          \
        });                                                                                     \
      })                                                                                        \
      .SetIsMatchedHob(                                                                         \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kHardsigmoidBackwardWithDyX, "dx", "x"));

#define REGISTER_HARDSHRINK_FORWARD_KERNEL()                                                   \
  REGISTER_USER_KERNEL("hardshrink")                                                           \
      .SetCreateFn([]() {                                                                      \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                     \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                              \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);       \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);      \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(      \
                  ctx->device_type(), ep::primitive::UnaryOp::kHardShrink, src->data_type(),   \
                  dst->data_type(), ctx->Attr<double>("lambd"));                               \
            });                                                                                \
      })                                                                                       \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kHardShrink, "out", "in")) \
      .SetInplaceProposalFn(                                                                   \
          [](const user_op::InferContext&,                                                     \
             const user_op::AddInplaceArgPair& AddInplaceArgPairFn) -> Maybe<void> {           \
            OF_RETURN_IF_ERROR(AddInplaceArgPairFn("out", 0, "in", 0, true));                  \
            return Maybe<void>::Ok();                                                          \
          });

#define REGISTER_HARDSHRINK_BACKWARD_KERNEL()                                                      \
  REGISTER_USER_KERNEL("hardshrink_grad")                                                          \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "y", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("y", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kHardshrinkBackwardWithDyY,             \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/, ctx->Attr<double>("lambd")); \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kHardshrinkBackwardWithDyY, "dx", "y"))   \
      .SetInplaceProposalFn(                                                                       \
          [](const user_op::InferContext&,                                                         \
             const user_op::AddInplaceArgPair& AddInplaceArgPairFn) -> Maybe<void> {               \
            OF_RETURN_IF_ERROR(AddInplaceArgPairFn("dx", 0, "dy", 0, true));                       \
            return Maybe<void>::Ok();                                                              \
          });

#define REGISTER_HARDTANH_FORWARD_KERNEL()                                                       \
  REGISTER_USER_KERNEL("hardtanh")                                                               \
      .SetCreateFn([]() {                                                                        \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                       \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                                \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);         \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);        \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(        \
                  ctx->device_type(), ep::primitive::UnaryOp::kHardTanh, src->data_type(),       \
                  dst->data_type(), ctx->Attr<double>("min_val"), ctx->Attr<double>("max_val")); \
            });                                                                                  \
      })                                                                                         \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kHardTanh, "out", "in"))     \
      .SetInplaceProposalFn(                                                                     \
          [](const user_op::InferContext&,                                                       \
             const user_op::AddInplaceArgPair& AddInplaceArgPairFn) -> Maybe<void> {             \
            OF_RETURN_IF_ERROR(AddInplaceArgPairFn("out", 0, "in", 0, true));                    \
            return Maybe<void>::Ok();                                                            \
          });

#define REGISTER_HARDTANH_BACKWARD_KERNEL()                                                     \
  REGISTER_USER_KERNEL("hardtanh_grad")                                                         \
      .SetCreateFn([]() {                                                                       \
        return user_op::NewOpKernel<                                                            \
            BinaryPrimitiveKernel>("dx", "y", "dy", [](user_op::KernelComputeContext* ctx) {    \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("y", 0);             \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);            \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>( \
              ctx->device_type(), ep::primitive::BinaryOp::kHardtanhBackwardWithDyY,            \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/,                           \
              ctx->Attr<double>("min_val"), ctx->Attr<double>("max_val"));                      \
        });                                                                                     \
      })                                                                                        \
      .SetIsMatchedHob(                                                                         \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kHardtanhBackwardWithDyY, "dx", "y"))  \
      .SetInplaceProposalFn(                                                                    \
          [](const user_op::InferContext&,                                                      \
             const user_op::AddInplaceArgPair& AddInplaceArgPairFn) -> Maybe<void> {            \
            OF_RETURN_IF_ERROR(AddInplaceArgPairFn("dx", 0, "dy", 0, true));                    \
            return Maybe<void>::Ok();                                                           \
          });

#define REGISTER_TANH_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("tanh")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "y", "x", [](user_op::KernelComputeContext* ctx) {                            \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);   \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("y", 0);   \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kTanh, src->data_type(),    \
                  dst->data_type());                                                      \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kTanh, "y", "x"));

#define REGISTER_TANH_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("tanh_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kTanhBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/);                                               \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kTanhBackwardWithDyX, "dx", "x"));

#define REGISTER_MISH_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("mish")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                         \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);  \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0); \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kMish, src->data_type(),    \
                  dst->data_type());                                                      \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kMish, "out", "in"));

#define REGISTER_MISH_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("mish_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kMishBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/);                                               \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kMishBackwardWithDyX, "dx", "x"));

#define REGISTER_SILU_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("silu")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                         \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);  \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0); \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kSilu, src->data_type(),    \
                  dst->data_type());                                                      \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kSilu, "out", "in"));

#define REGISTER_SILU_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("silu_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kSiluBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/);                                               \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kSiluBackwardWithDyX, "dx", "x"));

#define REGISTER_SELU_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("selu")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                         \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);  \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0); \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kSelu, src->data_type(),    \
                  dst->data_type());                                                      \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kSelu, "out", "in"));

#define REGISTER_SELU_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("selu_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kSeluBackwardWithDyX, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/);                                               \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kSeluBackwardWithDyX, "dx", "x"));

#define REGISTER_SOFTSIGN_FORWARD_KERNEL()                                                 \
  REGISTER_USER_KERNEL("softsign")                                                         \
      .SetCreateFn([]() {                                                                  \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                 \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                          \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);   \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);  \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(  \
                  ctx->device_type(), ep::primitive::UnaryOp::kSoftSign, src->data_type(), \
                  dst->data_type());                                                       \
            });                                                                            \
      })                                                                                   \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kSoftSign, "out", "in"));

#define REGISTER_SOFTSIGN_BACKWARD_KERNEL()                                                     \
  REGISTER_USER_KERNEL("softsign_grad")                                                         \
      .SetCreateFn([]() {                                                                       \
        return user_op::NewOpKernel<                                                            \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {    \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);             \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);            \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>( \
              ctx->device_type(), ep::primitive::BinaryOp::kSoftsignBackwardWithDyX,            \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/);                          \
        });                                                                                     \
      })                                                                                        \
      .SetIsMatchedHob(                                                                         \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kSoftsignBackwardWithDyX, "dx", "x"));

#define REGISTER_THRESHOLD_FORWARD_KERNEL()                                                 \
  REGISTER_USER_KERNEL("threshold")                                                         \
      .SetCreateFn([]() {                                                                   \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                  \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                           \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);    \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);   \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(   \
                  ctx->device_type(), ep::primitive::UnaryOp::kThreshold, src->data_type(), \
                  dst->data_type(), ctx->Attr<double>("threshold_val"),                     \
                  ctx->Attr<double>("value"));                                              \
            });                                                                             \
      })                                                                                    \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kThreshold, "out", "in"));

#define REGISTER_THRESHOLD_BACKWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("threshold_grad")                                                        \
      .SetCreateFn([]() {                                                                       \
        return user_op::NewOpKernel<                                                            \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {    \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);             \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);            \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>( \
              ctx->device_type(), ep::primitive::BinaryOp::kThresholdBackwardWithDyX,           \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/,                           \
              ctx->Attr<double>("threshold_val"));                                              \
        });                                                                                     \
      })                                                                                        \
      .SetIsMatchedHob(                                                                         \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kThresholdBackwardWithDyX, "dx", "x"));

#define REGISTER_SOFTPLUS_FORWARD_KERNEL()                                                      \
  REGISTER_USER_KERNEL("softplus")                                                              \
      .SetCreateFn([]() {                                                                       \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                      \
            "out", "in", [](user_op::KernelComputeContext* ctx) {                               \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("in", 0);        \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("out", 0);       \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>(       \
                  ctx->device_type(), ep::primitive::UnaryOp::kSoftPlus, src->data_type(),      \
                  dst->data_type(), ctx->Attr<double>("beta"), ctx->Attr<double>("threshold")); \
            });                                                                                 \
      })                                                                                        \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kSoftPlus, "out", "in"));

#define REGISTER_SOFTPLUS_BACKWARD_KERNEL()                                                      \
  REGISTER_USER_KERNEL("softplus_grad")                                                          \
      .SetCreateFn([]() {                                                                        \
        return user_op::NewOpKernel<                                                             \
            BinaryPrimitiveKernel>("dx", "x", "dy", [](user_op::KernelComputeContext* ctx) {     \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);              \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);             \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(  \
              ctx->device_type(), ep::primitive::BinaryOp::kSoftplusBackwardWithDyX,             \
              src->data_type(), dst->data_type(), 1 /*max_num_dims*/, ctx->Attr<double>("beta"), \
              ctx->Attr<double>("threshold"));                                                   \
        });                                                                                      \
      })                                                                                         \
      .SetIsMatchedHob(                                                                          \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kSoftplusBackwardWithDyX, "dx", "x"));

#define REGISTER_RELU_FORWARD_KERNEL()                                                    \
  REGISTER_USER_KERNEL("relu")                                                            \
      .SetCreateFn([]() {                                                                 \
        return user_op::NewOpKernel<UnaryPrimitiveKernel>(                                \
            "y", "x", [](user_op::KernelComputeContext* ctx) {                            \
              const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("x", 0);   \
              const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("y", 0);   \
              return ep::primitive::NewPrimitive<ep::primitive::ElementwiseUnaryFactory>( \
                  ctx->device_type(), ep::primitive::UnaryOp::kRelu, src->data_type(),    \
                  dst->data_type());                                                      \
            });                                                                           \
      })                                                                                  \
      .SetIsMatchedHob(UnaryPrimitiveExists(ep::primitive::UnaryOp::kRelu, "y", "x"))     \
      .SetInplaceProposalFn(                                                              \
          [](const user_op::InferContext&,                                                \
             const user_op::AddInplaceArgPair& AddInplaceArgPairFn) -> Maybe<void> {      \
            OF_RETURN_IF_ERROR(AddInplaceArgPairFn("y", 0, "x", 0, true));                \
            return Maybe<void>::Ok();                                                     \
          });

#define REGISTER_RELU_BACKWARD_KERNEL()                                                            \
  REGISTER_USER_KERNEL("relu_grad")                                                                \
      .SetCreateFn([]() {                                                                          \
        return user_op::NewOpKernel<                                                               \
            BinaryPrimitiveKernel>("dx", "y", "dy", [](user_op::KernelComputeContext* ctx) {       \
          const user_op::TensorDesc* src = ctx->TensorDesc4ArgNameAndIndex("y", 0);                \
          const user_op::TensorDesc* dst = ctx->TensorDesc4ArgNameAndIndex("dx", 0);               \
          return ep::primitive::NewPrimitive<ep::primitive::BroadcastElementwiseBinaryFactory>(    \
              ctx->device_type(), ep::primitive::BinaryOp::kReluBackwardWithDyY, src->data_type(), \
              dst->data_type(), 1 /*max_num_dims*/);                                               \
        });                                                                                        \
      })                                                                                           \
      .SetIsMatchedHob(                                                                            \
          BinaryPrimitiveExists(ep::primitive::BinaryOp::kReluBackwardWithDyY, "dx", "y"))         \
      .SetInplaceProposalFn(                                                                       \
          [](const user_op::InferContext&,                                                         \
             const user_op::AddInplaceArgPair& AddInplaceArgPairFn) -> Maybe<void> {               \
            OF_RETURN_IF_ERROR(AddInplaceArgPairFn("dx", 0, "dy", 0, true));                       \
            return Maybe<void>::Ok();                                                              \
          });

}  // namespace oneflow

#endif  // _ONEFLOW_USER_KERNELS_ACTIVATION_KERNELS_H_
