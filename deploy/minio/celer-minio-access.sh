#!/usr/bin/env bash
#
# It creates the versioned bucket `celer-cache`, applies the append-only policy
# defined in the JSON next to this script, and creates the access key celer uses:
# the MinIO user `celer-pkgcache`, visible under Identity -> Users.
#
# celer can then list, download and upload (overwrite) objects, and every delete
# attempt is denied. Removing an object stays an administrator action.
#
# Usage:
# ./celer-minio-access.sh \
#   --host=http://minio.example.com:9000 \
#   --admin-user=<root-user> \
#   --admin-password=<root-password>
#
# Requires: mc - https://min.io/docs/minio/linux/reference/minio-mc.html
#
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

POLICY_NAME=celer-append-only
POLICY_FILE=celer-minio-policy.json
BUCKET=celer-cache
ACCESS_KEY=celer-pkgcache   # Default access_key, it can be override later.
ALIAS=celer-admin           # Default minio's alias.

HOST="${MINIO_HOST:-}"
ADMIN_USER="${MINIO_ROOT_USER:-}"
ADMIN_PASSWORD="${MINIO_ROOT_PASSWORD:-}"
ROTATE=false

usage() {
    cat <<'EOF'
usage: 
./celer-minio-access.sh --host=<url> --admin-user=<user> --admin-password=<password>
    [--runtime-access-key=<key>] [--rotate]

  --host                 S3 API endpoint, not the console port
  --admin-user           MinIO administrator (env: MINIO_ROOT_USER)
  --admin-password       MinIO administrator password (env: MINIO_ROOT_PASSWORD)
  --runtime-access-key   Access key to create (default: celer-pkgcache)
  --rotate               Replace the secret key of an existing access key

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

if [ ! -f ./mc ]; then
    echo "Warning: mc bin not found, downloading..."
    curl -fL https://dl.min.io/aistor/mc/release/linux-amd64/mc -o ./mc
    chmod +x ./mc
fi

# The policy file next to this script is the source of truth; fail early if it
# is missing rather than at step 2/3.
if [ ! -f "$POLICY_FILE" ]; then
    echo "missing policy file: $POLICY_FILE" >&2
    exit 1
fi

SECRET="$(openssl rand -hex 20)"

# A temporary mc config dir keeps the administrator's own ~/.mc untouched, and
# the wrapper turns every call below into a one-liner.
CONF_DIR="$(mktemp -d)"
trap 'rm -rf "$CONF_DIR"' EXIT
mc() { command ./mc --config-dir "$CONF_DIR" --no-color "$@"; }

# Initialise the temp config dir silently, so the first real command does not
# print mc's "Configuration written to ..." notices.
mc alias list >/dev/null 2>&1 || true

echo "==> 1/3 bucket ${BUCKET} with object locking (implies versioning)"
mc alias set "$ALIAS" "$HOST" "$ADMIN_USER" "$ADMIN_PASSWORD"
mc mb --with-lock --ignore-existing "${ALIAS}/${BUCKET}"

echo "==> 2/3 policy ${POLICY_NAME} from ${POLICY_FILE}"
# `mc admin policy create` is an upsert, so the JSON file stays the source of
# truth. Removing first is not an option: MinIO refuses to remove a policy that
# is still attached to a user.
mc admin policy create "$ALIAS" "$POLICY_NAME" "$POLICY_FILE"

echo "==> 3/3 access key ${ACCESS_KEY}"
if mc admin user info "$ALIAS" "$ACCESS_KEY" >/dev/null 2>&1 && [[ "$ROTATE" == false ]]; then
    echo "    '${ACCESS_KEY}' already exists, secret key unchanged; pass --rotate to replace it"
    exit 0
fi

# `mc admin user add` is an upsert: it creates the user, or replaces the secret
# key of an existing one.
mc admin user add "$ALIAS" "$ACCESS_KEY" "$SECRET"
mc admin policy attach "$ALIAS" "$POLICY_NAME" --user "$ACCESS_KEY"

cat <<EOF

Access key ready. Point celer at the cache:

  celer configure --pkgcache-minio-host=${HOST} \\
    --pkgcache-minio-access-key=${ACCESS_KEY} \\
    --pkgcache-minio-secret-key=${SECRET}
EOF
