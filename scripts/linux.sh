#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_IMAGE="${SOTC_BUILD_IMAGE:-sotc-build:local}"
BUILD_CONTEXT=""
FORCE_BUILD=false

usage() {
    cat <<'EOF'
Usage: scripts/linux.sh [--build-image] COMMAND [ARG...]
       scripts/linux.sh --build-image

Run a command from the checkout root inside the Linux amd64 build image.
The image is built automatically when missing; --build-image rebuilds it.
Set SOTC_BUILD_IMAGE to use another local image.
EOF
}

cleanup() {
    if [[ -n "$BUILD_CONTEXT" ]]; then
        rm -rf "$BUILD_CONTEXT"
    fi
}
trap cleanup EXIT

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    usage
    exit 0
fi
if [[ "${1:-}" == "--build-image" ]]; then
    FORCE_BUILD=true
    shift
fi
if [[ "${1:-}" == "--" ]]; then
    shift
fi
if [[ $# -eq 0 && "$FORCE_BUILD" == false ]]; then
    usage >&2
    exit 2
fi
if ! command -v docker >/dev/null 2>&1; then
    echo "Docker is required to run the Linux build environment." >&2
    exit 1
fi

if [[ "$FORCE_BUILD" == true ]] || ! docker image inspect "$BUILD_IMAGE" >/dev/null 2>&1; then
    mkdir -p "$PROJECT_DIR/target"
    BUILD_CONTEXT="$(mktemp -d "$PROJECT_DIR/target/linux-docker-context.XXXXXX")"
    mkdir -p "$BUILD_CONTEXT/compiler"
    cp "$PROJECT_DIR/requirements.txt" "$BUILD_CONTEXT/requirements.txt"
    if [[ -f "$PROJECT_DIR/tools/cc/ee-gcc2.96/bin/ee-gcc" ]]; then
        cp -a "$PROJECT_DIR/tools/cc/ee-gcc2.96/." "$BUILD_CONTEXT/compiler/"
    fi
    docker build --platform linux/amd64 --tag "$BUILD_IMAGE" \
        --file "$PROJECT_DIR/scripts/Dockerfile" "$BUILD_CONTEXT"
fi
if [[ $# -eq 0 ]]; then
    exit 0
fi

mkdir -p "$PROJECT_DIR/target/linux-python"
MOUNTS=(
    --mount "type=bind,source=$PROJECT_DIR,target=/work"
    --mount "type=bind,source=$PROJECT_DIR/target/linux-python,target=/work/.python3"
)
if [[ -e "$PROJECT_DIR/.git" ]]; then
    MOUNTS+=(--mount "type=bind,source=$PROJECT_DIR/.git,target=/work/.git,readonly")
fi
docker run --rm --platform linux/amd64 "${MOUNTS[@]}" \
    --workdir /work "$BUILD_IMAGE" \
    bash -c 'set -e
        if [ ! -x tools/cc/ee-gcc2.96/bin/ee-gcc ]; then
            if [ ! -x /opt/ee-gcc2.96/bin/ee-gcc ]; then
                echo "The checkout and image have no usable EE compiler; rebuild with scripts/linux.sh --build-image." >&2
                exit 1
            fi
            mkdir -p tools/cc/ee-gcc2.96
            cp -a /opt/ee-gcc2.96/. tools/cc/ee-gcc2.96/
        fi
        if [ ! -x .python3/bin/python3 ]; then
            python3 -m venv --system-site-packages .python3
        fi
        export PATH="/work/.python3/bin:$PATH"
        exec "$@"' bash "$@"
