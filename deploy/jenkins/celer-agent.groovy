// Node job: install one node of the master.

// Run command on either a POSIX or a Windows agent (sh vs bat).
def execute(String cmd) {
    if (isUnix()) {
        sh "${cmd}"
    } else {
        bat "${args}"
    }
}

// Clean tmp files except buildtrees to reuse repo sources.
def cleanWorkspace() {
    if (isUnix()) {
        sh '''
        rm -rf celer.toml
        rm -rf installed
        rm -rf packages
        rm -rf stripped
        rm -rf toolchain_file.cmake
        rm -rf environment.sh
        rm -rf dag.json
        '''
    } else {
        bat '''
        del /F /Q "celer.toml" 2>nul
        rmdir /S /Q "installed" 2>nul
        rmdir /S /Q "packages" 2>nul
        rmdir /S /Q "stripped" 2>nul
        del /F /Q "toolchain_file.cmake" 2>nul
        del /F /Q "environment.bat" 2>nul
        del /F /Q "dag.json" 2>nul
        '''
    }
}

pipeline {
    agent { label "celer-agent" }

    options {
        timeout(time: 4, unit: 'HOURS')
    }

    parameters {
        string(name: 'MASTER_BUILD', description: 'celer-master build that exported dag.json')
        string(name: 'CELER_PORT', description: 'name@version to build and pass from celer-master.groovy')
        string(name: 'CELER_PLATFORM', description: 'platform from target_nodes')
        string(name: 'CELER_PROJECT', description: 'project from target_nodes')
    }

    stages {
        stage('install') {
            steps {
                script {
                    currentBuild.description = 
                        "platform: ${params.CELER_PLATFORM} | " + 
                        "project: ${params.CELER_PROJECT} | " +
                        "port: ${params.CELER_PORT}"

                    copyArtifacts (
                        projectName: 'celer-master',
                        selector: specific(params.MASTER_BUILD),
                        filter: 'dag.json',
                    )
                }

                script {
                    cleanWorkspace() // Keep buildtrees for reuse repo sources.
                    
                    execute 'celer init --url=https://github.com/celer-pkg/test-conf.git'
                    execute "celer install --dag=dag.json ${params.CELER_PORT}"
                }
            }
        }
    }
}
