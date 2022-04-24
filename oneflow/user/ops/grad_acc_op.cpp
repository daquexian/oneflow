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
#include "oneflow/core/framework/framework.h"
#include "oneflow/core/framework/op_generated.h"

namespace oneflow {

/*static*/ Maybe<void> AccOp::GetSbp(user_op::SbpContext* ctx) {
  const user_op::TensorDesc& in = ctx->LogicalTensorDesc4InputArgNameAndIndex("in", 0);
  FOR_RANGE(int64_t, i, 0, in.shape().NumAxes()) {
    ctx->NewBuilder().Split(user_op::OpArg("in", 0), i).Split(user_op::OpArg("out", 0), i).Build();
  }
  ctx->NewBuilder()
      .PartialSum(user_op::OpArg("in", 0))
      .PartialSum(user_op::OpArg("out", 0))
      .Build();
  return Maybe<void>::Ok();
}
/*static*/ Maybe<void> AccOp::InferLogicalTensorDesc(user_op::InferContext* ctx) {
  *ctx->OutputShape("out", 0) = ctx->InputShape("in", 0);
  *ctx->OutputIsDynamic("out", 0) = ctx->InputIsDynamic("in", 0);
  return Maybe<void>::Ok();
}
/*static*/ Maybe<void> AccOp::InferPhysicalTensorDesc(user_op::InferContext* ctx) {
  return AccOp::InferLogicalTensorDesc(ctx);
}
/*static*/ Maybe<void> AccOp::InferDataType(user_op::InferContext* ctx) {
  *ctx->OutputDType("out", 0) = ctx->InputDType("in", 0);
  return Maybe<void>::Ok();
}

namespace {

REGISTER_USER_OP_GRAD("grad_acc").SetBackwardOpConfGenFn([](user_op::BackwardOpConfContext* ctx)
                                                        -> Maybe<void> {
  const auto grad_op_name = ctx->FwOp().op_name() + "_grad_acc";
  ctx->DefineOp(grad_op_name, [&ctx](user_op::BackwardOpBuilder& builder) {
    return builder.OpTypeName("repeat")
        .InputBind("in", ctx->FwOp().output_grad("out", 0))
        .Output("out")
        .Attr<int32_t>("repeat_num", ctx->FwOp().attr<int32_t>("max_acc_num"))
        .Build();
  });
  ctx->FwOp().InputGradBind(user_op::OpArg("in", 0), [&ctx, &grad_op_name]() -> const std::string& {
    return ctx->GetOp(grad_op_name).output("out", 0);
  });
  return Maybe<void>::Ok();
});

}  // namespace

}  // namespace oneflow