#!/bin/sh

set -e


############################## Global Variable

PROJECT_DIR=`cd $( dirname $0 ); pwd`
DOCKER_FILE=$PROJECT_DIR/Dockerfile
DOCKER_BUILD_LABEL="cn.homqyy.docker.build=hcore"
WORK_DIR=/usr/src/hcore
BUILD_TOOL=/usr/src/hcore/build.sh
RANDOM_NAME="cn.homqyy.docker.build.hcore$( date +%Y%m%d%H%m%S )"

############################## Function


############################## Main

cd $PROJECT_DIR

image_id=`docker images -qf "label=$DOCKER_BUILD_LABEL"`

if [ -z "$image_id" ]; then
    image_id=`docker build -q -f $DOCKER_FILE -t $RANDOM_NAME --label $DOCKER_BUILD_LABEL $PROJECT_DIR`
fi

container_id=`docker ps -qf "label=$DOCKER_BUILD_LABEL"`

if [ -n "$container_id" ]; then
    docker start $container_id
else
    docker run -v $PROJECT_DIR:$WORK_DIR --name $RANDOM_NAME -w $WORK_DIR $image_id $BUILD_TOOL
fi