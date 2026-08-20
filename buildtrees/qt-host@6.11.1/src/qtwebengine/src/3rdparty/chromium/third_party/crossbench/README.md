# Crossbench

Crossbench is a cross-browser/cross-benchmark runner to extract performance
numbers.

Mailing list: <crossbench@chromium.org>

Issues/Bugs: [Tests > CrossBench](https://bugs.chromium.org/p/chromium/issues/list?q=component%3ATest%3ECrossBench)

Supported Browsers: Chrome/Chromium, Firefox, Safari and Edge.

Supported OS: MacOS, Android, Linux and Windows.

## Basic usage:
### Chromium Devs (with a full chromium checkout)

- Use the [`tools/perf/cb`](https://source.chromium.org/chromium/chromium/src/+/main:tools/perf/cb) helper script.

Alternative:
- Make sure to run `gclient sync` to get the latest crossbench roll.
- Use a standalone [crossbench checkout](#checking-out-code) and run `./cb.py`.

### Standalone installation
Note: The pip package is only irregularly updated and thus likely out of date.
- Use `pip install crossbench`,
- or use the "poetry" package manager, see the [development section](#development).

### Running Workloads Examples
Run the latest [speedometer benchmark](https://browserbench.org/Speedometer/)
20 times with the system default browser (chrome-stable):
```bash
# Run chrome-stable by default:
./cb.py speedometer --repeat=3

# Compare chrome browser versions and a local chrome build on jetstream:
./cb.py jetstream --browser=chrome-stable --browser=chrome-m90 --browser=$PATH
```

Profile individual line items (with pprof on linux):
```bash
./cb.py speedometer --probe='profiling' --separate
```

Use a custom chrome build and only run a subset of the stories:
```bash
./cb.py speedometer --browser=$PATH --probe='profiling' --story='jQuery.*'
```

Profile a website for 17 seconds on Chrome M100 (auto-downloading on macOS and linux):
```bash
./cb.py loading --browser=chrome-m100 --probe='profiling' --url=www.cnn.com,17s
```

Collect perfetto data from loading separate websites on multiple attached
android devices using the device ID or unique device names
(see `adb devices -l`):

```bash
./cb.py loading --probe-config=./config/probe/perfetto/default.config.hjson \
    --browser='Pixel_4:chrome-stable' --browser='AA00BB11:chrome-stable' \
    --parallel=platform \
    --url=https://theverge.com,15s,https://cnn.com,15s  --separate
```


## Main Components

### 🌐 Browsers
Crossbench supports running benchmarks on one or multiple browser configurations.
The main implementation uses selenium for maximum system independence.

You can specify a browser with `--browser=<name>`. You can repeat the
`--browser` argument to run multiple browser. If you need custom flags for
multiple browsers use `--browser-config` (or pass simple flags after `--` to
the browser).

Single browser example:
```bash
./cb.py speedometer --browser=$BROWSER -- --enable-field-trial-config
```

Multi-browser example:
```bash
./cb.py sp3 --stories='TodoMVC.*' \
  --browser=firefox --browser=safari \
  --browser=chrome-M123-dev --browser=./out/Release/Chromium.app
```

#### `--browser` flag on desktop:
| Example | Description |
|------|-------------|
|`--browser=chrome-stable`| Use the installed Chrome stable on the host. Also works with `beta`, `dev` and `canary` versions. |
|`--browser=edge-stable`| Use the installed Edge stable on the host. Also works with `beta`, `dev` and `canary` versions. |
|`--browser=safari-stable`| Use the installed Safari stable version on the host. Also works with `technology-preview` |
|`--browser=firefox-stable`| Use the installed Firefox stable version on the host. Also works with `dev` and `nightly` versions. |
|`--browser=./out/Release/chrome`| Use a locally compiled chrome version. Any path to a chrome binary will work. |
|`--browser=chrome-m123`| Download the latest M123 chrome stable release and install it locally |
|`--browser=chrome-M123-canary`| Download the latest M123 chrome canary release and install it locally |
|`--browser=chrome-latest`| Download the latest chrome stable release and install it locally |
|`--browser=chrome-latest-canary`| Download the latest chrome canary release and install it locally |
|`--browser=chrome-125.0.6422.112`| Download and install a specific stable chrome version. |
|`--browser=chrome-125.0.6422.112-dev`| Download and install a specific dev chrome version. |
|`--browser=chrome-M100...M123`| Download and install a range of 24 different chrome stable milestones. |


#### `--browser` flag on mobile:
You can directly run on attached android devices using the device ID or unique device names.
They need to have [developer mode and usb-debugging enabled](https://developer.android.com/studio/debug/dev-options#Enable-debugging).

| Example | Description |
|------|-------------|
| `--browser=adb:chrome-stable` | Use Chrome stable on a single attached adb device. Note this will fail if there is more than one attached device. |
|  `--browser=Pixel_7_pro:chrome-canary` | Use Chrome canary on an attached Pixel 7 Pro device. Note this will fail if there is more than one Pixel 7 pro attached.|
| `--browser=2900FF00BB:chrome-dev` | Use Chrome dev on an attached adb device with the serial id `2900FF00BB`. Use `adb devices -l` to find the serial id.|
| `--browser=adb:out/arm64.apk/bin/chrome_public_apk` | Use a locally built [chrome_public_apk helper](https://chromium.googlesource.com/chromium/src/+/main/docs/android_build_instructions.md#build-the-full-browser) with an automatically chosen locally build chromedriver from an adjacent build folder. This will also auto-install chrome on your device. |

#### Browser Config File
For more complex scenarios you can use a
[browser.config.hjson](config/doc/browser.config.hjson) file.
It allows you to specify multiple browser and multiple flag configurations in
a single file and produce performance numbers with a single invocation.

```bash
./cb.py speedometer --browser-config=config.hjson
```

The [example file](config/doc/browser.config.hjson) lists and explains all
configuration details.

#### Remote WebDriver Interface
Crossbench also supports benchmarking browsers on remote machines
running Linux or ChromeOS, via SSH.
The remote machine is expected to have at least two ports open to the host:
(a) the SSH port (typically `22`), and
(b) the WebDriver port (typically `9515`).
The [remote browser example](config/doc/remote_browser.config.hjson)
describes the configuration details for both Linux and ChromeOS.

On ChromeOS, Crossbench requires
[ChromeDriver](https://developer.chrome.com/docs/chromedriver/get-started/chromeos/)
to interact with Chrome,
and [Autotest](https://chromium.googlesource.com/chromiumos/third_party/autotest/+/HEAD/docs/user-doc.md)
for creating ephemeral sessions for testing.
Both ChromeDriver and Autotest are pre-installed on ChromeOS test images.
Detailed instructions for flashing Chromebooks with test images are provided at:
go/arc-setup-dev-mode-dut#usb-cros-test-image.

### Safari on macOS

Safari needs some extra steps to work:

- `safaridriver --enable` to allow automation
- Open the Safari settings:
  - "Advanced" tab: Check "Show features for web developers"
  - "Developer" tab: Check "Allow remote automation"
  - "Developer" tab: Optional, if you plan to use the apple-script browser, also check "Allow JavaScript from Apple Events"


### 🩺 Probes
Probes define a way to extract arbitrary (performance) numbers from a
host or running browser. This can reach from running simple JS-snippets to
extract page-specific numbers to system-wide profiling.

Multiple probes can be added with repeated `--probe='XXX'` options.
You can use the `describe probes` subcommand to list all probes:

```bash
# List all probes:
./cb.py describe probes

# List help for an individual probe:
./cb.py describe probe v8.log
```

#### Inline Probe Config
Some probes can be configured, either with inline JSON when using `--probe` or
in a separate `--probe-config` HJSON file. Use the `describe` command to list
all options. The inline JSON or HJSON is the same format as used in the separate
probe config files (see below).

```bash
# Get probe config details:
./cb.py describe probe v8.log

# Use inline HJSON to configure a probe:
./cb.py speedometer --probe='v8.log:{prof:true}'
```

#### Probe Config File
For complex probe setups you can use `--probe-config=<file>`.
The [example file](config/doc/probe.config.hjson) lists and explains all
configuration details. For the specific probe configuration properties consult
the `describe` command.
You can find more examples in [config/doc/probe](config/doc/probe).

### 📏 Benchmarks
Use the `describe` command to list all benchmark details:

```bash
# List all benchmark info:
./cb.py describe benchmarks

# List an individual benchmark info:
./cb.py describe benchmark speedometer_3.0

# List a benchmark's command line options:
./cb.py speedometer_3.0 --help
```

### 📚 Stories
Stories define sequences of browser interactions. This can be simply
loading a URL and waiting for a given period of time, or in more complex
scenarios, actively interact with a page and navigate multiple times.

Use `--help` or describe to list all stories for a benchmark:

```bash
./cb.py speedometer --help
```

Use `--stories` to list individual story names, or use regular expression
as filter.

```bash
# Only run Angular workloads:
./cb.py speedometer --browser=$BROWSER --stories='.*Angular.*'

# Exclude bomb-workers and segmentation:
./cb.py js --browser=chrome-m120-canary --stories='^(?!(segmentation|bomb-workers)).*'
```

### Loading Benchmark and Stories

For non-press benchmarks you should use the `loading` benchmark and you have
multiple ways to specify the stories:

| Flag | Description |
| ---- | -- |
| `--stories` or `--url`  | Use a comma-separate list of predefined pages or URLs for simple loading. |
| `--page-config`  |  Page configs (see [`config/docs/page.config.hjson`](config/doc/page.config.hjson)) for a detailed example |
| `--config` | All-in-one config including browser, probes and pages |


```bash
# Load https://cnn.com for 5 seconds:
./cb.py loading --url=cnn.com,5s
```

```bash
# Load cnn and facebook separately:
./cb.py loading --url=cnn.com,5s,facebook.com,10s --separate
```

```bash
# Use page-config for complex page interactions:
./cb.py loading --page-config=config/doc/page.config.hjson --browser=chrome-canary
```


### 🛜 Network

Crossbench supports various network settings directly, see `./cb.py help network` for more detail.
| Type | Description |
| ------- | -- |
| LIVE    | Live network.  |
| WPR     | Replayed network from a [wpr.go](https://chromium.googlesource.com/webpagereplay/) archive. Note you can use the `--probe=wpr` probe to record fresh network archives |
| LOCAL   | Serve content from a local http file server. This is useful for local debugging or running press benchmarks. |

| Example | Description |
| -- | -- |
| `--network=/path/to/speedometer` | Use a local fileserver. |
| `--network=3G-slow` | Use live network with slow 3G traffic shaping. |
| `--network=path/to/archive.wprgo` | Use 'wpr' replay network with the given request archive. |
| `--network='{type:"wpr", path:"./archive.wprgo", speed:"3G-regular"}'`| Use 'wpr' network with 3G traffic shaping. |

## 🛠️ Development

### Checking Out Code
Don't just `git clone` the crossbench repo! Use depot_tools to set everything
up correctly for you.

- Install [Chromium depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up).
- Get the crossbench code with all dependencies:
```
mkdir code
cd code
fetch crossbench
cd crossbench
```
- Don't forget to run `gclient sync` every time you pull new changes from the
crossbench repo.

### Poetry Setup
This project uses [poetry](https://python-poetry.org/) deps and package scripts
to setup the correct environment for testing and debugging.

```bash
# a) On debian:
sudo apt-get install python3.11 python3.11-dev python3-poetry
# b) With python 3.11 installed already:
pip3 install poetry
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
# Select the python version you want to use (3.11):
poetry env use 3.11
poetry install

# For windows you have to skip pytype support:
poetry env use 3.11
poetry install --without=dev-pytype
```

### Crossbench
For local development / non-chromium installation you should
use `poetry run cb ...` instead of `./cb.py ...`.

Side-note, beware that poetry eats up an empty `--`:

```bash
# With cb.py:
./cb.py speedometer ... -- --custom-chrome-flag ...
# With poetry:
poetry run cb speedometer ... -- -- --custom-chrome-flag ...
```

### Tests
```
poetry run pytest
```

Run detailed test coverage:
```bash
poetry run pytest --cov=crossbench --cov-report=html
```

Run [pytype](https://github.com/google/pytype) type checker:
```bash
poetry run pytype -j auto .
```
