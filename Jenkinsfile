pipeline {
    agent none
    stages {
        stage('All') {
            matrix {
                agent {
                    label "${platform}"
                }
                axes {
                    axis {
                        name 'platform'
                        values 'ubuntu20.04-x86_64', 'ubuntu18.04-x86_64', 'raspbian10-armv7l', 'windows10-x64', 'windows10-x86'
                    }
                }
                stages {
                    stage('Build') {
                        steps {
                            cmakeBuild buildDir: 'build', installation: 'InSearchPath', buildType: 'Release', cmakeArgs: '-G Ninja'
                            cmake workingDir: 'build', arguments: '--build .', installation: 'InSearchPath'
                            script {
                                if (platform == 'windows10-x64' || platform ==  'windows10-x86') {
                                    cmakeBuild buildDir: 'build_unicode', installation: 'InSearchPath', buildType: 'Release', cmakeArgs: '-G Ninja -DUNICODE=ON'
                                    cmake workingDir: 'build_unicode', arguments: '--build .', installation: 'InSearchPath'
                                }
                            }
                        }
                    }
                    stage('Test') {
                        steps {
                            ctest workingDir: 'build', installation: 'InSearchPath', arguments: '--output-on-failure'
                            script {
                                if (platform == 'windows10-x64' || platform ==  'windows10-x86') {
                                    ctest workingDir: 'build_unicode', installation: 'InSearchPath', arguments: '--output-on-failure'
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

