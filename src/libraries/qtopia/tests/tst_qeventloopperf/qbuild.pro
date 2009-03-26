TEMPLATE=app
# Uncomment this to use benchlib for this test...
#CONFIG+=benchmark
CONFIG+=qtopia
!benchmark:CONFIG+=unittest
TARGET=tst_qeventloopperf
SOURCES=tst_qeventloopperf.cpp
