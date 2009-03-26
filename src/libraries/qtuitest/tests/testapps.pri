# This file is included from the .pro file of any system test which needs
# the test apps in `testapps' installed.
# It causes the test apps to be compiled and installed before the test is run.

TESTAPP_PATH=/src/libraries/qtuitest/tests/testapps

qbuild {
    stub.TYPE=RULE
    stub.prerequisiteActions+=$$TESTAPP_PATH/testapp1/image
    stub.prerequisiteActions+=$$TESTAPP_PATH/testapp2/image
    stub.prerequisiteActions+=$$TESTAPP_PATH/testapp3/image
    stub.prerequisiteActions+=$$TESTAPP_PATH/testapp4/image
} else {
    testapps_install.commands*=@$$MAKE install -C $$fixpath($$QPEDIR$$TESTAPP_PATH)
    QMAKE_EXTRA_TARGETS*=testapps_install
    test.depends*=testapps_install
}

