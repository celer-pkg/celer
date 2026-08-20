# Create an SBOM document for target

# Usage: see sbom.py --help

import os
import sys
import json
import argparse
import dataclasses
import licenses as license_tools
from typing import Any, DefaultDict
import spdx_writer
import pathlib
import subprocess
import datetime
import logging

logger = logging.getLogger(__name__)

# <webengine-root>/src/3rdparty/chromium
ROOT = license_tools._REPOSITORY_ROOT

# Field name used in README.chromium files -> SPDX JSON schema name
CHROMIUM_TO_SPDX_KEY = {
    'Version': 'versionInfo',
    'URL': 'homepage',
    'Source Info': 'sourceInfo',
    'Comment': 'comment',
}

# third_party/README.chromium.template says to use the string
# "This is the canonical public repository." to mark projects that don't
# have a homepage. Not all READMEs follow the text exactly, so just
# match against the first words.
CANONICAL_HOMEPAGE_STRING = "This is the canonical"

# Packages that have bad homepages that don't start with CANONICAL_HOMEPAGE_STRING
PACKAGES_TO_CLEAN_BAD_URL = [
    'psm (private set membership) client side',
]

# Some packages don't have a license file, but their license is a known SPDX
# identifier so we can still use it.
OVERRIDE_LICENSE_FILE_METADATA_KEY = "Override License File"
PACKAGES_TO_OVERRIDE_LICENSE_FILE_WITH_ID = [
    'libdrm',
    'mini_chromium',
    'temporal_rs',
    'temporal_capi',
    'timezone_provider',
    'unicode-ident',
]

# Command to find the first "Baseline" git commit in src/3rdparty
FIND_BASELINE_COMMIT_GIT_COMMAND = \
  ['git', 'rev-list', '-n1', '--first-parent', '--grep=^BASELINE: Update Chromium', 'HEAD', '--']

# Hardcoded metadata for GN
GN_BASE_METADATA = {
    'Id': 'GN',
    'Name': 'GN',
    'Description': 'GN is a meta-build system that generates build files for Ninja.',
    'URL': 'https://gn.googlesource.com/gn',
    'LicenseId': 'BSD-3-Clause',
    'License': 'BSD 3-clause \"New\" or \"Revised\" License',
    'LicenseFile': 'LICENSE',
    'Copyright': 'Copyright 2015 The Chromium Authors. All rights reserved.'
}

DIRECTORIES_TO_SKIP_BECAUSE_THEY_HAVE_VARIOUS_PARSING_ISSUES = [
    # Missing README.chromium and/or license
    os.path.join('third_party', 'catapult'),
    os.path.join('third_party', 'crashpad', 'crashpad', 'third_party', 'lss'),
    os.path.join('third_party', 'crashpad', 'crashpad', 'third_party', 'zlib'),
    os.path.join('third_party', 'devtools-frontend', 'src', 'front_end', 'third_party', 'chromium'),
    os.path.join('third_party', 'perfetto', 'protos', 'third_party', 'chromium'),
    os.path.join('third_party', 'rust'),
    os.path.join('third_party', 'tflite'),
]

# Wrappers for Chromium's package and SPDX writer tools
@dataclasses.dataclass(frozen=True)
class ExtendedPackage(spdx_writer._Package):
  extra_metadata: DefaultDict[str, Any]

class ExtendedSpdxJsonWriter(spdx_writer._SPDXJSONWriter):
  def __init__(self, root: str, root_package: ExtendedPackage, link_prefix: str,
               doc_name: str, doc_namespace: str):
    read_file = lambda x: pathlib.Path(x).read_text(encoding='utf-8')
    super().__init__(root, root_package, link_prefix, doc_name, doc_namespace, read_file)
    # Fixup top-level fields
    iso_time = datetime.datetime.now(datetime.timezone.utc).replace(microsecond=0).strftime('%Y-%m-%dT%H:%M:%SZ')
    self.content['creationInfo']['created'] = iso_time
    self.content['creationInfo']['comment'] = "Created with src/3rdparty/chromium/tools/licenses/sbom.py as part of the QtWebEngine build system"
    self.content['spdxVersion'] = 'SPDX-2.3'

  def add_package(self, pkg: ExtendedPackage):
    # super().add_package() adds everything as a dependency of the root package, we want to set
    # up our dependencies more precisely.
    pkg_id = self._get_package_id(pkg)
    if OVERRIDE_LICENSE_FILE_METADATA_KEY in pkg.extra_metadata:
      logger.info("Allowing known license ID without file for package %s" % (pkg.name))
      license_id = pkg.file
      need_to_add_license = False
    else:
      license_id, need_to_add_license = self._get_license_id(pkg)

    # Required fields
    pkg_content = {
        'SPDXID': pkg_id,
        'name': pkg.name,
        'licenseConcluded': license_id,
        'downloadLocation': 'NOASSERTION',
        'copyrightText': 'NOASSERTION',
        'licenseDeclared': 'NOASSERTION',
    }

    # Optional metadata that may be in README.chromium
    for metadata_name, json_name in CHROMIUM_TO_SPDX_KEY.items():
      if metadata_name in pkg.extra_metadata:
        pkg_content[json_name] = pkg.extra_metadata[metadata_name]

    # Add any existing CPE info as a comment, since it is not all in the correct format
    if 'CPEPrefix' in pkg.extra_metadata:
      cpe = pkg.extra_metadata['CPEPrefix']
      if cpe == 'unknown':
        comment = "Chromium authors declared this package to have an unknown CPE"
      else:
        comment = ("Chromium authors declared this package to have the CPE prefix '%s'" % cpe)

      if 'comment' in pkg_content:
        comment = pkg_content['comment'] + '\n' + comment
      pkg_content['comment'] = comment

    self.content['packages'].append(pkg_content)
    if need_to_add_license:
      self._add_license_file(pkg, license_id)

    return pkg_id

  def add_dependency(self, parent_id, child_id):
    self.content['relationships'].append({
      'spdxElementId': parent_id,
      'relationshipType': 'CONTAINS',
      'relatedSpdxElement': child_id,
    })

