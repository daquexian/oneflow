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
#ifdef WITH_CUDA

#include "oneflow/core/vm/cuda_copy_d2h_stream_type.h"
#include "oneflow/core/vm/cuda_copy_d2h_device_context.h"

namespace oneflow {
namespace vm {

// Initializes CudaCopyD2HDeviceCtx which contains CudaStreamHandle
// object, The related istructions will be handled with CudaCopyD2HDeviceCtx
void CudaCopyD2HStreamType::InitDeviceCtx(std::unique_ptr<DeviceCtx>* device_ctx,
                                          Stream* stream) const {
  device_ctx->reset(new CudaCopyD2HDeviceCtx(stream->device_id()));
}

// Reinterprets status_buffer as CudaOptionalEventRecordStatusQuerier
void CudaCopyD2HStreamType::InitInstructionStatus(const Stream& stream,
                                                  InstructionStatusBuffer* status_buffer) const {
  static_assert(sizeof(CudaOptionalEventRecordStatusQuerier) < kInstructionStatusBufferBytes, "");
  auto* event_provider = dynamic_cast<QueryCudaEventProvider*>(stream.device_ctx().get());
  auto* data_ptr = status_buffer->mut_buffer();
  const auto& cuda_event = CHECK_NOTNULL(event_provider)->GetCudaEvent();
  CudaOptionalEventRecordStatusQuerier::PlacementNew(data_ptr, cuda_event);
}

void CudaCopyD2HStreamType::DeleteInstructionStatus(const Stream& stream,
                                                    InstructionStatusBuffer* status_buffer) const {
  auto* ptr = CudaOptionalEventRecordStatusQuerier::MutCast(status_buffer->mut_buffer());
  ptr->~CudaOptionalEventRecordStatusQuerier();
}

// Returns true if the instruction launched and the cuda event completed.
bool CudaCopyD2HStreamType::QueryInstructionStatusDone(
    const Stream& stream, const InstructionStatusBuffer& status_buffer) const {
  return CudaOptionalEventRecordStatusQuerier::Cast(status_buffer.buffer())->done();
}

// Launches a cuda kernel
void CudaCopyD2HStreamType::Compute(Instruction* instruction) const {
  auto* stream = instruction->mut_stream();
  cudaSetDevice(stream->device_id());
  instruction->instruction_type().ComputeIf(instruction);
  OF_CUDA_CHECK(cudaGetLastError());
  char* data_ptr = instruction->mut_status_buffer()->mut_buffer();
  CudaOptionalEventRecordStatusQuerier::MutCast(data_ptr)->SetLaunched(stream->device_ctx().get());
}

}  // namespace vm
}  // namespace oneflow

#endif
