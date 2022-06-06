FROM homqyy/dev_env_centos8

ARG USER=1000
ARG GROUP=1000

# install 'ripgrep' for 'todo-tree' extension of vscode
RUN curl https://copr.fedorainfracloud.org/coprs/carlwgeorge/ripgrep/repo/epel-7/carlwgeorge-ripgrep-epel-7.repo \
        > /etc/yum.repos.d/carlwgeorge-ripgrep-epel-7.repo \
    && yum -y install ripgrep

USER $USER:$GROUP

LABEL cn.homqyy.docker.author="homqyy"
LABEL cn.homqyy.docker.email="yilupiaoxuewhq@163.com"
LABEL cn.homqyy.docker.title="hcore"