def GetDirectoryRevisionInfo(d):
  git_rev_list_result = subprocess.check_output(
      FIND_BASELINE_COMMIT_GIT_COMMAND + [d],
      cwd=ROOT,
      encoding='utf-8')
  commit_sha = git_rev_list_result.strip()
  git_log_result = subprocess.check_output(
      ['git', 'log', '--oneline', f'{commit_sha}..HEAD', '--', d],
      cwd=ROOT,
      encoding='utf-8')
  num_revisions = git_log_result.count('\n')
  if num_revisions == 0:
    return ''
  plural = '' if num_revisions == 1 else 's'
  comment_text = f'{num_revisions} revision{plural} added by Qt'
  return comment_text


def CleanupLicenseMetadata(dep_metadata):
  num_licenses = len(dep_metadata['License File'])
  if dep_metadata['Name'] in PACKAGES_TO_OVERRIDE_LICENSE_FILE_WITH_ID:
    logger.info("Using known license ID and ignoring license file(s) for package %s" % (dep_metadata['Name']))

    dep_metadata['License File'] = dep_metadata['License']
    dep_metadata[OVERRIDE_LICENSE_FILE_METADATA_KEY] = True
  elif num_licenses == 1:
    dep_metadata['License File'] = dep_metadata['License File'][0]
  elif num_licenses == 2 and dep_metadata['License'].endswith(", Patent"):
    # Handle cases like libaom, libwebp: We don't want to include the patent file
    former_license = dep_metadata['License']
    updated_license = dep_metadata['License'][:-len(", Patent")]
    logger.info("Patent file detected for package %s, skipping patent" % (dep_metadata['Name']))
    dep_metadata['License'] = updated_license

    dep_metadata['License File'] = dep_metadata['License File'][0]
  else:
    dep_metadata['License File'] = None
    raise license_tools.LicenseError("Dependency has %d licenses, expected exactly 1" % num_licenses)

def IsChromiumSubmoduleGitHistoryAvailable():
  baseline_cmd_result = subprocess.run(
      FIND_BASELINE_COMMIT_GIT_COMMAND + ['.'],
      cwd=ROOT,
      capture_output=True,
      encoding='utf-8')
  return baseline_cmd_result.returncode == 0 and baseline_cmd_result.stdout.strip()

def GetTargetMetadatas(gn_binary: str, gn_out_dir: str, gn_target: str):
  optional_keys = list(CHROMIUM_TO_SPDX_KEY.keys()) + ['Short Name', 'CPEPrefix']

  prev_cwd = os.getcwd()
  os.chdir(gn_out_dir)
  third_party_dirs = license_tools.FindThirdPartyDeps(gn_binary, gn_out_dir, gn_target, True, 'all')
  os.chdir(prev_cwd)

  # If src/3rdparty is not a git repo, skip adding revision info since it would fail on trying
  # to invoke git.
  can_include_git_info = IsChromiumSubmoduleGitHistoryAvailable()
  if not can_include_git_info:
      logger.warning("Could not find git history for '%s', git revision info will be missing" % os.path.join(ROOT, '..'))

  metadatas = {}
  for d in third_party_dirs:
    if d in DIRECTORIES_TO_SKIP_BECAUSE_THEY_HAVE_VARIOUS_PARSING_ISSUES:
      logger.info("Skipping '%s' because it has known parsing issue" % d)
      continue
    try:
      dir_metadata, errors = license_tools.ParseDir(d, ROOT,
                                                    require_license_file=True,
                                                    enable_warnings=True,
                                                    optional_keys=optional_keys)
      if not dir_metadata:
        logger.warning("Parsing '%s' returned nothing" % d)
      metadatas[d] = dir_metadata
      git_revision_info = GetDirectoryRevisionInfo(d) if can_include_git_info else None
      for dep_metadata in dir_metadata:
        CleanupLicenseMetadata(dep_metadata)

        if git_revision_info:
          dep_metadata['Source Info'] = git_revision_info
        dep_metadata['Comment'] = f'Location within source: {d}'
        # Prefer short name when available as the full name might have spaces/punctuation and
        # be quite long.
        if 'Short Name' in dep_metadata:
          dep_metadata['Name'] = dep_metadata['Short Name']
        dep_metadata['Name'] = dep_metadata['Name'].lower()
        if dep_metadata['Name'] in PACKAGES_TO_CLEAN_BAD_URL or CANONICAL_HOMEPAGE_STRING in dep_metadata['URL']:
          logger.info("Cleaning bad URL from package: %s" % dep_metadata['Name'])
          del dep_metadata['URL']
    except license_tools.LicenseError as err:
      logger.error("Failed parsing '%s': %s" % (d, err))
      pass
    except Exception as err:
      logger.error("Failed parsing '%s': %s" % (d, err))
      pass
    license_tools.LogParseDirErrors(errors)

  return metadatas

