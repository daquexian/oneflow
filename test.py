import oneflow as flow
from oneflow.framework.check_point_v2 import copy_slice_by_slice

x = flow.tensor([1,2,3,4], sbp=flow.sbp.broadcast, placement=flow.placement('cpu', [0]))
y = flow.tensor([2,2,3,4], sbp=flow.sbp.broadcast, placement=flow.placement('cuda', [0]))

m = flow.nn.Conv2d(1, 1, 1)
m.to_global(sbp=flow.sbp.broadcast, placement=flow.placement('cuda', [0]))

m.load_state_dict(m.state_dict())

print(m.weight)
