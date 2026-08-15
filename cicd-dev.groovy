node('linux') {
    stage ('Poll') {
      checkout([
        $class: 'GitSCM', branches: [[name: '*/main']], extensions: [],
        userRemoteConfigs: [[url: 'https://github.com/zopencommunity/uvport.git']]])
    }
    stage('Build') {
      build job: 'Port-Pipeline', parameters: [
        string(name: 'PORT_GITHUB_REPO', value: 'https://github.com/zopencommunity/uvport.git'),
        string(name: 'PORT_DESCRIPTION', value: 'An extremely fast Python package and project manager'),
        string(name: 'BUILD_LINE', value: 'DEV')
      ]
    }
}
