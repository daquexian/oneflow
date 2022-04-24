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
#include "oneflow/core/graph/compute_task_node.h"
#include "oneflow/core/graph/task_stream_index_manager.h"
#include "oneflow/core/framework/framework.h"

namespace oneflow {

class GradAccCompTaskNode final : public CompTaskNode {
 public:
  OF_DISALLOW_COPY_AND_MOVE(GradAccCompTaskNode);
  GradAccCompTaskNode() = default;
  ~GradAccCompTaskNode() = default;
  TaskType GetTaskType() const override { return TaskType::kGradAcc; }
  void BuildExecGphAndRegst() override;
  void ProduceAllRegstsAndBindEdges() override;
  void ConsumeAllRegsts() override;
  void InferExecInterval() override;
};

void GradAccCompTaskNode::ProduceAllRegstsAndBindEdges() {
  std::shared_ptr<RegstDesc> regst = ProduceRegst("out", false, 1, 1);
  ForEachOutDataEdge([&](TaskEdge* edge) { edge->AddRegst("out", regst); });
}

void GradAccCompTaskNode::ConsumeAllRegsts() {
  ConsumeRegst("in", SoleInDataEdge()->GetSoleRegst());
}

void GradAccCompTaskNode::BuildExecGphAndRegst() {
  std::shared_ptr<RegstDesc> in_regst = GetSoleConsumedRegst("in");
  std::shared_ptr<RegstDesc> out_regst = GetProducedRegst("out");
  ExecNode* exec_node = mut_exec_gph().NewNode();
  exec_node->mut_op() = op();
  exec_node->BindBnWithRegst(op()->SoleIbn(), in_regst);
  out_regst->AddLbi(op()->BnInOp2Lbi(op()->SoleObn()));
  exec_node->BindBnWithRegst(op()->SoleObn(), out_regst);
  exec_node->InferBlobDescs(parallel_ctx());
  out_regst->ForEachLbi([out_regst](const LogicalBlobId& lbi) {
    const BlobDesc* blob_desc = out_regst->GetBlobDesc(lbi);
    CHECK_EQ(blob_desc->is_dynamic(), false);
  });
}

void GradAccCompTaskNode::InferExecInterval() {
  CHECK(op()->op_conf().user_conf().op_type_name() == "_grad_acc");
  set_exec_interval(user_op::UserOpConfWrapper(op()->op_conf()).attr<int32_t>("acc_num"));
  ForEachNodeOnInDataEdge([](TaskNode* src_node) { CHECK_EQ(src_node->exec_interval(), 1); });
}

REGISTER_COMP_TASK_STREAM_INDEX_GETTER(TaskType::kGradAcc);

REGISTER_USER_OP_COMP_TASK_NODE_TYPE("_grad_acc", GradAccCompTaskNode);

}  // namespace oneflow