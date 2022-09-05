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

    post {
        failure {
            mail to: yilupiaoxuewhq@163.com, subject: 'The Pipeline failed for hcore project :('
        }
    }
}
