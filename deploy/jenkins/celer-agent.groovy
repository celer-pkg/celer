// Node job: install one node of the master.
//
// The coordinator starts this job once every scheduled dependency of the node has
// finished, and each of those uploaded its artifact before returning, so those
// dependencies are restored from the shared pkgcache.
pipeline {
    agent { label "celer-agent" }

    options {
        timeout(time: 4, unit: 'HOURS')
    }

    parameters {
        string(name: 'CELER_PORT', description: 'name@version from scheduled_nodes')
        string(name: 'CELER_PLATFORM', description: 'platform from scheduled_nodes')
        string(name: 'CELER_PROJECT', description: 'project from scheduled_nodes')
        string(name: 'MASTER_BUILD', defaultValue: '', description: 'celer-master build that exported dag.json')
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

                sh """
                #!/bin/bash
                set -e
                
                celer init --url=https://github.com/celer-pkg/test-conf.git
                celer install --dag=dag.json '${params.CELER_PORT}'
                """
            }
        }
    }
}
