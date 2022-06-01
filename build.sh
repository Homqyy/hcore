#!/bin/sh

set -e

############################## Global Variable

PROJECT_DIR=`cd $( dirname $0 ); pwd`
TOOLS_DIR=$PROJECT_DIR/tools
CONFIG_FILE=$PROJECT_DIR/.config
RELEASE_DIR=$PROJECT_DIR/release
DEBUG_DIR=$PROJECT_DIR/debug
PACKAGES_DIR=$PROJECT_DIR/packages

############################## Function

function init
{
    [ -e $PACKAGES_DIR ] && rm -rf $PACKAGES_DIR
    [ -e $RELEASE_DIR ] && rm -rf $RELEASE_DIR
    [ -e $DEBUG_DIR ] && rm -rf $DEBUG_DIR
    [ -e $PROJECT_DIR/CMakeLists.txt ] && rm -f $PROJECT_DIR/CMakeLists.txt
    
    mkdir $RELEASE_DIR $DEBUG_DIR

    $TOOLS_DIR/gen_cmakelists.pl \
        $PROJECT_DIR/CMakeLists.txt.in \
        $PROJECT_DIR/CMakeLists.txt \
        $PROJECT_DIR
}

function build
{

    # build release
    cd $RELEASE_DIR
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .
    cd -

    # build debug
    cd $DEBUG_DIR
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
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