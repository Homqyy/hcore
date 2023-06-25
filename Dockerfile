FROM homqyy/dev_env_centos8

ARG USER=1000
ARG GROUP=1000

USER root

#
# install 'ripgrep' for 'todo-tree' extension of vscode
# install clang-format
# install doxygen
RUN curl https://copr.fedorainfracloud.org/coprs/carlwgeorge/ripgrep/repo/epel-7/carlwgeorge-ripgrep-epel-7.repo \
        > /etc/yum.repos.d/carlwgeorge-ripgrep-epel-7.repo \
    && yum -y install ripgrep git-clang-format sudo tree \
    && curl -o /tmp/doxygen-1.8.5-4.el7.x86_64.rpm http://mirror.centos.org/centos/7/os/x86_64/Packages/doxygen-1.8.5-4.el7.x86_64.rpm \
        && rpm -i /tmp/doxygen-1.8.5-4.el7.x86_64.rpm \
        && rm -f /tmp/doxygen-1.8.5-4.el7.x86_64.rpm

RUN groupadd -g $GROUP hcore \
    && useradd -g $GROUP -u $USER -p '' hcore \
    && echo "hcore ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER $USER:$GROUP

WORKDIR /workspace/hcore

LABEL cn.homqyy.docker.author="homqyy"
LABEL cn.homqyy.docker.email="yilupiaoxuewhq@163.com"
LABEL cn.homqyy.docker.project="hcore"

ENTRYPOINT [ "/bin/bash", "./build.sh" ]
