// Coordinator job: export the build plan, then run the scheduled nodes in
// dependency waves.
//
// A wave holds every node whose scheduled dependencies already ran; the nodes
// inside a wave are independent and start in parallel. A node whose dependency is
// only listed in `cached` is not waiting for anybody: pkgcache already has it.
pipeline {
  agent { label 'celer-coordinator' }

  stages {
    stage('export the plan') {
      steps {
        sh 'celer deploy --dag=dag.json'

        // Every node job runs on its own agent and needs the plan file.
        archiveArtifacts artifacts: 'dag.json', fingerprint: true
      }
    }

    stage('build the plan') {
      steps {
        script {
          def dag = readJSON file: 'dag.json'

          // The key is omitted when pkgcache already has every node.
          def nodes = dag.scheduled_nodes ?: []
          if (nodes.isEmpty()) {
            echo 'nothing to build: pkgcache already has every node'
          } else {
            def byName = nodes.collectEntries { n -> [(n.name_version): n] }
            def done = [] as Set
            def remaining = byName.keySet().toList() as List

            while (!remaining.isEmpty()) {
              def wave = remaining.findAll { name ->
                byName[name].dependencies.every { dep ->
                  !byName.containsKey(dep) || done.contains(dep)
                }
              }
              if (wave.isEmpty()) {
                error "dependency cycle in dag.json: ${remaining}"
              }

              def jobs = wave.collectEntries { name ->
                [(name): {
                  build job: 'celer-node', wait: true, propagate: true,
                    parameters: [string(name: 'NODE', value: name)]
                }]
              }
              parallel jobs

              done.addAll(wave)
              remaining.removeAll(wave)
            }
          }
        }
      }
    }
  }
}
