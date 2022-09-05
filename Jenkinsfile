pipeline {
    agent { label 'Master' }
    
    stages {
        stage('init') {
            echo 'init..'
            steps {
                git 'https://gitee.com/homqyy/hcore.git'
                
                sh 'git submodule init'
                sh 'git submodule update'
            }
        }
        stage('Build') {
            echo 'Building..'
            steps {
                git 'https://gitee.com/homqyy/hcore.git'
                
                sh 'bash -x ./build.sh -dr -u `id -u` -g `id -g` configure'
                sh 'bash -x ./build.sh -d -u `id -u` -g `id -g` compile'
            }
        }
        stage('Test') {
            steps {
                sh 'bash -x ./build.sh -d -u `id -u` -g `id -g` test'
            }
        }
        stage('pack') {
            steps {
                sh 'bash -x ./build.sh -d -u `id -u` -g `id -g` install'
            }
        }
        stage('Deploy') {
            steps {
                echo 'Deploying....'
            }
        }
    }
}
