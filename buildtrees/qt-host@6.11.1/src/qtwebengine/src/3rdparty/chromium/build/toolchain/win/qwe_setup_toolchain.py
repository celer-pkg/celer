# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This is symplified version of setup_toolchains.py striped of
# depot tools specifics.

import errno
import os
import re
import subprocess
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir))
import gn_helpers

SCRIPT_DIR = os.path.dirname(__file__)

def _LoadEnvFromBat(args):
  """Given a bat command, runs it and returns env vars set by it."""
  env = {}
  args = args[:]
  args.extend(('&&', 'set'))
  popen = subprocess.Popen(
      args, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  variables, _ = popen.communicate()
  if popen.returncode != 0:
    raise Exception('"%s" failed with error %d' % (args, popen.returncode))
  output = variables.decode(errors='ignore')
  for line in output.splitlines():
      if re.match("^(.*)=(.*)$",line):
        var, setting = line.split('=', 1)
        env[var] = setting
  return env

def _LoadToolchainEnv(cpu, visual_studio_path, win_sdk_path):
  """Returns a dictionary with environment variables that must be set while
  running binaries from the toolchain (e.g. INCLUDE and PATH for cl.exe)."""
  # Check if we are running in the SDK command line environment and use
  # the setup script from the SDK if so. |cpu| should be either
  # 'x86' or 'x64' or 'arm' or 'arm64'.
  assert cpu in ('x86', 'x64', 'arm', 'arm64')
  script_path = os.path.normpath(os.path.join(visual_studio_path, 'VC/vcvarsall.bat'))
  if not os.path.exists(script_path):
    # vcvarsall.bat for VS 2017 fails if run after running vcvarsall.bat from
    # VS 2013 or VS 2015. Fix this by clearing the vsinstalldir environment
    # variable. Since vcvarsall.bat appends to the INCLUDE, LIB, and LIBPATH
    # environment variables we need to clear those to avoid getting double
    # entries when vcvarsall.bat has been run before gn gen. vcvarsall.bat
    # also adds to PATH, but there is no clean way of clearing that and it
    # doesn't seem to cause problems.
    if 'VSINSTALLDIR' in os.environ:
      del os.environ['VSINSTALLDIR']
    if 'INCLUDE' in os.environ:
      del os.environ['INCLUDE']
    if 'LIB' in os.environ:
      del os.environ['LIB']
    if 'LIBPATH' in os.environ:
      del os.environ['LIBPATH']
    other_path = os.path.normpath(os.path.join(visual_studio_path,
                                               'VC/Auxiliary/Build/vcvarsall.bat'))
    if not os.path.exists(other_path):
      raise Exception('%s is missing - make sure VC++ tools are installed.' %
                      script_path)
    script_path = other_path
  cpu_arg = "amd64"
  if (cpu != 'x64'):
    # x64 is default target CPU thus any other CPU requires a target set
    cpu_arg += '_' + cpu
  args = [script_path, cpu_arg, ]
  return _LoadEnvFromBat(args)

def _FormatAsEnvironmentBlock(envvar_dict):
  """Format as an 'environment block' directly suitable for CreateProcess.
  Briefly this is a list of key=value\0, terminated by an additional \0. See
  CreateProcess documentation for more details."""
  block = ''
  nul = '\0'
  for key, value in envvar_dict.items():
    block += key + '=' + value + nul
  block += nul
  return block

def main():
  if len(sys.argv) != 7:
    print('Usage setup_toolchain.py '
          '<visual studio path> <win sdk path> '
          '<runtime dirs> <target_os> <target_cpu> '
          '<environment block name|none>')
    sys.exit(2)

  visual_studio_path = sys.argv[1]
  win_sdk_path = sys.argv[2]
  runtime_dirs = sys.argv[3]
  target_os = sys.argv[4]
  target_cpu = sys.argv[5]
  environment_block_name = sys.argv[6]

  # Extract environment variables for subprocesses.
  env = _LoadToolchainEnv(target_cpu, visual_studio_path, win_sdk_path)

  def relflag(s):  # Make s relative to builddir when cwd and sdk on same drive.
    try:
      return os.path.relpath(s).replace('\\', '/')
    except ValueError:
      return s

  def q(s):  # Quote s if it contains spaces or other weird characters.
    return s if re.match(r'^[a-zA-Z0-9._/\\:-]*$', s) else '"' + s + '"'

  include = [p.replace('"', r'\"') for p in env['INCLUDE'].split(';') if p]
  include = list(map(relflag, include))
  include_imsvc = ['-imsvc' + i for i in include]

  env_block = _FormatAsEnvironmentBlock(env)
  with open(environment_block_name, 'w', encoding='utf8') as f:
    f.write(env_block)

  def ListToArgList(x):
    return f'[{", ".join(gn_helpers.ToGNString(i) for i in x)}]'

  print(f'include_flags_imsvc_list = {ListToArgList(include_imsvc)}')

if __name__ == '__main__':
  main()
