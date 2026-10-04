// Node job: install one node of the plan.
//
// The coordinator starts this job once every scheduled dependency of the node has
// finished, and each of those uploaded its artifact before returning, so those
// dependencies are restored from the shared pkgcache.
pipeline {
    // The plan's platform selects the pool: celer-agent-<platform>. A manual run has
    // to fill PLATFORM in: an empty one matches no agent and queues forever.
    agent { label "celer-agent-${params.PLATFORM}" }

    options {
        timeout(time: 4, unit: 'HOURS')
    }

    parameters {
        string(name: 'CELER_NODE', description: 'name@version from scheduled_nodes')
        string(name: 'PLATFORM', description: 'platform of the plan: the job runs on celer-agent-<platform>')
        string(name: 'PLAN_BUILD', defaultValue: '', description: 'celer-plan build that exported dag.json')
    }

    stages {
        stage('install') {
            steps {
                script {
                    // Archived by the celer-plan job; it also carries the pins for
                    // local_nodes. Take the plan of the build that scheduled this node: a
                    // newer plan from another run must not be handed to it. A manual run
                    // has no such build and takes the latest plan.
                    if (params.PLAN_BUILD) {
                        copyArtifacts projectName: 'celer-plan', selector: specific(params.PLAN_BUILD), filter: 'dag.json'
                    } else {
                        copyArtifacts projectName: 'celer-plan', selector: lastSuccessful(), filter: 'dag.json'
                    }
                }

                sh "celer install --dag=dag.json '${params.CELER_NODE}'"
            }
        }
    }
}
