// Node job: install one node of the plan.
//
// The coordinator starts this job once every scheduled dependency of the node has
// finished, and each of those uploaded its artifact before returning, so those
// dependencies are restored from the shared pkgcache.
pipeline {
  agent { label 'celer-agent' }

  parameters {
    string(name: 'NODE', description: 'name@version from scheduled_nodes')
  }

  stages {
    stage('install') {
      steps {
        // Archived by the celer-plan job; also carries the pins for local_nodes.
        copyArtifacts projectName: 'celer-plan', selector: lastSuccessful(), filter: 'dag.json'

        sh "celer install --dag=dag.json '${params.NODE}'"
      }
    }
  }
}
