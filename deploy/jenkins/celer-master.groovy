// Master job: export the build schedule (for the whole project with `deploy`, or ports
// with `install`), then run one job per scheduled node, each as soon as its own
// dependencies are done.

// Run command on either a POSIX or a Windows agent (sh vs bat).
def run(String cmd) {
    if (isUnix()) {
        sh "${cmd}"
    } else {
        bat "${cmd}"
    }
}

// Ports of CELER_PORTS: one or more `name@version`, space or comma separated.
def portList(String value) {
    return (value ?: '').split(/[,\s]+/).findAll { it }.unique()
}

// Build `celer <deploy|install> --<dagFlag>=dag.json [ports...]` for the configured mode.
def dagCommand(String dagFlag) {
    def cmd = "celer ${params.CELER_MODE} ${dagFlag}=dag.json"
    def ports = portList(params.CELER_PORTS)
    if (params.CELER_MODE == 'install' && ports) {
        cmd += ' ' + ports.collect { "'${it}'" }.join(' ')
    }
    return cmd
}

// Clean tmp files except buildtrees to reuse repo sources.
def cleanWorkspace() {
    if (isUnix()) {
        sh 'rm -rf celer.toml installed packages stripped toolchain_file.cmake environment.sh dag.json'
    } else {
        bat '''
        del /F /Q "celer.toml" "toolchain_file.cmake" "environment.bat" "dag.json" 2>nul
        rmdir /S /Q "installed" 2>nul
        rmdir /S /Q "packages" 2>nul
        rmdir /S /Q "stripped" 2>nul
        '''
    }
}

// pkgcache solution about minio and fs. One of the two is mandatory:
// the master collects the distributed artifacts from that cache, 
// so a run without it cannot finish the build.
def configurePkgCache() {
    def minio = [env.CELER_PKGCACHE_MINIO_HOST, env.CELER_PKGCACHE_MINIO_ACCESS_KEY, env.CELER_PKGCACHE_MINIO_SECRET_KEY]
    if (minio.any() && !minio.every()) {
        error 'pkgcache: set all of CELER_PKGCACHE_MINIO_HOST, _ACCESS_KEY and _SECRET_KEY, or none of them.'
    }
    if (!minio.every() && !env.CELER_PKGCACHE_FS_DIR) {
        error 'pkgcache: set CELER_PKGCACHE_FS_DIR, or CELER_PKGCACHE_MINIO_HOST with _ACCESS_KEY and _SECRET_KEY.'
    }

    def extra = env.CELER_PKGCACHE_WRITABLE ? " --pkgcache-writable=${env.CELER_PKGCACHE_WRITABLE}" : ''
    if (!minio.every()) {
        run "celer configure --pkgcache-fs-dir=${env.CELER_PKGCACHE_FS_DIR}${extra}"
        return
    }

    if (env.CELER_PKGCACHE_FS_DIR) {
        echo 'pkgcache: both minio and fs are set, using minio'
    }
    run "celer configure " +
        "--pkgcache-minio-host=${env.CELER_PKGCACHE_MINIO_HOST} " +
        "--pkgcache-minio-access-key=${env.CELER_PKGCACHE_MINIO_ACCESS_KEY} " +
        "--pkgcache-minio-secret-key=${env.CELER_PKGCACHE_MINIO_SECRET_KEY}${extra}"
}

// CELER_PLATFORM / CELER_PROJECT choices: the conf repo's platforms/<name>.toml and
// projects/<name>.toml, as the previous build left them in the workspace.
def confChoices(String kind) {
    return findFiles(glob: "conf/${kind}/*.toml")
        .collect { it.name.replaceAll(/\.toml$/, '') }
        .sort()
}

// Active Choices dropdown of a fixed list: a plain `choice` parameter cannot search.
// randomName is the DOM id the plugin's filter script looks up.
def choiceParam(String name, String description, List values, boolean filterable) {
    return [
        $class: 'ChoiceParameter',
        name: name,
        randomName: name.toLowerCase().replace('_', '-'),
        description: description,
        choiceType: 'PT_SINGLE_SELECT',
        filterable: filterable,
        filterLength: 1,
        script: [$class: 'GroovyScript', script: [classpath: [], sandbox: true, script: 'return ' + values.inspect()]]
    ]
}

// `name@version` of a .../<name>/<version>/port.toml file.
def portNameOf(def file) {
    def path = file.path.replace('\\', '/').split('/')
    return path[-3] + '@' + path[-2]
}

