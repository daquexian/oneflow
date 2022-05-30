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
#ifndef ONEFLOW_USER_KERNELS_ONEDNN_POOL_KERNEL_UTIL_H_
#define ONEFLOW_USER_KERNELS_ONEDNN_POOL_KERNEL_UTIL_H_
#ifdef WITH_ONEDNN

#include "oneflow/core/ep/cpu/cpu_stream.h"
#include "oneflow/core/ep/cpu/cpu_device.h"
#include "oneflow/user/kernels/onednn_util.h"

namespace oneflow {

template<typename T>
bool OneDnnPoolIsSupportDtype() {
  return (std::is_same<T, float>::value || std::is_same<T, int32_t>::value);
}

template<typename T>
struct OneDnnPoolKernelUtil {
  static void OneDnnPoolForwardCompute(ep::Stream* stream, const DnnlDims& src_dims,
                                       const DnnlDims& dst_dims, const DnnlDims& kernel_dims,
                                       const DnnlDims& strides_dims, const DnnlDims& padding_dims_l,
                                       const DnnlDims& padding_dims_r, const DnnlDims& dilation,
                                       dnnl::memory::format_tag format, void* src, void* dest,
                                       void* indice_ptr, dnnl::algorithm algorithm) {
    stream->As<ep::CpuStream>()->onednn_executor()->Launch([&](dnnl::engine* onednn_engine,
                                                               dnnl::stream* onednn_stream) {
      auto data_type = CppTypeToOneDnnDtype<T>();
      auto src_md = dnnl::memory::desc(src_dims, data_type, format);
      auto dst_md = dnnl::memory::desc(dst_dims, data_type, format);
      auto src_mem = dnnl::memory(src_md, *onednn_engine, src);
      auto dst_mem = dnnl::memory(dst_md, *onednn_engine, dest);

      auto pooling_desc = dnnl::pooling_v2_forward::desc(
          dnnl::prop_kind::forward_training, algorithm, src_md, dst_md, strides_dims, kernel_dims,
          dilation, padding_dims_l, padding_dims_r);
      auto pooling_primitive_desc =
          dnnl::pooling_v2_forward::primitive_desc(pooling_desc, *onednn_engine);
      auto pooling_primitive = dnnl::pooling_v2_forward(pooling_primitive_desc);
      auto workspace_mem =
          dnnl::memory(pooling_primitive_desc.workspace_desc(), *onednn_engine, indice_ptr);

      pooling_primitive.execute(
          *onednn_stream,
          {{DNNL_ARG_SRC, src_mem}, {DNNL_ARG_DST, dst_mem}, {DNNL_ARG_WORKSPACE, workspace_mem}});
    });
  }

  static void OneDnnpoolBackwardCompute(
      ep::Stream* stream, const DnnlDims& diff_dst_dims, const DnnlDims& diff_src_dims,
      const DnnlDims& kernel_dims, const DnnlDims& strides_dims, const DnnlDims& padding_dims_l,
      const DnnlDims& padding_dims_r, const DnnlDims& dilation, dnnl::memory::format_tag format,
      void* diff_dst, void* diff_src, void* workspace, dnnl::algorithm algorithm) {
    stream->As<ep::CpuStream>()->onednn_executor()->Launch([&](dnnl::engine* onednn_engine,
                                                               dnnl::stream* onednn_stream) {
      auto data_type = CppTypeToOneDnnDtype<T>();
      auto diff_dst_md = dnnl::memory::desc(diff_dst_dims, data_type, format);
      auto diff_src_md = dnnl::memory::desc(diff_src_dims, data_type, format);
      auto diff_dst_mem = dnnl::memory(diff_dst_md, *onednn_engine, diff_dst);
      auto diff_src_mem = dnnl::memory(diff_src_md, *onednn_engine, diff_src);

      auto pooling_back_desc =
          dnnl::pooling_v2_backward::desc(algorithm, diff_src_md, diff_dst_md, strides_dims,
                                          kernel_dims, dilation, padding_dims_l, padding_dims_r);
      // forward
      auto pooling_desc = dnnl::pooling_v2_forward::desc(
          dnnl::prop_kind::forward_training, algorithm, diff_src_md, diff_dst_md, strides_dims,
          kernel_dims, dilation, padding_dims_l, padding_dims_r);
      auto pooling_primitive_desc =
          dnnl::pooling_v2_forward::primitive_desc(pooling_desc, *onednn_engine);
      auto workspace_mem =
          dnnl::memory(pooling_primitive_desc.workspace_desc(), *onednn_engine, workspace);
      // Backward
      auto pooling_back_primitive_desc = dnnl::pooling_v2_backward::primitive_desc(
          pooling_back_desc, *onednn_engine, pooling_primitive_desc);
      auto pooling_primitive = dnnl::pooling_v2_backward(pooling_back_primitive_desc);
      pooling_primitive.execute(*onednn_stream, {{DNNL_ARG_DIFF_DST, diff_dst_mem},
                                                 {DNNL_ARG_DIFF_SRC, diff_src_mem},
                                                 {DNNL_ARG_WORKSPACE, workspace_mem}});
    });
  }
};

}  // namespace oneflow
#endif  // WITH_ONEDNN
#endif  // ONEFLOW_USER_KERNELS_ONEDNN_POOL_KERNEL_UTIL_H_