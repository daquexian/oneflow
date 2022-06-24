import oneflow as flow
# from oneflow.multiprocessing.shared_memory import SharedMemory
from multiprocessing.shared_memory import SharedMemory
from time import sleep
import atexit
import subprocess
import numpy as np
import sys

def f(shm):
    print('close and link')
    shm.close()
    shm.unlink()
    print(f'ls retcode: {subprocess.run(["ls", shm.name]).returncode}')

shm = SharedMemory(sys.argv[1], size=40)

atexit.register(f, shm)

sleep(8)

a = np.frombuffer(shm.buf)
print(a)

print(shm)