// CELER_PORTS candidates per project: its own ports first, then the global ones.
def confPorts() {
    def global = findFiles(glob: 'ports/*/*/*/port.toml').collect { portNameOf(it) }
    def byProject = [:]
    findFiles(glob: 'conf/projects/**/port.toml').each { f ->
        // conf/projects/<project>/[ports/]<name>/<version>/port.toml
        def path = f.path.replace('\\', '/').split('/')
        if (path.length >= 6 && path[2]) {
            byProject[path[2]] = (byProject[path[2]] ?: []) + portNameOf(f)
        }
    }
    return [global: global.unique().sort(),
            projects: byProject.collectEntries { k, v -> [(k): v.unique().sort()] }]
}

// Active Choices script of CELER_PORTS: the project's own ports plus the global ones, and
// nothing to pick while deploying. Reactive, so the referenced values arrive on the Ajax
// update rather than on the first render.
def confPortScript(Map ports) {
    return """
try {
    def ports = ${ports.inspect()}
    if (CELER_MODE != 'install') {
        return ['']
    }
    return ((ports['projects'][CELER_PROJECT] ?: []) + ports['global']).unique()
} catch (MissingPropertyException e) {
    return []
} catch (Throwable t) {
    return ['ERROR ' + t.class.name + ': ' + t.message]
}
"""
}

// Active Choices script behind CELER_PORTS_VISIBILITY: while deploying the ports choice has
// nothing to offer, so its row is hidden instead of being shown empty. Hiding is presentation
// and stays in CSS: :has() follows the mode select by itself, which matters because the plugin
// replaces this HTML through innerHTML on every Ajax update and would not run a script.
// Without :has() support the rule is ignored and the row simply stays visible.
def confPortsVisibilityScript() {
    return """
return '''
<style>
/* Nothing to pick while deploying: hide the whole row, not just the field. */
:root:has(#celer-mode option:checked:not([value='install'])) :is(.jenkins-form-item, tr):has(#celer-ports),
/* ... and the row of the parameter that carries this snippet. */
:is(.jenkins-form-item, tr):has(.hidden_uno_choice_parameter) {
    display: none;
}
</style>
'''
"""
}

// Active Choices script behind CELER_CONF_BRANCH: the conf repo's branches, HEAD branch
// first (Jenkins takes the first entry as the default).
def confBranchScript(String confUrl) {
    def safeUrl = confUrl.replace("'", '')
    return """

// Any throw here makes Active Choices use its fallbackScript, so failures are returned
// as list entries instead of being swallowed.
try {
    def url = '${safeUrl}'

    // No shell, no credential prompt for a private repo, and one merged stream: reading
    // stdout and stderr separately throws IOException: Stream Closed.
    def git = { List args ->
        def builder = new ProcessBuilder(['git', '-c', 'credential.helper=', '-c', 'core.askPass='] + args)
        builder.redirectErrorStream(true)
        def progress = builder.start()
        def out = progress.inputStream.text
        progress.waitFor()
        return out.readLines()
    }

    def lines = git(['ls-remote', '--symref', url, 'HEAD']) + git(['ls-remote', '--heads', url])

    def refLine = lines.find { it.startsWith('ref: ') }
    def defBranch = refLine ? refLine.split()[1].replace('refs/heads/', '') : ''
    def heads = lines.findAll { it.contains('refs/heads/') }.collect { it.split('refs/heads/')[1].trim() }

    if (!heads) {
        return ['ERROR: no branch read from ' + url + ' -> ' + lines.take(3).join(' | ')]
    }
    return ([defBranch] + heads.findAll { it != defBranch }).findAll { it }.unique()
} catch (Throwable t) {
    return ['ERROR ' + t.class.name + ': ' + t.message]
}
"""
}

