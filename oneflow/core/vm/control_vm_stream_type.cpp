#include "oneflow/core/vm/vm_stream_desc.msg.h"
#include "oneflow/core/vm/control_vm_stream_type.h"
#include "oneflow/core/vm/vm_instruction.msg.h"
#include "oneflow/core/vm/scheduler.msg.h"
#include "oneflow/core/common/util.h"
#include "oneflow/core/common/flat_msg_view.h"

namespace oneflow {
namespace vm {

enum CtrlInstrOpCode { kNewSymbol = 0, kDeleteSymbol };

typedef void (*CtrlInstrFunc)(Scheduler*, InstructionMsg*);
std::vector<CtrlInstrFunc> ctrl_instr_table;

#define REGISTER_CTRL_INSTRUCTION(op_code, function_name) \
  COMMAND({                                               \
    ctrl_instr_table.resize(op_code + 1);                 \
    ctrl_instr_table.at(op_code) = &function_name;        \
  })

// clang-format off
FLAT_MSG_VIEW_BEGIN(NewSymbolCtrlInstruction);
  FLAT_MSG_VIEW_DEFINE_PATTERN(LogicalObjectId, logical_object_id);
  FLAT_MSG_VIEW_DEFINE_PATTERN(int64_t, parallel_num);
FLAT_MSG_VIEW_END(NewSymbolCtrlInstruction);
// clang-format on

ObjectMsgPtr<InstructionMsg> ControlStreamType::NewSymbol(const LogicalObjectId& logical_object_id,
                                                          int64_t parallel_num) const {
  auto vm_instr_msg = ObjectMsgPtr<InstructionMsg>::New();
  auto* vm_instr_id = vm_instr_msg->mutable_vm_instr_id();
  vm_instr_id->set_vm_stream_type_id(kStreamTypeId);
  vm_instr_id->set_opcode(CtrlInstrOpCode::kNewSymbol);
  {
    FlatMsgView<NewSymbolCtrlInstruction> view(vm_instr_msg->mutable_operand());
    view->set_logical_object_id(logical_object_id);
    view->set_parallel_num(parallel_num);
  }
  return vm_instr_msg;
}

void NewSymbol(Scheduler* scheduler, InstructionMsg* vm_instr_msg) {
  FlatMsgView<NewSymbolCtrlInstruction> view;
  CHECK(view->Match(vm_instr_msg->mut_operand()));
  auto logical_object = ObjectMsgPtr<LogicalObject>::NewFrom(
      scheduler->mut_scheduler_thread_only_allocator(), view->logical_object_id(), scheduler);
  CHECK(scheduler->mut_id2logical_object()->Insert(logical_object.Mutable()).second);
  auto* parallel_id2mirrored_object = logical_object->mut_parallel_id2mirrored_object();
  for (int64_t i = 0; i < view->parallel_num(); ++i) {
    auto mirrored_object = ObjectMsgPtr<MirroredObject>::NewFrom(scheduler->mut_allocator(),
                                                                 logical_object.Mutable(), i);
    CHECK(parallel_id2mirrored_object->Insert(mirrored_object.Mutable()).second);
  }
}
REGISTER_CTRL_INSTRUCTION(CtrlInstrOpCode::kNewSymbol, NewSymbol);
COMMAND(RegisterInstructionId<ControlStreamType>("NewSymbol", kNewSymbol, kRemote));
COMMAND(RegisterInstructionId<ControlStreamType>("LocalNewSymbol", kNewSymbol, kLocal));

// clang-format off
FLAT_MSG_VIEW_BEGIN(DeleteSymbolCtrlInstruction);
  FLAT_MSG_VIEW_DEFINE_PATTERN(MutableMirroredObjectOperand, mirrored_object_operand);
FLAT_MSG_VIEW_END(DeleteSymbolCtrlInstruction);
// clang-format on

const StreamTypeId ControlStreamType::kStreamTypeId;

ObjectMsgPtr<InstructionMsg> ControlStreamType::DeleteSymbol(
    const LogicalObjectId& logical_object_id) const {
  auto vm_instr_msg = ObjectMsgPtr<InstructionMsg>::New();
  auto* vm_instr_id = vm_instr_msg->mutable_vm_instr_id();
  vm_instr_id->set_vm_stream_type_id(kStreamTypeId);
  vm_instr_id->set_opcode(CtrlInstrOpCode::kDeleteSymbol);
  {
    FlatMsgView<DeleteSymbolCtrlInstruction> view(vm_instr_msg->mutable_operand());
    auto* mirrored_object_operand = view->mutable_mirrored_object_operand()->mutable_operand();
    mirrored_object_operand->set_logical_object_id(logical_object_id);
    mirrored_object_operand->mutable_all_parallel_id();
  }
  return vm_instr_msg;
}
void DeleteSymbol(Scheduler* scheduler, InstructionMsg* vm_instr_msg) {
  FlatMsgView<DeleteSymbolCtrlInstruction> view;
  CHECK(view->Match(vm_instr_msg->mut_operand()));
  const auto& logical_objectId = view->mirrored_object_operand().operand().logical_object_id();
  auto* logical_object = scheduler->mut_id2logical_object()->FindPtr(logical_objectId);
  CHECK_NOTNULL(logical_object);
  auto* parallel_id2mirrored_object = logical_object->mut_parallel_id2mirrored_object();
  for (int i = 0; i < parallel_id2mirrored_object->size(); ++i) {
    auto* mirrored_object = parallel_id2mirrored_object->FindPtr(i);
    CHECK(!mirrored_object->has_object_type());
    parallel_id2mirrored_object->Erase(mirrored_object);
  }
  scheduler->mut_id2logical_object()->Erase(logical_object);
}
REGISTER_CTRL_INSTRUCTION(CtrlInstrOpCode::kDeleteSymbol, DeleteSymbol);
COMMAND(RegisterInstructionId<ControlStreamType>("DeleteSymbol", kDeleteSymbol, kRemote));
COMMAND(RegisterInstructionId<ControlStreamType>("LocalDeleteSymbol", kDeleteSymbol, kLocal));

void ControlStreamType::Run(Scheduler* scheduler, InstructionMsg* vm_instr_msg) const {
  InstructionOpcode opcode = vm_instr_msg->vm_instr_id().opcode();
  return ctrl_instr_table.at(opcode)(scheduler, vm_instr_msg);
}

void ControlStreamType::Run(Scheduler* scheduler, InstrChain* vm_instr_chain) const {
  OBJECT_MSG_LIST_UNSAFE_FOR_EACH_PTR(vm_instr_chain->mut_vm_instruction_list(), vm_instr) {
    Run(scheduler, vm_instr->mut_vm_instr_msg());
  }
}

void ControlStreamType::InitInstructionStatus(const Stream& vm_stream,
                                              InstructionStatusBuffer* status_buffer) const {
  // do nothing
}

void ControlStreamType::DeleteInstructionStatus(const Stream& vm_stream,
                                                InstructionStatusBuffer* status_buffer) const {
  // do nothing
}

bool ControlStreamType::QueryInstructionStatusDone(
    const Stream& vm_stream, const InstructionStatusBuffer& status_buffer) const {
  UNIMPLEMENTED();
}

bool ControlStreamType::IsSourceOpcode(InstructionOpcode opcode) const {
  return opcode == kNewSymbol;
}

void ControlStreamType::Run(InstrChain* vm_instr_chain) const { UNIMPLEMENTED(); }

COMMAND(RegisterStreamType<ControlStreamType>());

}  // namespace vm
}  // namespace oneflow
