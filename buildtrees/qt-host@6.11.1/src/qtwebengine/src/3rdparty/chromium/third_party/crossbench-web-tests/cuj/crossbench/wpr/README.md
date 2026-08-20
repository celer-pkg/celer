# Managing Web Page Replay Archives (.wprgo)

This guide explains how to update Web Page Replay (WPR) archives (`.wprgo` files). Use these instructions if you need to:

* Adding new pages to the archive
* Removing pages from the archive
* Replacing some web pages in the archive

Follow the instructions in this doc.

## Source code location

Web Page Replay can be found under `third_party/webpagereplay` of this repo.

## Adding new pages to the archive

### Record new page loads in a new archive
Run `src/wpr.go` in recording mode. Run the following under `realpath ~/go/pkg/mod/github.com/catapult-project/catapult/web_page_replay_go@*`:

```bash
# If wpr is in your PATH:
# wpr replay --http_port=8080 --https_port=8081 /tmp/new_pages.wprgo
# Or, from the web_page_replay_go source directory:
go run src/wpr.go replay --http_port=8080 --https_port=8081 /tmp/new_pages.wprgo
```

Wpr runs as a proxy that sits between the browser and the web server and records the web requests and responses that flow through it. To manually record the web pages, start Chrome as:

```bash
/opt/google/chrome/chrome \
  --user-data-dir=.chrome-profile \
  --host-resolver-rules="MAP *:80 127.0.0.1:8080,MAP *:443 127.0.0.1:8081,EXCLUDE localhost" \
  --ignore-certificate-errors-spki-list="PhrPvGIaAMmd29hj8BCZOq096yj7uMpRNHpn5PDxI6I=,2HcXCSKKJS0lEXLQEWhpHUfGuojiU0tiT5gOF9LP6IQ="
```

Interact with the web server. When done, terminate the `wpr` process (Ctrl+C); web requests and responses will be saved to `/tmp/new_pages.wprgo`.

### Notes on stateful web sites
Some web sites may serve different web pages for the same URL, especially if steps requiring human interaction (like logins or CAPTCHAs) are involved. To record the desired content on such sites (e.g., after a login or verification step):

1.  Finish the steps requiring human interactions.
2.  Clear browser cache (HTML, CSS, images, JS sources). **Important:** Do not clear cookies or browsing history, as these may be needed to maintain your session state for the recording.
3.  Start `wpr` (or `go run src/wpr.go` if in the source directory) in recording mode as shown above.
4.  Interact with the web server. `wpr` will record all web pages.

### Merge with the existing archive
Then, merge the newly recorded pages (e.g., `/tmp/new_pages.wprgo`) into your main archive (e.g., `existing.wprgo`) using `httparchive` (or `go run src/httparchive.go` if in the source directory):

```bash
go run src/httparchive.go merge existing.wprgo /tmp/new_pages.wprgo merged.wprgo
```

## Removing pages from the archive
Trim the archive using `httparchive` (or `go run src/httparchive.go` if in the source directory).

### Removing by URI
To remove page(s) by their URI, use the `--full_path` option:

```bash
go run src/httparchive.go trim \
  --full_path /chromiumos-test-assets-public/power_LoadTest/2023-08-09/google.html \
  existing.wprgo trimmed.wprgo
```

This removes all requests with the URI `/chromiumos-test-assets-public/power_LoadTest/2023-08-09/google.html` from `existing.wprgo` and saves the trimmed archive as `trimmed.wprgo`.

### Removing by host names
To remove page(s) by host names:

```bash
go run src/httparchive.go trim --host "www.google.com" \
  existing.wprgo trimmed.wprgo
```
This removes all requests with the hostname `www.google.com` from `existing.wprgo`
and saves the trimmed archive as `trimmed.wprgo`. Note that `--host` matches the
full host name.

To remove all requests from hosts ending with `.example.com` (e.g., `a.example.com`, `b.example.com`):

1.  List relevant hostnames in the archive:
    ```bash
    go run src/httparchive.go ls existing.wprgo | awk '{print $2}' | \
      grep -e 'example.com$' | uniq
    ```
2.  Iterate and trim. The following script provides an example.
    **Note:** This script modifies `existing.wprgo` by processing a temporary copy.

    ```bash
    WPR_TOOL_CMD="go run src/httparchive.go" # Assuming execution from source dir

    INPUT_WPR="existing.wprgo"
    TEMP_WPR_CURRENT="${INPUT_WPR}.current_trim_step.wprgo"
    TEMP_WPR_NEXT="${INPUT_WPR}.next_trim_step.wprgo"

    HOSTS_TO_REMOVE=(
            "a.example.com"
            "b.example.com"
            "c.example.com"
    )

    # Make a backup of the original file before starting extensive operations
    cp "${INPUT_WPR}" "${INPUT_WPR}.bak_$(date +%Y%m%d_%H%M%S)"

    cp "${INPUT_WPR}" "${TEMP_WPR_CURRENT}"

    for host in "${HOSTS_TO_REMOVE[@]}"; do
      ${WPR_TOOL_CMD} trim --host "${host}" "${TEMP_WPR_CURRENT}" "${TEMP_WPR_NEXT}"
      if [ $? -ne 0 ]; then
        rm -f "${TEMP_WPR_NEXT}" # Clean up next temp file if created
        exit 1
      fi
      mv "${TEMP_WPR_NEXT}" "${TEMP_WPR_CURRENT}"
    done

    # All trims successful, move the final result back to the original filename
    mv "${TEMP_WPR_CURRENT}" "${INPUT_WPR}"
    ```

## Replacing web pages in the archive
To replace web pages in the archive:
1.  Remove the old pages using the methods described above.
2.  Add the new pages as detailed in the "Adding new pages to the archive" section.

## Uploading to the archive
When finished updating the archive (e.g., `existing.wprgo`):
1.  Upload the archive to Google Cloud Storage:
    ```bash
    gsutil cp existing.wprgo gs://chrome-partner-telemetry/cros/cuj/crossbench/page-click-scroll-$TIMESTAMP.wprgo
    ```
    (Replace `$TIMESTAMP` with a relevant identifier, e.g., the current date `YYYYMMDD` or a version number.)

2.  Finally, update the `wpr-setup.sh` script (located in this directory) to point to the new archive filename in Google Cloud Storage.

## Reference
See [https://chromium.googlesource.com/catapult/+/HEAD/web_page_replay_go/README.md](https://chromium.googlesource.com/catapult/+/HEAD/web_page_replay_go/README.md) for the official WprGo documentation.
