#!/usr/bin/python
import os
from pathlib import Path

files = []
files_unix = []
files_windows = []

includes = []

for path in Path('.').rglob('*.cpp'):
    if str(path).lower().find('windows') != -1:
        files_windows.append(path)
    elif str(path).lower().find('unix') != -1 or str(path).lower().find('linux') != -1:
        files_unix.append(path)
    elif str(path).find('CmMathUtils.cpp') == -1:
        files.append(path)

for path in Path('.').rglob('*.h'):
    if str(path).lower().find('windows') != -1:
        files_windows.append(path)
    elif str(path).lower().find('unix') != -1 or str(path).lower().find('linux') != -1:
        files_unix.append(path)
    else:
        files.append(path)
    includes.append(os.path.dirname(path))

files = sorted(set(files))
files_windows = sorted(set(files_windows))
files_unix = sorted(set(files_unix))
includes = sorted(set(includes))

print("Common sources:")
for path in files:
    print(path)
print("")
print("Windows sources:")
for path in files_windows:
    print(path)
print("")
print("Unix sources:")
for path in files_unix:
    print(path)
print("")
print("include directories:")
for path in includes:
    print(path)
