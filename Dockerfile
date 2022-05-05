FROM centos:8

# update yum repos
RUN rm -f /etc/yum.repos.d/* \
        && cd /etc/yum.repos.d/ \
        && curl http://mirrors.aliyun.com/repo/Centos-8.repo > CentOS-Linux-BaseOS.repo \
        && sed -i 's/\$releasever/8-stream/g' CentOS-Linux-BaseOS.repo \
        && cd -

# install deps
RUN yum clean all \
    && yum makecache \
    && yum -y update --allowerasing \
    && yum -y group install "Development Tools" \
    && yum -y install cmake

LABEL cn.homqyy.docker.author="homqyy"
LABEL cn.homqyy.docker.email="yilupiaoxuewhq@163.com"
LABEL cn.homqyy.docker.title="hcore"
