# `web-tests`

`web-tests` contains:
- Definitions for CUJs implemented using crossbench's loading benchmark
- Configuration files for running benchmarks that are built in to crossbench (such as speedometer)
- Metric definitions and queries for CUJs and benchmarks.

## Setup

**Do not `git clone` web-tests**. Use the `fetch` command included with `depot_tools`.

- Install [Chromium depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up).
- Get the web-tests code with all dependencies:
```
mkdir src
cd src
fetch web-tests
cd web-tests
```
Don't forget to run `gclient sync` every time you pull new changes from origin.

### Poetry
web-tests uses [poetry](https://python-poetry.org/)
manage python dependencies.
```bash
# python3.11-dev is required for pandas
sudo apt-get install python3.11 python3.11-dev python3-poetry
```

Alternatively, install poetry in a python venv:
```bash
python3 -m venv web-tests-venv
source web-tests-venv/bin/activate
pip install poetry
```

Check that you have poetry on your path and make sure you have the right
`$PATH` settings.
```bash
poetry --help || echo "Please update your \$PATH to include poetry bin location";
# Depending on your setup, add one of the following to your $PATH:
echo "`python3 -m site --user-base`/bin";
python3 -c "import sysconfig; print(sysconfig.get_path('scripts'))";
```

Install the necessary dependencies from the lock file using poetry:

```bash
cd cuj/crossbench/runner
export PYTHON_KEYRING_BACKEND=keyring.backends.null.Keyring
poetry env use 3.11
poetry install
```
Setting PYTHON_KEYRING_BACKEND to keyring.backends.null.Keyring disables keyring
and prevents `poetry install` from getting stuck waiting for user input in the
GUI.

## Running Tests

### Android
Before running a test against an android target, make sure your device is connected through `adb`:
```
adb devices
> List of devices attached
> 192.168.20.194:5555     device
```

Replace `<DEVICE ID>` below with the actual device ID from `adb devices`:
```bash
cd cuj/crossbench/runner
poetry run python run.py --platform adb --device <DEVICE ID>
```

### ChromeOS
Before running a test against a ChromeOS target, make sure passwordless SSH is available to the device. Either an IP address or a SSH host is supported as the device id.

Replace `<DEVICE>` below with the IP address or hostname of your device:
```bash
cd cuj/crossbench/runner
poetry run python run.py --platform cros --device <DEVICE>
```

### Local
*Running against a local browser on linux is minimally supported, but may require manual changes to test configuration.*

```bash
cd cuj/crossbench/runner
poetry run python run.py --platform local
```

### Specifying Tests and Variants
The minimal invocation of the runner will attempt to run all benchmarks, CUJs, and corresponding variants in series.

To run a subset of tests, use the `--tests` flag. `--tests` supports Python regex format for matching the test names.
```bash
poetry run python run.py --platform adb --device <DEVICE ID> --tests speedometer.*
```

To specify only certain variants of a test, you can use the `--variants` flag. `--variants` also supports Python regex format for matching variants.
```bash
poetry run python run.py --platform adb --device <DEVICE ID> --tests local-conference --variants 16p
```

### Specifying Browsers
By default the runner will use 'Chrome' if the `--browser` flag is not specified. The format accepted by the `--browser` flag depends on the platform. For Android, use the package name of the installed browser. For ChromeOS or local, use the path to the browser executable.

### Secrets
Some CUJs require secrets to perform privileged actions (such as a test account username/password, or auth tokens for the Google Meet Bond API). Place your secrets in `secrets.hjson` and pass the file to the runner:

```bash
poetry run python run.py --platform adb --device <DEVICE ID> --secrets /home/me/secrets.hjson --tests docs
```

### Looping Tests
Tests can be repeated for a number of iterations or for a specified amount of time using the `--playback` flag. This flag is supported by crossbench and will iterate the post-setup sections of a CUJ and collect metrics for the entire invocation (instead of splitting metrics by iteration).

```bash
poetry run python run.py --platform adb --device <DEVICE ID> --tests tab-stress --variants blank-tab --playback 50x
```

```bash
poetry run python run.py --platform adb --device <DEVICE ID> --tests tab-stress --variants blank-tab --playback 2h
```


## Test Definitions

All test definitions and supporting files for crossbench based tests should be within the `cuj/crossbench` directory.

### Benchmarks

Benchmarks are tests that are directly supported by and integrated into crossbench. Examples of benchmarks are speedometer, motionmark, and jetstream.

Every directory within `cuj/crossbench/benchmarks` defines a crossbench benchmark that is supported by web-tests.

For example, `cuj/crossbench/benchmarks/speedometer_3.0` contains the necessary configuration files for running the speedometer_3.0 benchmark as a web-test using crossbench.

Within a benchmark directory, the following files can be present:

- `browser-flags.hjson`
  - Defines the browser flags used when running the benchmark
- `probe-config.hjson`
  - Defines the probe config used when running the benchmark
- `cb-args` (Optional)
  - Single-line (no trailing newline) file that specifies extra arguments to pass to Crossbench.

### CUJs

CUJs are tests that are implemented on top of crossbench's loading benchmark. These tests use `page-config.hjson` files to define a list of actions to perform in the browser.

Every directory within `cuj/crossbench/cujs` defines a CUJ that can be run using crossbench.

Within a CUJ directory, the following files determine how a CUJ is run:

- `page-config.hjson`
  - Contains the page configuration for the loading benchmark.
  - Optionally several page configurations can be specified using the format `<variant>.page-config.hjson` if several similar tests should be grouped together under a single CUJ directory.
  - Page configs define one configuration of a test. If you want variants of a test
that differ only in probe configs or args, you will need a new page config file
as well.
- `probe-config.hjson` or `<variant>.probe-config.hjson`
  - Defines the probe config used when running the CUJ
  - When running `<variant>.page-config.hjson`, if `<variant>.probe-config.hjson`
exists it will be used, otherwise `probe-config.hjson` will be used.
- `browser-flags.hjson` or `<variant>.browser-flags.hjson`
  - Defines the browser flags used when running the benchmark
  - When running `<variant>.page-config.hjson`, if `<variant>.browser-flags.hjson`
exists it will be used, otherwise `browser-flags.hjson` will be used.
- `cb-args` (Optional)
  - Single-line (no trailing newline) file that specifies extra arguments to pass to Crossbench.
