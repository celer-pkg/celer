import groovy.transform.Field

// Master job: export the build schedule (for the whole project with `deploy`, or ports
// with `install`), then run one job per scheduled node, each as soon as its own
// dependencies are done.

// Directory of the celer version CELER_VERSION downloaded; empty means the installed celer.
@Field String celerBinDir = ''

// Run command on either a POSIX or a Windows agent (sh vs bat). 
// Once this build pinned a celer version it wins over both $HOME/.local/bin and PATH.
def run(String cmd) {
    def prefix = celerBinDir ? celerBinDir + (isUnix() ? ':' : ';') : ''
    if (isUnix()) {
        sh "export PATH=\"${prefix}$HOME/.local/bin:\$PATH\"; ${cmd}"
    } else if (prefix) {
        bat "set \"PATH=${prefix}%PATH%\" && ${cmd}"
    } else {
        bat "${cmd}"
    }
}

// CELER_VERSION choices: the celer binaries already in pkgcache, newest first. `celer
// client list` reads the cache from CELER_PKGCACHE_*, so it also works before `init`.
def celerVersions() {
    try {
        def out = isUnix()
            ? sh(returnStdout: true, script: 'export PATH="$HOME/.local/bin:$PATH"; celer client list').trim()
            : bat(returnStdout: true, script: 'celer client list').trim()
        return out ? out.readLines() : []
    } catch (Throwable t) {
        echo "celer client list failed, CELER_VERSION stays empty: ${t.message}"
        return []
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

// Clean tmp files except buildtrees to reuse repo sources. celer-bins/ is where the celer
// binary selected by CELER_VERSION is downloaded; *.tar.gz is the previous packed artifact.
def cleanWorkspace() {
    if (isUnix()) {
        sh 'rm -rf celer.toml installed packages stripped toolchain_file.cmake environment.sh dag.json celer-bins *.tar.gz'
    } else {
        bat '''
        del /F /Q "celer.toml" "toolchain_file.cmake" "environment.bat" "dag.json" "celer-*.tar.gz" 2>nul
        rmdir /S /Q "installed" 2>nul
        rmdir /S /Q "packages" 2>nul
        rmdir /S /Q "stripped" 2>nul
        rmdir /S /Q "celer-bins" 2>nul
        '''
    }
}

// pkgcache solution about minio and fs. One of the two is mandatory:
// the master collects the distributed artifacts from that cache, 
// so a run without it cannot finish the build.
def configurePkgCache() {
    def minio = [env.CELER_PKGCACHE_MINIO_HOST, env.CELER_PKGCACHE_MINIO_ACCESS_KEY, env.CELER_PKGCACHE_MINIO_SECRET_KEY]
    def useMinio = minio.every()
    if (minio.any() && !useMinio) {
        error 'pkgcache: set all of CELER_PKGCACHE_MINIO_HOST, CELER_PKGCACHE_MINIO_ACCESS_KEY and CELER_PKGCACHE_MINIO_SECRET_KEY, or none of them.'
    }
    if (!useMinio && !env.CELER_PKGCACHE_FS_DIR) {
        error 'pkgcache: set CELER_PKGCACHE_FS_DIR, or CELER_PKGCACHE_MINIO_HOST with CELER_PKGCACHE_MINIO_ACCESS_KEY and CELER_PKGCACHE_MINIO_SECRET_KEY.'
    }

    def extra = env.CELER_PKGCACHE_READONLY ? " --pkgcache-readonly=${env.CELER_PKGCACHE_READONLY}" : ''
    if (env.CELER_PKGCACHE_KEEP_ARTIFACT_COUNT) {
        extra += " --pkgcache-keep-artifact-count=${env.CELER_PKGCACHE_KEEP_ARTIFACT_COUNT}"
    }
    if (!useMinio) {
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
// randomName is the DOM id the plugin's filter script looks up. `script` overrides the
// generated fixed-list script with a dynamic one (e.g. the conf repo's branches).
def choiceParam(String name, String description, List values, boolean filterable, Map script = null) {
    return [
        $class: 'ChoiceParameter',
        name: name,
        randomName: name.toLowerCase().replace('_', '-'),
        description: description,
        choiceType: 'PT_SINGLE_SELECT',
        filterable: filterable,
        filterLength: 1,
        script: script ?: [$class: 'GroovyScript', script: [classpath: [], sandbox: true, script: 'return ' + values.inspect()]]
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

    environment {
        // Force Jenkins plugin AnsiColor to render color.
        FORCE_COLOR = '1'
    }

    // agent {
    //     docker {
    //         label 'celer-master'
    //         image 'ubuntu:22.04'
    //         args '-u 1000:1000 ' +
    //             '-v /mnt/data/celer_buildtrees:/workspace/buildtrees ' +
    //             '-v /etc/localtime:/etc/localtime:ro ' +
    //             '-e HOME=/mnt/data/celer_home ' +
    //             '-e CELER_PKGCACHE_MINIO_HOST ' +
    //             '-e CELER_PKGCACHE_MINIO_ACCESS_KEY ' +
    //             '-e CELER_PKGCACHE_MINIO_SECRET_KEY ' +
    //             '-e CELER_PKGCACHE_FS_DIR ' +
    //             '-e CELER_PKGCACHE_READONLY'
    //     }
    // }
    
    options {
        timeout(time: 4, unit: 'HOURS')
        buildDiscarder(logRotator(numToKeepStr: '100', daysToKeepStr: '90', artifactNumToKeepStr: '30'))
        ansiColor('xterm')
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
                    def celerList = celerVersions() ?: ['']

                    properties([
                        parameters([
                            choiceParam('CELER_VERSION', 'The celer version this build runs, from pkgcache - [mandatory]', celerList, true),
                            string(name: 'CELER_CONF_URL', defaultValue: 'https://github.com/celer-pkg/test-conf.git', description: 'The git repo url of celer workspace - [mandatory]'),
                            choiceParam('CELER_CONF_BRANCH', 'The git branch of conf repo, defaulting to its HEAD branch - [optional]', [], true, [
                                $class: 'GroovyScript',
                                fallbackScript: [classpath: [], sandbox: true, script: 'return ["master", "SCRIPT FAILED: see Manage Jenkins > In-process Script Approval"]'],
                                script: [classpath: [], sandbox: false, script: confBranchScript(confUrl)]
                            ]),
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
                            booleanParam(name: 'CELER_FORCE', defaultValue: false, description: 'Ignore pkgcache cache - [optional]'),
                            booleanParam(name: 'CELER_PACK', defaultValue: true, description: 'Archive the artifact as a tarball for download - [optional]')
                        ])
                    ])
                }
            }
        }

        stage('export the schedule dag') {
            steps {
                script {
                    // Validate the required parameters up front.
                    def required = [
                        'CELER_CONF_URL'  : 'specified',
                        'CELER_PLATFORM'  : 'selected',
                        'CELER_PROJECT'   : 'selected',
                        'CELER_BUILD_TYPE': 'selected',
                        'CELER_MODE'      : 'selected'
                    ]
                    def missing = required.find { name, verb -> !params[name] }
                    if (missing) {
                        error "no ${missing.key} ${missing.value} for celer workspace."
                    }
                    if (params.CELER_MODE == 'install' && !portList(params.CELER_PORTS)) {
                        error 'no port given for install mode.'
                    }

                    cleanWorkspace() // Keep buildtrees for reuse repo sources.

                    // Run this build with the selected celer version. `client download` itself
                    // has to use the installed celer, so the pin is recorded afterwards.
                    if (params.CELER_VERSION) {
                        def dir = "${env.WORKSPACE}/celer-bins"
                        run "celer client download --version=${params.CELER_VERSION} --dest=${dir}/${isUnix() ? 'celer' : 'celer.exe'}"
                        celerBinDir = dir
                    }

                    // The conf branch defaults to master.
                    def confBranch = params.CELER_CONF_BRANCH ?: 'master'
                    run "celer init --url=${params.CELER_CONF_URL} --branch=${confBranch}"
                    run "celer update --ports-repo"
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
                    def scheduled = targetNodes*.name_version

                    if (targetNodes.isEmpty()) {
                        echo 'nothing to build: pkgcache has already cached every port'
                        return
                    }

                    // A node starts once its own scheduled dependencies is done.
                    def done = [:]
                    def branches = targetNodes.collectEntries { n ->
                        def nameVersion = n.name_version
                        def deps = n.dependencies.findAll { it in scheduled }
                        [
                            (nameVersion): {
                                waitUntil(quiet: true) {
                                    deps.every { done[it] }
                                }

                                // Any result other than success fails the branch,
                                // regardless of how the node build ended, and failFast reacts to that failure.
                                def run = build(job: 'celer-agent', propagate: false, parameters: [
                                    string(name: 'MASTER_BUILD', value: env.BUILD_NUMBER),
                                    string(name: 'CELER_PORT', value: nameVersion),
                                    string(name: 'CELER_PLATFORM', value: dag.platform),
                                    string(name: 'CELER_PROJECT', value: dag.project)
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

        stage('pack the artifact') {
            when { expression { params.CELER_PACK } }
            steps {
                script {
                    def dag = readJSON(file: 'dag.json')
                    def installedDir = "installed/${dag.platform}/${dag.project}/${dag.build_type}"
                    if (!fileExists(installedDir)) {
                        error "nothing to pack: ${installedDir} is missing"
                    }

                    def timestamp = new Date(currentBuild.startTimeInMillis).format('yyyyMMdd-HHmmss')
                    def tarball = "${dag.platform}-${dag.project}-${dag.build_type}-${env.BUILD_NUMBER}-${timestamp}.tar.gz"
                    run "tar -czf ${tarball} -C ${installedDir} ."
                }

                archiveArtifacts artifacts: '*.tar.gz', fingerprint: false
            }
        }
    }
}
