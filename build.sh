#!/bin/bash

#################################### global variable

G_PROJECT_DIR=`cd $( dirname $0 ); pwd`
G_TOOLS_DIR=$G_PROJECT_DIR/tools-dev
G_CONFIG_FILE=$G_PROJECT_DIR/.config
G_RELEASE_DIR=$G_PROJECT_DIR/release
G_DEBUG_DIR=$G_PROJECT_DIR/debug
G_PACKAGES_DIR=$G_PROJECT_DIR/packages

#################################### function

. $G_TOOLS_DIR/base_for_bash.func

function configure
{
    BUILD_NUMBER_FILE=$G_PROJECT_DIR/.build_number

    clean
    
    mkdir $G_RELEASE_DIR $G_DEBUG_DIR || return 1

    [ -e $BUILD_NUMBER_FILE ] || echo "1" > $BUILD_NUMBER_FILE

    $G_TOOLS_DIR/gen_cmakelists.pl \
        $G_PROJECT_DIR/CMakeLists.txt.in \
        $G_PROJECT_DIR/CMakeLists.txt \
        $G_PROJECT_DIR
}

function compile
{
    BUILD_NUMBER_FILE=$G_PROJECT_DIR/.build_number

    # increase build number

    build_n=`cat $BUILD_NUMBER_FILE`

    build_n=$(($build_n+1))

    echo $build_n > $BUILD_NUMBER_FILE

    # each 10 build to increase minor version at next build

    build_n=$(($build_n%10))
    if [ $build_n -eq 0 ]; then
        $G_TOOLS_DIR/gen_cmakelists.pl \
            --next_minor \
            -- \
            $G_PROJECT_DIR/CMakeLists.txt.in \
            $G_PROJECT_DIR/CMakeLists.txt \
            $G_PROJECT_DIR \
        || return 1
    fi

    # build debug

    cd $G_DEBUG_DIR

    cmake -DCMAKE_BUILD_TYPE=Debug .. \
        && cmake --build . \
        || error=1

    cd -

    [ $error ] && return 1

    # build release

    cd $G_RELEASE_DIR

    cmake -DCMAKE_BUILD_TYPE=Release .. \
        && cmake --build . \
        || error=1

    [ $error ] && return 1

    cd -
}

function test_case
{
    error=;

    cd $G_DEBUG_DIR || error=1

    ctest

    cd -

    [ $error ] && return 1

    cd $G_RELEASE_DIR || error=1

    ctest

    cd -

    [ $error ] && return 1
}


function install
{
    cpack --config CPackConfig-debug.cmake || return 1
    cpack --config CPackConfig-release.cmake || return 1
}

function clean
{
    [ -e $G_PACKAGES_DIR ] && rm -rf $G_PACKAGES_DIR
    [ -e $G_RELEASE_DIR ] && rm -rf $G_RELEASE_DIR
    [ -e $G_DEBUG_DIR ] && rm -rf $G_DEBUG_DIR
    [ -e $G_PROJECT_DIR/CMakeLists.txt ] && rm -f $G_PROJECT_DIR/CMakeLists.txt
}

function get_image_id
{
    project_label=$1
    rebuild=$2
    g_image_id=`docker images -qf label=$project_label` || return 1

    if [[ -n "$rebuild" && -n "$g_image_id" ]]; then
        docker rmi $g_image_id
	g_image_id=
    fi

    if [ -z "$g_image_id" ]; then
        # to build a image

        build_option=

        if [ -n "$user_id" ]; then
            build_option="--build-arg USER=$user_id $build_option"
        fi

        if [ -n "$group_id" ]; then
            build_option="--build-arg GROUP=$group_id $build_option"
        fi

        g_image_id=`docker build -qf ./Dockerfile $build_option .` || return 1
    fi

    echo "image_id=$g_image_id"

    return 0
}

function run_on_docker
{
    action=$1
    user_id=$2
    group_id=$3
    rebuild=$4

    project_label="cn.homqyy.docker.project=hcore"

    g_container_id=`docker ps -qf label=$project_label` || return 1

    if [ -z "$g_container_id" ]; then
        g_container_id=`docker ps -qaf label=$project_label` || return 1

        if [ -n "$g_container_id" ]; then
            # container was stoped, remove it ( don't arrived )
            docker rm $g_container_id
        fi

        # to run a new container
        if ! get_image_id "$project_label" "$rebuild"; then
            error_msg "fail to get image"
            return 1
        fi

        docker run --rm \
                -v "${G_PROJECT_DIR}:/workspace/hcore" \
                $g_image_id $action \
            || return 1
    else
        error_msg "container is running in id $g_container_id";
        return 1
    fi

    return 0;
}

function usage
{
    echo "Usage: $0 [-h] [-d] [-r] [-u <user_id] [-g <group_id] [configure|compile|test|install|clean]
  -h            : help
  -d            : build on docker

Valid options if '-d' is provided:
  -u <user_id>  : set user id in container
  -g <group_id> : set group id in container
  -r            : rebuild image" >& 2
}

#################################### main

docker=
user_id=
group_id=
rebuild_image=

while getopts :hdru:g: opt
do
    case $opt in
        h)
            usage
            exit 0;
            ;;
        d)
            docker=1
            ;;
        r)
            rebuild_image=1
            ;;
        u)
            user_id=$OPTARG
            ;;
        g)
            group_id=$OPTARG
            ;;
        '?')
            error_msg "$0: invalid option -$OPTARG"
            usage
            exit 1
            ;;
    esac
done

shift $(($OPTIND - 1))

if [[ $# -eq 1 ]]; then
    case $1 in
        configure)
            if [[ $docker -eq 1 ]]; then
                run_on_docker configure "$user_id" "$group_id" "$rebuild_image"
            else
                configure
            fi
            ;;
        compile)
            if [[ $docker -eq 1 ]]; then
                run_on_docker compile "$user_id" "$group_id" "$rebuild_image"
            else
                compile
            fi
            ;;
       test)
            if [[ $docker -eq 1 ]]; then
                run_on_docker test "$user_id" "$group_id" "$rebuild_image"
            else
                test_case
            fi
            ;;
        install)
            if [[ $docker -eq 1 ]]; then
                run_on_docker install "$user_id" "$group_id" "$rebuild_image"
            else
                install
            fi
            ;;
        clean)
            if [[ $docker -eq 1 ]]; then
                run_on_docker clean $user_id $group_id $rebuild_image
            else
                clean
            fi
            ;;
        *)
            usage
            exit 1
    esac
elif [[ $# -gt 1 ]]; then
    error_msg "arguments too much"
    usage
    exit 1
else
    # all
    if [[ $docker -eq 1 ]]; then
        run_on_docker "" "$user_id" "$group_id" "$rebuild_image"
    else
        configure && compile && test_case && install
    fi
fi
