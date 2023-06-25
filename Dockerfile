FROM homqyy/dev_env_centos8

ARG USER=1000
ARG GROUP=1000

USER root

#
# install 'ripgrep' for 'todo-tree' extension of vscode
# install clang-format
RUN curl https://copr.fedorainfracloud.org/coprs/carlwgeorge/ripgrep/repo/epel-7/carlwgeorge-ripgrep-epel-7.repo \
        > /etc/yum.repos.d/carlwgeorge-ripgrep-epel-7.repo \
    && yum -y install ripgrep git-clang-format sudo

RUN groupadd -g $GROUP hcore \
    && useradd -g $GROUP -u $USER -p "" hcore

USER $USER:$GROUP

WORKDIR /workspace/hcore

LABEL cn.homqyy.docker.author="homqyy"
LABEL cn.homqyy.docker.email="yilupiaoxuewhq@163.com"
LABEL cn.homqyy.docker.project="hcore"

ENTRYPOINT [ "/bin/bash", "./build.sh" ]
