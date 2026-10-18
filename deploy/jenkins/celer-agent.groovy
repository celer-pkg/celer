// Node job: install one node of the master.

// Run command on either a POSIX or a Windows agent (sh vs bat).
def execute(String cmd) {
    if (isUnix()) {
        sh "${cmd}"
    } else {
        bat "${cmd}"
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
    // agent {
    //     docker {
    //         image 'ubuntu:22.04'
    //         label 'celer-agent'
    //         args '-u 1000:1000 ' +
    //             '-v /mnt/data/celer_buildtrees:/workspace/buildtrees ' +
    //             '-e HOME=/mnt/data/celer_home ' +
    //             '-e CELER_PKGCACHE_MINIO_HOST ' +
    //             '-e CELER_PKGCACHE_MINIO_ACCESS_KEY ' +
    //             '-e CELER_PKGCACHE_MINIO_SECRET_KEY ' +
    //             '-e CELER_PKGCACHE_FS_DIR ' +
    //             '-e CELER_PKGCACHE_WRITABLE'
    //     }
    // }

    options {
        timeout(time: 4, unit: 'HOURS')
        buildDiscarder(logRotator(numToKeepStr: '500'))
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

                    // Clean first: it also drops a stale dag.json, so the copy must come after.
                    cleanWorkspace() // Keep buildtrees for reuse repo sources.

                    copyArtifacts (
                        projectName: 'celer-master',
                        selector: specific(params.MASTER_BUILD),
                        filter: 'dag.json',
                    )

                    execute "celer init --url=https://github.com/celer-pkg/test-conf.git"
                    execute "celer install --apply-dag=dag.json --prefer=source --force --clean-source ${params.CELER_PORT}"
                }
            }
        }
    }
}
