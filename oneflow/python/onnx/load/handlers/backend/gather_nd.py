"""
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
"""
import tensorflow as tf

from oneflow.python.onnx.load.handlers.backend_handler import BackendHandler
from oneflow.python.onnx.load.handlers.handler import onnx_op
from oneflow.python.onnx.load.handlers.handler import tf_func
from .gather_and_scatter_mixin import GatherAndScatterMixin


@onnx_op("GatherND")
@tf_func(tf.gather_nd)
class GatherND(GatherAndScatterMixin, BackendHandler):
    @classmethod
    def version_11(cls, node, **kwargs):
        data = kwargs["tensor_dict"][node.inputs[0]]
        indices = kwargs["tensor_dict"][node.inputs[1]]

        result = cls.chk_idx_out_of_bounds(data, indices)
        msg = "GatherND indices are out of bounds, please double check the indices and retry."
        with tf.control_dependencies(
            [tf.compat.v1.assert_equal(result, True, message=msg)]
        ):
            indices = cls.process_neg_idx(data, indices)
            return [
                cls.make_tensor_from_onnx_node(node, inputs=[data, indices], **kwargs)
            ]
