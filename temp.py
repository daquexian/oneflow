import oneflow as flow
from multiprocessing.shared_memory import SharedMemory
# from oneflow.multiprocessing.shared_memory import SharedMemory
from time import sleep
import atexit
import subprocess
import numpy as np

def f(shm):
    print('close and unlink')
    shm.close()
    shm.unlink()
    print(f'ls retcode: {subprocess.run(["ls", shm.name]).returncode}')

shm = SharedMemory(create=True, size=40)

# atexit.register(f, shm)
#
print(shm)
a = np.frombuffer(shm.buf)

a[:] = 1
print(a)

sleep(8)
