#!/bin/bash
#
# Copyright (C) 2026  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Find source code locations for crash addresses in symbolicated crash reports
#
# Usage: ./locate_crash_source.sh <crash_report.txt> [frame_number]
#
# Examples:
#   ./locate_crash_source.sh crash_report.txt           # Analyze all frames
#   ./locate_crash_source.sh crash_report.txt 0         # Analyze only frame 0
#

set -e

# Get the directory where this script lives
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <crash_report.txt> [frame_number]"
    echo ""
    echo "Examples:"
    echo "  $0 crash_report.txt           # Analyze all frames in crashed thread"
    echo "  $0 crash_report.txt 0         # Analyze only frame 0"
    echo "  $0 crash_report.txt 0-5       # Analyze frames 0 through 5"
    exit 1
fi

CRASH_REPORT="$1"
FRAME_FILTER="$2"

# Check if crash report exists
if [ ! -f "$CRASH_REPORT" ]; then
    echo "ERROR: Crash report not found: $CRASH_REPORT"
    exit 1
fi

# Check if atos is available (macOS only)
if ! command -v atos &> /dev/null; then
    echo "ERROR: 'atos' command not found. This script requires macOS."
    exit 1
fi

# Find the dSYM directory relative to the script
DSYM_BASE="${SCRIPT_DIR}/../../../stage/app"
if [ ! -d "$DSYM_BASE" ]; then
    # Try alternative path (if running from installed location)
    DSYM_BASE="${SCRIPT_DIR}/.."
fi

echo "=========================================="
echo "Source Location Finder"
echo "=========================================="
echo ""
echo "Crash Report: $CRASH_REPORT"
echo "dSYM Base:    $DSYM_BASE"
echo ""

# Extract module load addresses from crash report
# Format: 0x1000a4000 - 0x100537fff +RV arm64 <8fc63f63-638a-3a89-a7aa-e2b4f1c436e3> /path/to/RV
# Using arrays instead of associative arrays for bash 3.2 compatibility
MODULE_DATA=()

while IFS= read -r line; do
    # Match lines like: 0x1000a4000 - 0x100537fff +RV arm64 <uuid> /path/to/binary
    # Use grep for more portable regex matching
    if echo "$line" | grep -qE '^ *0x[0-9a-f]+ *- *0x[0-9a-f]+ +\+?[^ ]+ +(arm64|x86_64) +<[^>]+> +'; then
        # Extract fields using sed
        load_addr=$(echo "$line" | sed -E 's/^ *0x([0-9a-f]+) *-.*/0x\1/')
        module_name=$(echo "$line" | sed -E 's/^[^-]+-[^+]*\+?([^ ]+) .*/\1/')
        arch=$(echo "$line" | sed -E 's/^.* (arm64|x86_64) .*/\1/')
        path=$(echo "$line" | sed -E 's/^.*<[^>]+> +(.*)/\1/')

        # Store as "module_name|load_addr|arch|path"
        MODULE_DATA+=("$module_name|$load_addr|$arch|$path")
    fi
done < "$CRASH_REPORT"

if [ ${#MODULE_DATA[@]} -eq 0 ]; then
    echo "ERROR: Could not find module load addresses in crash report"
    echo "Make sure this is a symbolicated crash report from symbolicate_crash.sh"
    exit 1
fi

echo "Found ${#MODULE_DATA[@]} loaded modules"
echo ""

# Helper function to get module info
get_module_info() {
    local search_module="$1"
    local field="$2"  # "load_addr", "arch", or "path"

    for entry in "${MODULE_DATA[@]}"; do
        IFS='|' read -r module_name load_addr arch path <<< "$entry"
        if [ "$module_name" = "$search_module" ]; then
            case "$field" in
                "load_addr") echo "$load_addr" ;;
                "arch") echo "$arch" ;;
                "path") echo "$path" ;;
            esac
            return
        fi
    done
}

