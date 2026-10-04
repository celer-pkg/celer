// Plan job: export the build plan, then run one job per scheduled node, each as
// soon as its own dependencies are done.
//
// There is no wave barrier: a node starts the moment its dependencies finished, so
// a slow branch never holds back a node that does not depend on it. A dependency
// that is only listed in `cached` is not waiting for anybody: pkgcache already has
// it.
pipeline {
    agent { label 'celer-plan' }
    
    options {
        timeout(time: 4, unit: 'HOURS')
    }

    stages {
        stage('export the plan') {
            steps {
                sh 'celer deploy --dag=dag.json'

                // Every node job runs on its own agent and needs the plan file.
                archiveArtifacts artifacts: 'dag.json', fingerprint: true
            }
        }

        stage('destribute the plan') {
            steps {
                script {
                    // The key is omitted when pkgcache already has every node, and an empty
                    // plan simply leaves nothing to schedule.
                    def plan = readJSON(file: 'dag.json')
                    def nodes = plan.scheduled_nodes ?: []
                    def byNameVersion = nodes.collectEntries { n -> [(n.name_version): n] }

                    // Every node installs on the platform of the plan, and reads the plan
                    // file of this build: a newer plan must never reach it.
                    def platform = plan.platform
                    def planBuild = env.BUILD_NUMBER

                    // A node starts once every one of its own scheduled dependencies is
                    // done. A cached dependency is not in the plan, so it never blocks.
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
                                    string(name: 'CELER_NODE', value: nameVersion),
                                    string(name: 'PLATFORM', value: platform),
                                    string(name: 'PLAN_BUILD', value: planBuild)
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
    }
}
