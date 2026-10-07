// Master job: export the build schedule, then run one job per scheduled node, each as
// soon as its own dependencies are done.
//
// There is no wave barrier: a node starts the moment its dependencies finished, so
// a slow branch never holds back a node that does not depend on it. A dependency
// that is only listed in `cached` is not waiting for anybody: pkgcache already has
// it.
pipeline {
    agent { label 'celer-master' }
    
    options {
        timeout(time: 4, unit: 'HOURS')
    }

    parameters {
        string(name: 'CELER_PLATFORM', description: 'platform to configure')
        string(name: 'CELER_PROJECT', description: 'project to configure')
    }

    stages {
        stage('export the schedule dag') {
            steps {
                sh """
                #!/bin/bash
                set -e

                if [ -z "${params.CELER_PLATFORM}" ]; then
                    echo "error: no CELER_PLATFORM specified for celer workspace."
                    exit 1
                fi

                if [ -z "${params.CELER_PROJECT}" ]; then
                    echo "error: no CELER_PROJECT specified for celer workspace."
                    exit 1
                fi
                
                celer init --url=https://github.com/celer-pkg/test-conf.git
                celer configure --platform=${params.CELER_PLATFORM}
                celer configure --project=${params.CELER_PROJECT}
                celer deploy --export-dag=dag.json
                """

                // Every node job runs on its own agent and needs the dag json file.
                archiveArtifacts artifacts: 'dag.json', fingerprint: true
            }
        }

        stage('distribute the schedule') {
            steps {
                script {
                    // The key is omitted when pkgcache already has every node, and an empty
                    // dag simply leaves nothing to schedule.
                    def dag = readJSON(file: 'dag.json')
                    def nodes = dag.scheduled_nodes ?: []
                    def byNameVersion = nodes.collectEntries { n -> [(n.name_version): n] }

                    // Every node installs on the platform of the schedule, and reads the
                    // schedule file of this build: a newer schedule must never reach it.
                    def platform = dag.platform
                    def project = dag.project
                    def masterBuild = env.BUILD_NUMBER

                    // A node starts once every one of its own scheduled dependencies is
                    // done. A cached dependency is not in the dag, so it never blocks.
                    def done = [:]

                    if (nodes.isEmpty()) {
                        echo 'nothing to build: pkgcache has already cached every port'
                        return
                    }

                    def branches = nodes.collectEntries { n ->
                        def nameVersion = n.name_version
                        def deps = n.dependencies.findAll { byNameVersion.containsKey(it) }
                        [
                            (nameVersion): {
                                waitUntil(quiet: true) { 
                                    deps.every { done[it] } 
                                }

                                // Anything but success fails the branch, whatever the node
                                // build ended as, so failFast reacts to it.
                                def run = build(job: 'celer-agent', propagate: false, parameters: [
                                    string(name: 'CELER_PORT', value: nameVersion),
                                    string(name: 'CELER_PLATFORM', value: platform),
                                    string(name: 'CELER_PROJECT', value: project),
                                    string(name: 'MASTER_BUILD', value: masterBuild)
                                ])
                                if (run.result != 'SUCCESS') {
                                    error "node ${nameVersion} ended ${run.result}"
                                }
                                done[nameVersion] = true
                            }
                        ]
                    }

                    // One node that fails or times out stops every other node: the
                    // branches still waiting are terminated, and each branch inside its
                    // build step aborts the celer-agent build it started.
                    branches.failFast = true
                    parallel branches
                }
            }
        }

        stage('deploy with dag') {
            steps {
                // Deploy with dag collects the artifacts of every node of the schedule.
                sh 'celer deploy --apply-dag=dag.json'
            }
        }
    }
}