# Function to convert address using atos
locate_address() {
    local module="$1"
    local function="$2"
    local offset="$3"
    local frame_num="$4"

    local load_addr=$(get_module_info "$module" "load_addr")
    local module_path=$(get_module_info "$module" "path")
    local arch=$(get_module_info "$module" "arch")

    if [ -z "$load_addr" ]; then
        echo "  [Frame $frame_num] WARNING: No load address found for module: $module"
        return
    fi

    # Try to find dSYM file
    local dsym_file=""
    local base_name=$(basename "$module_path")

    # Look for dSYM in common locations
    for dsym_path in \
        "${DSYM_BASE}/${base_name}.dSYM/Contents/Resources/DWARF/${base_name}" \
        "${DSYM_BASE}/../${base_name}.dSYM/Contents/Resources/DWARF/${base_name}" \
        "${module_path}.dSYM/Contents/Resources/DWARF/${base_name}" \
        "${module_path%/*}/../${base_name}.dSYM/Contents/Resources/DWARF/${base_name}"
    do
        if [ -f "$dsym_path" ]; then
            dsym_file="$dsym_path"
            break
        fi
    done

    # If no dSYM, try the binary itself
    if [ -z "$dsym_file" ] && [ -f "$module_path" ]; then
        dsym_file="$module_path"
    fi

    if [ -z "$dsym_file" ]; then
        echo "  [Frame $frame_num] WARNING: No dSYM found for: $module"
        echo "              Function: $function + $offset"
        return
    fi

    # Calculate actual address (load_addr is already in hex with 0x prefix)
    # offset comes as "0x5dc" format
    local actual_addr=$(printf "0x%x" $((load_addr + offset)))

    # Use atos to find source location
    local source_info=$(atos -o "$dsym_file" -arch "$arch" -l "$load_addr" "$actual_addr" 2>/dev/null)

    if [ -n "$source_info" ] && [[ ! "$source_info" =~ "0x" ]]; then
        echo "  [Frame $frame_num] $module!$function + $offset"
        echo "              → $source_info"
    else
        # atos failed, just show what we know
        echo "  [Frame $frame_num] $module!$function + $offset"
        echo "              (Source location not available)"
    fi
}

# Parse stack frames from crashed thread
# Format: 0  mio_ffmpeg_commercial.dylib!TwkMovie::MovieFFMpegReader::decodeImageAtFrame(int, TwkMovie::VideoTrack*) + 0x5dc

echo "=========================================="
echo "Analyzing Crashed Thread Stack Frames"
echo "=========================================="
echo ""

in_crashed_thread=0
frame_count=0

while IFS= read -r line; do
    # Detect start of crashed thread
    if echo "$line" | grep -q '^Thread [0-9]* (crashed)'; then
        in_crashed_thread=1
        continue
    fi

    # Detect end of thread section
    if [ $in_crashed_thread -eq 1 ] && echo "$line" | grep -q '^Thread [0-9]*' && ! echo "$line" | grep -q 'crashed'; then
        break
    fi

    # Parse stack frame if we're in crashed thread
    if [ $in_crashed_thread -eq 1 ]; then
        # Match: "0  module!function + 0x123"
        # Use grep for portable matching
        if echo "$line" | grep -qE '^ *[0-9]+ +[^!]+!.+ \+ +0x[0-9a-f]+'; then
            frame_num=$(echo "$line" | sed -E 's/^ *([0-9]+) .*/\1/')
            module=$(echo "$line" | sed -E 's/^ *[0-9]+ +([^!]+)!.*/\1/')
            function=$(echo "$line" | sed -E 's/^ *[0-9]+ +[^!]+!(.+) \+ +0x[0-9a-f]+/\1/')
            offset=$(echo "$line" | sed -E 's/^.* \+ +(0x[0-9a-f]+)/\1/')

            # Apply frame filter if specified
            if [ -n "$FRAME_FILTER" ]; then
                if echo "$FRAME_FILTER" | grep -qE '^[0-9]+-[0-9]+$'; then
                    # Range specified (e.g., "0-5")
                    start=$(echo "$FRAME_FILTER" | sed 's/-.*//')
                    end=$(echo "$FRAME_FILTER" | sed 's/.*-//')
                    if [ "$frame_num" -lt "$start" ] || [ "$frame_num" -gt "$end" ]; then
                        continue
                    fi
                elif [ "$FRAME_FILTER" != "$frame_num" ]; then
                    # Single frame specified
                    continue
                fi
            fi

            locate_address "$module" "$function" "$offset" "$frame_num"
            frame_count=$((frame_count + 1))
        fi
    fi
done < "$CRASH_REPORT"

if [ $frame_count -eq 0 ]; then
    echo "No stack frames found to analyze."
    echo ""
    echo "Make sure:"
    echo "  1. The crash report is symbolicated (use symbolicate_crash.sh first)"
    echo "  2. The crashed thread section exists"
    echo "  3. Stack frames have the format: '0  module!function + 0x123'"
else
    echo ""
    echo "=========================================="
    echo "Analysis complete: $frame_count frame(s) processed"
    echo "=========================================="
fi
