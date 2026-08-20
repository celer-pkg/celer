# This file is used to manage the dependencies. It is
# used by gclient to determine what version of each dependency to check out, and
# where.
#
# For more information, please refer to the official documentation:
#   https://sites.google.com/a/chromium.org/dev/developers/how-tos/get-the-code
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.
#
# -----------------------------------------------------------------------------
# Rolling deps
# -----------------------------------------------------------------------------
# All repositories in this file are git-based, using Chromium git mirrors where
# necessary (e.g., a git mirror is used when the source project is SVN-based).
# To update the revision that Chromium pulls for a given dependency:
#
#  # Create and switch to a new branch
#  git new-branch depsroll
#  # Run roll-dep (provided by depot_tools) giving the dep's path and optionally
#  # a regex that will match the line in this file that contains the current
#  # revision. The script ALWAYS rolls the dependency to the latest revision
#  # in origin/master. The path for the dep should start with src/.
#  roll-dep src/third_party/foo_package/src foo_package.git
#  # You should now have a modified DEPS file; commit and upload as normal
#  git commit -aspv_he
#  git cl upload
#
# For more on the syntax and semantics of this file, see:
#   https://bit.ly/chromium-gclient-conditionals
#
# which is a bit incomplete but the best documentation we have at the
# moment.

# We expect all git dependencies specified in this file to be in sync with git
# submodules (gitlinks).
git_dependencies = 'SYNC'

use_relative_paths = True

vars = {
  'chromium_tsproxy_git': 'https://chromium.googlesource.com/external/github.com/catchpoint/WebPageTest.tsproxy.git',
  'chromium_webpagereplay_git': 'https://chromium.googlesource.com/webpagereplay',
  'hjson_js_git': 'https://chromium.googlesource.com/external/github.com/hjson/hjson-js',

  # This variable is overridden in Chromium's DEPS file.
  'build_with_chromium': False,

  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling tsproxy
  # and whatever else without interference from each other.
  'tsproxy_revision': '7915adec656341bfab173484e1e0ca661eea1627',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling webpagereplay
  # and whatever else without interference from each other.
  'webpagereplay_revision': '77a306d6ce62d22b7d05055c977e8534e29284c3',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling webpagereplay
  # and whatever else without interference from each other.
  'hjson_js_revision': '5734a70a17b94f12b59081aa6fdf966aac066b23',
}

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, contact chrome infrastructure team.
allowed_hosts = [
  'chromium.googlesource.com',
  'chromium-nodejs',
]

deps = {
  'third_party/hjson_js': Var('hjson_js_git') + '@' + Var('hjson_js_revision'),
  'third_party/node/linux': {
      'dep_type': 'gcs',
      'bucket': 'chromium-nodejs',
      'objects': [
          {
              'object_name': 'fa98c6432de572206bc5519f85e9c96bd518b039',
              'sha256sum': 'fb563633b5bfe2d4307075c54c6bb54664a3b5ec6bc811f5b15742720549007a',
              'size_bytes': 50288755,
              'generation': 1730835522207929,
              'output_file': 'node-linux-x64.tar.gz',
          },
      ],
  },
  'third_party/node/mac': {
      'dep_type': 'gcs',
      'condition': 'host_os == "mac"',
      'bucket': 'chromium-nodejs',
      'objects': [
          {
              'object_name': '4c8952a65a1ce7a2e4cff6db68f9b7454c46349f',
              'sha256sum': 'fadb4530fbe6e35ed298848c66102a0aa7d92974789e6222c4eadee26a381e7e',
              'size_bytes': 45672893,
              'generation': 1730835514382259,
              'output_file': 'node-darwin-x64.tar.gz',
          },
      ],
  },
  'third_party/node/mac_arm64': {
      'dep_type': 'gcs',
      'condition': 'host_os == "mac"',
      'bucket': 'chromium-nodejs',
      'objects': [
          {
              'object_name': '0886aa6a146cb5c213cb09b59ed1075982e4cb57',
              'sha256sum': 'd39e2d44d58bb89740b9aca1073959fc92edbdbbe810a5e48448e331cf72c196',
              'size_bytes': 44929037,
              'generation': 1730835518292126,
              'output_file': 'node-darwin-arm64.tar.gz',
          },
      ],
  },
  'third_party/node/win': {
      'dep_type': 'gcs',
      'condition': 'host_os == "win"',
      'bucket': 'chromium-nodejs',
      'objects': [
          {
              'object_name': '907d7e104e7389dc74cec7d32527c1db704b7f96',
              'sha256sum': '7447c4ece014aa41fb2ff866c993c708e5a8213a00913cc2ac5049ea3ffc230d',
              'size_bytes': 80511640,
              'generation': 1730835526374028,
              'output_file': 'node.exe',
          },
      ],
  },
  'third_party/tsproxy': Var('chromium_tsproxy_git') + '@' + Var('tsproxy_revision'),
  'third_party/webpagereplay': {
    'url': Var('chromium_webpagereplay_git') + '@' + Var('webpagereplay_revision'),
    'condition': 'not build_with_chromium',
  }
}

hooks = [
  {
    'name': 'vpython3_common',
    'pattern': '.',
    'action': [ 'vpython3',
                '-vpython-spec', '.vpython3',
                '-vpython-tool', 'install',
    ],
  },
]
