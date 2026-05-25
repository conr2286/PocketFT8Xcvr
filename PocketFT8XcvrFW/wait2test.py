# Workaround:  This script is a workaround for unit testing on MacOS executing on
# a MacBook Air M2 suffering an apparent race condition in which PlatformIO attempts
# to access the USB serial port before MacOS has finished enumerating its ports.
# The script implements a wait-a-bit solution that should work on any computer.
import time

Import("env")

print("wait2test");

if "test" in env.GetBuildType():
    env.AddPostAction("upload", lambda *_, **__: time.sleep(2))


