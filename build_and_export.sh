#!/usr/bin/env bash

set -euo pipefail

readonly repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly rom_path="$repo_dir/pokeemerald.gba"
destination_dir="${1:-/mnt/d/DEV/Projects/pokeemerald-ex-rando-nuz-root/playtest}"
temp_path=""

if (( $# > 1 )); then
    echo "Usage: $0 [destination-directory]" >&2
    exit 2
fi

if ! make -C "$repo_dir" -j2; then
    echo "Build failed; ROM was not exported." >&2
    exit 1
fi

if [[ ! -f "$rom_path" ]]; then
    echo "Build succeeded but ROM was not found: $rom_path" >&2
    exit 1
fi

if ! mkdir -p -- "$destination_dir"; then
    echo "Failed to create export directory: $destination_dir" >&2
    exit 1
fi

if ! destination_dir="$(cd -- "$destination_dir" && pwd -P)"; then
    echo "Failed to resolve export directory: $destination_dir" >&2
    exit 1
fi

cleanup()
{
    if [[ -n "$temp_path" ]]; then
        rm -f -- "$temp_path"
    fi
}
trap cleanup EXIT

if ! temp_path="$(mktemp "$destination_dir/.pokeemerald-export.XXXXXX")"; then
    echo "Failed to create a temporary export file in: $destination_dir" >&2
    exit 1
fi

if ! cp -- "$rom_path" "$temp_path"; then
    echo "Failed to copy ROM to export directory: $destination_dir" >&2
    exit 1
fi

timestamp="$(date '+%Y-%m-%d_%H-%M-%S')"
export_path="$destination_dir/pokeemerald-$timestamp.gba"
suffix=1

while ! ln -- "$temp_path" "$export_path" 2>/dev/null; do
    if [[ ! -e "$export_path" ]]; then
        echo "Failed to create export: $export_path" >&2
        exit 1
    fi

    export_path="$destination_dir/pokeemerald-$timestamp-$suffix.gba"
    ((suffix += 1))
done

if ! rm -- "$temp_path"; then
    echo "Export created, but temporary file cleanup failed: $temp_path" >&2
    exit 1
fi
temp_path=""

echo "$export_path"