def CreateSpdxText(targets_and_metadatas, package_id: str, doc_namespace: str, gn_version: str):
  root_license = os.path.join(ROOT, 'LICENSE')
  root = os.path.dirname(ROOT)
  link_prefix = ''
  doc_name = package_id

  def make_pkg_name(s):
    return 'QtWebEngine-Chromium-' + package_id + '-' + s

  root_pkg = ExtendedPackage(make_pkg_name("Internal-Components"), root_license, {})
  writer = ExtendedSpdxJsonWriter(root, root_pkg, link_prefix, doc_name, doc_namespace)

  already_added_packages = dict()
  for top_level_target, metadatas in targets_and_metadatas:

    assert top_level_target[0] == ':'
    top_level_pkg_id = writer.add_package(ExtendedPackage(make_pkg_name(top_level_target[1:]), root_license, {}))
    writer.add_dependency(writer.root_package_id, top_level_pkg_id)

    for directory in sorted(metadatas):
      dir_metadata = metadatas[directory]
      for dep_metadata in dir_metadata:
        license_file = dep_metadata.pop('License File')
        # This is used in CleanupLicenseMetadata as a sentinel to skip an
        # errored-out dependency.
        if license_file is None:
          continue

        child_pkg_name = make_pkg_name(dep_metadata.pop('Name'))
        child_pkg_key = directory + ':' + child_pkg_name
        if child_pkg_key not in already_added_packages:
          child_pkg_id = writer.add_package(ExtendedPackage(child_pkg_name, license_file, dep_metadata))
          already_added_packages[child_pkg_key] = child_pkg_id
        writer.add_dependency(top_level_pkg_id, already_added_packages[child_pkg_key])


  # Manually add GN package
  gn_license = os.path.join(os.path.dirname(ROOT), 'gn', 'LICENSE')
  gn_metadata = GN_BASE_METADATA.copy()
  gn_metadata['Version'] = gn_version
  gn_pkg_id = writer.add_package(ExtendedPackage(make_pkg_name('GN'), gn_license, gn_metadata))
  writer.add_dependency(writer.root_package_id, gn_pkg_id)

  return writer.write()

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--gn-target-list', required=True, help='Semicolon-separated list of GN targets to scan for dependencies.')
  parser.add_argument('--build-dir-list', required=True,
                      help='Semicolon-separated list of build directories corresponding to the targets in --gn-target-list')
  parser.add_argument('--gn-binary', required=True, help="GN binary location.")
  parser.add_argument('--gn-version', required=True, help="GN version.")
  parser.add_argument('--package-id', required=True, help="Camelcase package id. This is used for several purposes")
  parser.add_argument('--namespace', required=True, help="Namespace of the document")
  parser.add_argument('--verbose', action="store_true", help="Verbose output")
  parser.add_argument('output_file')
  args = parser.parse_args()

  logging.basicConfig(level=logging.INFO if args.verbose else logging.WARNING)
  logger.info("sbom.py run with arguments: %s" % (' '.join(sys.argv)))
  gn_target_list = args.gn_target_list.split(';')
  build_dir_list = args.build_dir_list.split(';')
  if len(gn_target_list) != len(build_dir_list):
    print("Number of targets must match number of build directories (%s, %s)" % (gn_target_list, build_dir_list))
    sys.exit(1)

  targets_and_metadatas = [(t, GetTargetMetadatas(args.gn_binary, b, t)) for b, t in zip(build_dir_list, gn_target_list)]

  spdx_text = CreateSpdxText(targets_and_metadatas, args.package_id, args.namespace, args.gn_version)

  with open(args.output_file, 'w', encoding='utf-8') as f:
    f.write(spdx_text)

if __name__ == '__main__':
  sys.exit(main())
