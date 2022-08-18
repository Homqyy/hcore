#!/bin/sh

set -e

############################## Global Variable

PROJECT_DIR=`cd $( dirname $0 ); pwd`
TOOLS_DIR=$PROJECT_DIR/tools-dev
CONFIG_FILE=$PROJECT_DIR/.config
RELEASE_DIR=$PROJECT_DIR/release
DEBUG_DIR=$PROJECT_DIR/debug
PACKAGES_DIR=$PROJECT_DIR/packages
ACTION=$1

############################## Function

BUILD_NUMBER_FILE=$PROJECT_DIR/.build_number

function init
{
    if [ "$ACTION" == "clean" ]; then
        [ -e $PACKAGES_DIR ] && rm -rf $PACKAGES_DIR
        [ -e $RELEASE_DIR ] && rm -rf $RELEASE_DIR
        [ -e $DEBUG_DIR ] && rm -rf $DEBUG_DIR
        [ -e $PROJECT_DIR/CMakeLists.txt ] && rm -f $PROJECT_DIR/CMakeLists.txt
        
        mkdir $RELEASE_DIR $DEBUG_DIR
    fi

    # increase build number
    [ -e $BUILD_NUMBER_FILE ] || echo "1" > $BUILD_NUMBER_FILE
    build_n=`cat $BUILD_NUMBER_FILE`

    build_n=$(($build_n+1))

    echo $build_n > $BUILD_NUMBER_FILE

    # each 10 build to increase minor version at next build
    build_n=$(($build_n%10))
    if [ $build_n -eq 0 ]; then
        OPTIONS="--next_minor"
    fi

    $TOOLS_DIR/gen_cmakelists.pl \
        $OPTIONS \
        -- \
        $PROJECT_DIR/CMakeLists.txt.in \
        $PROJECT_DIR/CMakeLists.txt \
        $PROJECT_DIR
}

function build
{
    # build debug
    cd $DEBUG_DIR
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    ctest
    cd -

    # build release
    cd $RELEASE_DIR
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .
    ctest
    cd -
}

function build_done
{
    cpack --config MultiCPackConfig.cmake
}

############################## Main

cd $PROJECT_DIR
init
build
build_done