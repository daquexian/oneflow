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
import unittest
from collections import OrderedDict
import numpy as np

import oneflow as flow
import oneflow.unittest
import torch as ori_torch

from oneflow.test_utils.automated_test_util import *


@flow.unittest.skip_unless_1n1d()
class TestCumOp(flow.unittest.TestCase):
    @autotest(n=5, check_graph=True)
    def test_cumsum(test_case):
        device = random_device()
        x = random_tensor().to(device)
        dim = random(0, x.ndim.pytorch).to(int)
        z = torch.cumsum(x, dim)
        return z

    @autotest(n=5, check_graph=True)
    def test_cumprod(test_case):
        device = random_device()
        x = random_tensor().to(device)
        dim = random(0, x.ndim.pytorch).to(int)
        y = torch.cumprod(x, dim)
        return y

    def test_cumop_with_dtype(test_case):
        x = flow.tensor([2, 3, 4])
        cumsum_res = flow.cumsum(x, dim=0, dtype=flow.float)
        cumprod_res = flow.cumprod(x, dim=0, dtype=flow.float)
        test_case.assertEqual(cumsum_res.dtype, flow.float)
        test_case.assertEqual(cumprod_res.dtype, flow.float)

    @autotest(n=5, check_graph=True)
    def test_cumsum(test_case):
        device = random_device()
        x = random_tensor().to(device)
        dim = random(0, x.ndim.pytorch).to(int)
        y = x.cumsum(dim)
        return y

    @autotest(n=5, check_graph=True)
    def test_cumprod_with_user_dy(test_case):
        device = random_device()
        x = random_tensor().to(device)
        dim = random(0, x.ndim.pytorch).to(int)
        y = torch.cumprod(x, dim)
        z = y * 2
        return z

    def test_cumprod_with_zero(test_case):
        np_arr = np.ones((5, 5))
        np_arr_grad = np_arr
        np_arr[2][3] = 0
        np_arr[4][3] = 0
        of_tensor = flow.tensor(np_arr, dtype=flow.float, requires_grad=True)
        of_res = of_tensor.cumprod(dim=0)
        of_res.backward(flow.tensor(np_arr_grad, dtype=flow.float))

        torch_tensor = ori_torch.tensor(
            np_arr, dtype=ori_torch.float, requires_grad=True
        )
        torch_res = torch_tensor.cumprod(dim=0)
        torch_res.backward(ori_torch.tensor(np_arr_grad, dtype=ori_torch.float))
        test_case.assertTrue(
            np.allclose(
                of_tensor.grad.numpy(),
                torch_tensor.grad.numpy(),
                rtol=0.0001,
                atol=1e-05,
            )
        )


if __name__ == "__main__":
    unittest.main()
