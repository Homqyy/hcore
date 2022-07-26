#!/bin/sh
#
# build_on_docker.sh <build|pack> [user] [group]

set -e

action=$1
user=$2
group=$3

############################## Global Variable

PROJECT_DIR=`cd $( dirname $0 ); pwd`
DOCKER_FILE=$PROJECT_DIR/Dockerfile
DOCKER_BUILD_LABEL="cn.homqyy.docker.build=hcore"
WORK_DIR=/usr/src/hcore
BUILD_TOOL=
RANDOM_NAME="cn.homqyy.docker.build.hcore.$( date +%Y%m%d%H%m%S )"
BUILD_OPTIONS=

############################## Function

function usage
{
    msg=$1;

    echo "error: $msg";

    echo "build_on_docker.sh <build|upload> [user] [group]"
    exit 1;
}

function init
{
    if [ -n "$user" ]; then
        BUILD_OPTIONS="--build-arg USER=$user $BUILD_OPTIONS"
    fi

    if [ -n "$group" ]; then
        BUILD_OPTIONS="--build-arg GROUP=$group $BUILD_OPTIONS"
    fi

    if [ "$action" == "build" ]; then
        BUILD_TOOL="$WORK_DIR/build.sh"
    elif [ "$action" == "upload" ]; then
        BUILD_TOOL="$WORK_DIR/upload.sh"
    else
        usage "invalid action"
    fi
}

############################## Main

cd $PROJECT_DIR

init

image_id=`docker images -qf "label=$DOCKER_BUILD_LABEL"`

if [ -z "$image_id" ]; then
    image_id=`docker build $BUILD_OPTIONS -q -f $DOCKER_FILE -t $RANDOM_NAME --label $DOCKER_BUILD_LABEL $PROJECT_DIR`
fi

container_id=`docker ps -qf "label=$DOCKER_BUILD_LABEL"`

if [ -n "$container_id" ]; then
	# already running
	exit 0;
fi

container_id=`docker ps -qaf "label=$DOCKER_BUILD_LABEL"`

if [ -n "$container_id" ]; then
    echo "already running"
    exit 0
else
    docker run --rm -v $PROJECT_DIR:$WORK_DIR --name $RANDOM_NAME -w $WORK_DIR $image_id $BUILD_TOOL
fi
