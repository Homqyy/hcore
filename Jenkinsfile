pipeline {
    agent { label 'Master' }
    
    stages {
        stage('init') {
            steps {
                git 'https://gitee.com/homqyy/hcore.git'
                
                sh 'git submodule init'
                sh 'git submodule update'
            }
        }
        stage('Build') {
            steps {
                git 'https://gitee.com/homqyy/hcore.git'
                
                sh 'bash -x ./build_on_docker.sh build `id -u` `id -g`'
            }
        }
        stage('Test') {
            steps {
                echo 'Testing..'
            }
        }
        stage('Deploy') {
            steps {
                echo 'Deploying....'
            }
        }
    }
}