pipeline {
    agent { label 'celer-master' }
    // agent {
    //     docker {
    //         label 'celer-master'
    //         image 'ubuntu:22.04'
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
    }

    stages {
        // Active Choices cannot be declared in `parameters {}`, so register all of them
        // here; they show up from the next build on.
        stage('register parameters') {
            steps {
                script {
                    def confUrl = params.CELER_CONF_URL ?: 'https://github.com/celer-pkg/test-conf.git'
                    def platforms = confChoices('platforms') ?: [params.CELER_PLATFORM ?: 'aarch64-linux-ubuntu-22.04-gcc-11.5.0']
                    def projects = confChoices('projects') ?: [params.CELER_PROJECT ?: 'project_test_01']
                    def ports = confPorts()

                    properties([
                        parameters([
                            string(name: 'CELER_CONF_URL', defaultValue: 'https://github.com/celer-pkg/test-conf.git', description: 'The git repo url of celer workspace - [mandatory]'),
                            [
                                $class: 'ChoiceParameter',
                                name: 'CELER_CONF_BRANCH',
                                randomName: 'celer-conf-branch',
                                description: 'The git branch of conf repo, defaulting to its HEAD branch - [optional]',
                                choiceType: 'PT_SINGLE_SELECT',
                                filterable: true,
                                filterLength: 1,
                                script: [
                                    $class: 'GroovyScript',
                                    fallbackScript: [classpath: [], sandbox: true, script: 'return ["master", "SCRIPT FAILED: see Manage Jenkins > In-process Script Approval"]'],
                                    script: [classpath: [], sandbox: false, script: confBranchScript(confUrl)]
                                ]
                            ],
                            choiceParam('CELER_PLATFORM', 'The platform of celer workspace, from the conf repo - [mandatory]', platforms, true),
                            choiceParam('CELER_PROJECT', 'The project of celer workspace, from the conf repo - [mandatory]', projects, true),
                            choiceParam('CELER_BUILD_TYPE', 'The CMake build type - [mandatory]', ['Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel'], false),
                            choiceParam('CELER_MODE', 'deploy the whole project, or install the selected ports - [mandatory]', ['deploy', 'install'], false),
                            [
                                $class: 'CascadeChoiceParameter',
                                name: 'CELER_PORTS',
                                randomName: 'celer-ports',
                                description: 'Ports (name@version) to install; ignored when deploying - [optional]',
                                choiceType: 'PT_MULTI_SELECT',
                                referencedParameters: 'CELER_MODE,CELER_PROJECT',
                                filterable: true,
                                filterLength: 1,
                                script: [$class: 'GroovyScript', script: [classpath: [], sandbox: false, script: confPortScript(ports)]]
                            ],
                            // Carries the snippet that hides CELER_PORTS while deploying; it has
                            // no value of its own and its own row is hidden by the plugin.
                            [
                                $class: 'DynamicReferenceParameter',
                                name: 'CELER_PORTS_VISIBILITY',
                                description: 'Hides the ports choice while deploying - [internal]',
                                choiceType: 'ET_FORMATTED_HIDDEN_HTML',
                                referencedParameters: 'CELER_MODE',
                                omitValueField: true,
                                script: [$class: 'GroovyScript', script: [classpath: [], sandbox: false, script: confPortsVisibilityScript()]]
                            ],
                            booleanParam(name: 'CELER_FORCE', defaultValue: false, description: 'Ignore pkgcache cache - [optional]')
                        ])
                    ])
                }
            }
        }

        stage('export the schedule dag') {
            steps {
                script {
                    // Validate the required parameters up front.
                    if (!params.CELER_CONF_URL) {
                        error 'no CELER_CONF_URL specified for celer workspace.'
                    }
                    if (!params.CELER_PLATFORM) {
                        error 'no CELER_PLATFORM selected for celer workspace.'
                    }
                    if (!params.CELER_PROJECT) {
                        error 'no CELER_PROJECT selected for celer workspace.'
                    }
                    if (!params.CELER_BUILD_TYPE) {
                        error 'no CELER_BUILD_TYPE selected for celer workspace.'
                    }
                    if (!params.CELER_MODE) {
                        error 'no CELER_MODE selected for celer workspace.'
                    }
                    if (params.CELER_MODE == 'install' && !portList(params.CELER_PORTS)) {
                        error 'no port given for install mode.'
                    }

                    cleanWorkspace() // Keep buildtrees for reuse repo sources.

                    // The conf branch defaults to master.
                    def confBranch = params.CELER_CONF_BRANCH ?: 'master'
                    run "celer init --url=${params.CELER_CONF_URL} --branch=${confBranch}"
                    run "celer configure --platform=${params.CELER_PLATFORM}"
                    run "celer configure --project=${params.CELER_PROJECT}"
                    run "celer configure --build-type=${params.CELER_BUILD_TYPE}"

                    // pkgcache solution from the environment: minio and fs are mutually
                    // exclusive, and one of them is required to collect the artifacts.
                    configurePkgCache()

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
                    // Collects the artifacts of all nodes (deploy), or installs the requested ports from them (install). 
                    // This side never builds: a node missing from pkgcache fails the stage instead of being rebuilt here.
                    def applyCmd = dagCommand('--apply-dag')
                    if (params.CELER_MODE == 'install') {
                        applyCmd += ' --prefer=pkgcache'
                    }
                    run applyCmd
                }
            }
        }
    }
}
