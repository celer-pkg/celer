#!/usr/bin/env bash
#
# Provision the append-only MinIO setup used by celer's pkgcache backend.
# Run once as a MinIO administrator; safe to re-run.
#
#   ./celer-minio-access.sh --host=http://minio.example.com:9000 \
#              --admin-user=<root-user> --admin-password=<root-password>
#
# It creates the versioned bucket `celer-cache`, applies the append-only policy
# defined in the JSON next to this script, and creates the access key celer uses:
# the MinIO user `celer-pkgcache`, visible under Identity -> Users.
#
# celer can then list, download and upload (overwrite) objects, and every delete
# attempt is denied. Removing an object stays an administrator action.
#
# Requires: mc on PATH - https://min.io/docs/minio/linux/reference/minio-mc.html
#
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

POLICY_NAME=celer-append-only
POLICY_FILE=celer-minio-policy.json
BUCKET=celer-cache
# A MinIO user is itself an access key/secret pair, so this is both the user and
# the access key celer authenticates with. A service account was not used because
# its access key may not be named after the user that owns it.
ACCESS_KEY=celer-pkgcache
ALIAS=celer-admin

HOST="${MINIO_HOST:-}"
ADMIN_USER="${MINIO_ROOT_USER:-}"
ADMIN_PASSWORD="${MINIO_ROOT_PASSWORD:-}"
ROTATE=false
DRY_RUN=false

usage() {
    cat <<'EOF'
usage: ./celer-minio-access.sh --host=<url> --admin-user=<user> --admin-password=<password>
                [--runtime-access-key=<key>] [--rotate] [--dry-run]

  --host                 S3 API endpoint, not the console port
  --admin-user           MinIO administrator (env: MINIO_ROOT_USER)
  --admin-password       MinIO administrator password (env: MINIO_ROOT_PASSWORD)
  --runtime-access-key   Access key to create (default: celer-pkgcache)
  --rotate               Replace the secret key of an existing access key
  --dry-run              Print the mc commands, change nothing

Errors are fatal, and existing objects/keys are never deleted by celer.
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
    --host=*) HOST="${1#*=}" ;;
    --admin-user=*) ADMIN_USER="${1#*=}" ;;
    --admin-password=*) ADMIN_PASSWORD="${1#*=}" ;;
    --runtime-access-key=*) ACCESS_KEY="${1#*=}" ;;
    --rotate) ROTATE=true ;;
    --dry-run) DRY_RUN=true ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        usage >&2
        exit 1
        ;;
    esac
    shift
done

if [[ -z "$HOST" || -z "$ADMIN_USER" || -z "$ADMIN_PASSWORD" ]]; then
    echo "--host, --admin-user and --admin-password are required" >&2
    exit 1
fi

if ! command -v mc >/dev/null 2>&1; then
    cat >&2 <<'EOF'
mc was not found on PATH, install it first:
  linux:   mkdir -p ~/.local/bin && curl -L https://dl.min.io/aistor/mc/release/linux-amd64/mc -o ~/.local/bin/mc && chmod +x ~/.local/bin/mc
  windows: mkdir -p ~/.local/bin && curl -L https://dl.min.io/aistor/mc/release/windows-amd64/mc.exe -o ~/.local/bin/mc.exe
  (add ~/.local/bin to PATH if it is not there yet)
EOF
    exit 1
fi

SECRET="$(openssl rand -hex 20)"

# A temporary mc config dir keeps the administrator's own ~/.mc untouched, and
# the wrapper turns every call below into a one-liner.
CONF_DIR="$(mktemp -d)"
trap 'rm -rf "$CONF_DIR"' EXIT
mc() { command mc --config-dir "$CONF_DIR" --no-color "$@"; }

# Initialise the temp config dir silently, so the first real command does not
# print mc's "Configuration written to ..." notices.
mc alias list >/dev/null 2>&1 || true

# probe: does it already exist? (dry-run always reports "no", so the printed
# sequence describes a first-time setup)
probe() {
    [[ "$DRY_RUN" == false ]] && mc "$@" >/dev/null 2>&1
}

# run: execute, or just print the command in dry-run mode.
run() {
    if [[ "$DRY_RUN" == true ]]; then
        local printable="${*//$ADMIN_PASSWORD/********}"
        printf '+ mc %s\n' "${printable//$SECRET/********}"
        return 0
    fi
    mc "$@"
}

echo "==> 1/3 bucket ${BUCKET} with object locking (implies versioning)"
run alias set "$ALIAS" "$HOST" "$ADMIN_USER" "$ADMIN_PASSWORD"
run mb --with-lock --ignore-existing "${ALIAS}/${BUCKET}"

echo "==> 2/3 policy ${POLICY_NAME} from ${POLICY_FILE}"
# `mc admin policy create` is an upsert, so the JSON file stays the source of
# truth. Removing first is not an option: MinIO refuses to remove a policy that
# is still attached to a user.
run admin policy create "$ALIAS" "$POLICY_NAME" "$POLICY_FILE"

echo "==> 3/3 access key ${ACCESS_KEY}"
if probe admin user info "$ALIAS" "$ACCESS_KEY" && [[ "$ROTATE" == false ]]; then
    echo "    '${ACCESS_KEY}' already exists, secret key unchanged; pass --rotate to replace it"
    exit 0
fi

# `mc admin user add` is an upsert: it creates the user, or replaces the secret
# key of an existing one.
run admin user add "$ALIAS" "$ACCESS_KEY" "$SECRET"
run admin policy attach "$ALIAS" "$POLICY_NAME" --user "$ACCESS_KEY"

if [[ "$DRY_RUN" == true ]]; then
    echo
    echo "Dry run: nothing was changed."
    exit 0
fi

cat <<EOF

Access key ready. Point celer at the cache:

  celer configure --pkgcache-minio-host=${HOST} \\
                  --pkgcache-minio-access-key=${ACCESS_KEY} \\
                  --pkgcache-minio-secret-key=${SECRET}
EOF
