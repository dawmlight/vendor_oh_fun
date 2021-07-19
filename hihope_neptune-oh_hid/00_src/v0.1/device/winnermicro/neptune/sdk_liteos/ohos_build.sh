#!/bin/bash
#set -e
product="w800"

CUR_DIR=$(dirname ${BASH_SOURCE[0]})
IMG_DIR=${CUR_DIR}/bin/${product}
OHOS_OUT=${CUR_DIR}/../../../../out/neptune/wifiiot_neptune
export OHOS_LIB=${OHOS_OUT}/${product}/libs
#export OHOS_LIB=${CUR_DIR}/bin/build/ohos/lib
#echo "OHOS_OUT == $OHOS_OUT, OHOS_LIB == $OHOS_LIB"

#rm -rf ${OHOS_LIB}
#mkdir -pv ${OHOS_LIB}
#find ${OHOS_OUT}/${product}/libs/ -name '*.a' -exec cp "{}" ${OHOS_LIB} \;

# export USE_DEMO_APP=true

make clean && make lib && make
exitcode=$?

if [ $exitcode -eq 0 ]; then
    echo "Build success!"
    if [ ! -d $OHOS_OUT ]; then
        mkdir $OHOS_OUT
    fi
    cp $IMG_DIR/* $OHOS_OUT/ -rf
else
    echo "Build failed!"
fi

exit $exitcode
