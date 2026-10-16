// Master job: export the build schedule (for the whole project with `deploy`, or a
// single port with `install`), then run one job per scheduled node, each as soon as
// its own dependencies are done.

// Run command on either a POSIX or a Windows agent (sh vs bat).
def run(String cmd) {
    if (isUnix()) {
        sh "${cmd}"
    } else {
        bat "${cmd}"
    }
}

// Build `celer <deploy|install> --<dagFlag>=dag.json [port]` for the configured mode:
// an empty CELER_PORT deploys the whole project, a port installs that single port.
def dagCommand(String dagFlag) {
    def cmd = "celer ${params.CELER_PORT ? 'install' : 'deploy'} ${dagFlag}=dag.json"
    if (params.CELER_PORT) {
        cmd += " '${params.CELER_PORT}'"
    }
    return cmd
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
    agent { label 'celer-master' }
    
    options {
        timeout(time: 4, unit: 'HOURS')
    }

    parameters {
        string(name: 'CELER_CONF_URL', defaultValue: 'https://github.com/celer-pkg/test-conf.git', description: 'The git repo url of celer workspace- [mandatory]', trim: true)
        string(name: 'CELER_CONF_BRANCH', defaultValue: 'master', description: 'The git branch of conf repo - [optional]', trim: true)
        string(name: 'CELER_PLATFORM', defaultValue: 'aarch64-linux-ubuntu-22.04-gcc-11.5.0', description: 'The platform of celer workspace - [mandatory]', trim: true)
        string(name: 'CELER_PROJECT', defaultValue: 'project_test_01', description: 'The project of celer workspace - [mandatory]', trim: true)
        string(name: 'CELER_PORT', defaultValue: '', description: 'A port (name@version) to install; leave empty to deploy the whole project - [optional]', trim: true)
        booleanParam(name: 'CELER_FORCE', defaultValue: false, description: 'Ignore pkgcache cache - [optional]')

        // ======== fs and minio backends are mutually exclusive ======== // 

        // Pkgcache solution: MINIO
        // string(name: 'CELER_PKGCACHE_MINIO_HOST', defaultValue: '', description: 'pkgcache: minio host url', trim: true)
        // string(name: 'CELER_PKGCACHE_ACCESS_KEY', defaultValue: '', description: 'pkgcache: minio access key', trim: true)
        // string(name: 'CELER_PKGCACHE_SECRET_KEY', defaultValue: '', description: 'pkgcache: minio secret key', trim: true)

        // Pkgcache solution: FS
        // string(name: 'CELER_PKGCACHE_FS_DIR', defaultValue: '', description: 'pkgcache: fs dir',  trim: true)
    }

    stages {
        stage('export the schedule dag') {
            steps {
                script {
                    // Validate the required parameters up front.
                    if (!params.CELER_CONF_URL) {
                        error 'no CELER_CONF_URL specified for celer workspace.'
                    }
                    if (!params.CELER_PLATFORM) {
                        error 'no CELER_PLATFORM specified for celer workspace.'
                    }
                    if (!params.CELER_PROJECT) {
                        error 'no CELER_PROJECT specified for celer workspace.'
                    }

                    cleanWorkspace() // Keep buildtrees for reuse repo sources.

                    // The conf branch defaults to master.
                    def confBranch = params.CELER_CONF_BRANCH ?: 'master'
                    run "celer init --url=${params.CELER_CONF_URL} --branch=${confBranch}"
                    run "celer configure --platform=${params.CELER_PLATFORM}"
                    run "celer configure --project=${params.CELER_PROJECT}"

                    if (params.CELER_PKGCACHE_MINIO_HOST && params.CELER_PKGCACHE_ACCESS_KEY && params.CELER_PKGCACHE_SECRET_KEY) {
                        run "celer configure " +
                            "--pkgcache-minio-host=${params.CELER_PKGCACHE_MINIO_HOST} " +
                            "--pkgcache-minio-access-key=${params.CELER_PKGCACHE_ACCESS_KEY} " +
                            "--pkgcache-minio-secret-key=${params.CELER_PKGCACHE_SECRET_KEY}"
                    } else if (params.CELER_PKGCACHE_FS_DIR) {
                        run "celer configure --pkgcache-fs-dir=${params.CELER_PKGCACHE_FS_DIR}"
                    }

                    def exportCmd = dagCommand('--export-dag')
                    if (params.CELER_FORCE) {
                        exportCmd += ' --force'
                    }
                    run exportCmd
                }

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
                    def targetNodes = dag.target_nodes ?: []
                    def byNameVersion = targetNodes.collectEntries { n -> [(n.name_version): n] }

                    // Every node installs on the platform of the schedule, and reads the
                    // schedule file of this build: a newer schedule must never reach it.
                    def platform = dag.platform
                    def project = dag.project
                    def masterBuild = env.BUILD_NUMBER

                    // A node starts once every one of its own scheduled dependencies is
                    // done. A cached dependency is not in the dag, so it never blocks.
                    def done = [:]

                    if (targetNodes.isEmpty()) {
                        echo 'nothing to build: pkgcache has already cached every port'
                        return
                    }

                    def branches = targetNodes.collectEntries { n ->
                        def nameVersion = n.name_version
                        def deps = n.dependencies.findAll { byNameVersion.containsKey(it) }
                        [
                            (nameVersion): {
                                waitUntil(quiet: true) { 
                                    deps.every { done[it] } 
                                }

                                // Any result other than success fails the branch, 
                                // regardless of how the node build ended, and failFast reacts to that failure.
                                def run = build(job: 'celer-agent', propagate: false, parameters: [
                                    string(name: 'MASTER_BUILD', value: masterBuild),
                                    string(name: 'CELER_PORT', value: nameVersion),
                                    string(name: 'CELER_PLATFORM', value: platform),
                                    string(name: 'CELER_PROJECT', value: project)
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

        stage('apply the schedule dag') {
            steps {
                script {
                    // Collects the artifacts of all nodes (deploy), or installs the
                    // requested port from them (install).
                    run dagCommand('--apply-dag')
                }
            }
        }
    }
}
