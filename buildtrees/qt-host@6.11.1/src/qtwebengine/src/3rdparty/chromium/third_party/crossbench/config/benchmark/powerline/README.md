# PowerLine benchmark

This folder contains configs for the PowerLine benchmark. This benchmark contains some long-running tests that are designed to test common battery-draining scenarios. It's only intended to run on Pixel devices with the correct power-rails telemetry.

## Running the benchmarks

```
./cb.py powerline --browser=adb:chromium
```

The browser can be `android:chrome-canary`, `android:chrome-stable` or anything else.

### Workload
`podcast` simulates a user clicking play on a podcast with an HTML5 media element, then switching the screen off.

