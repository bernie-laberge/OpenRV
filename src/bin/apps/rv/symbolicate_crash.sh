#!/bin/bash
#
# Copyright (C) 2026  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Symbolicate RV crash dumps using Breakpad
#
# Usage: ./symbolicate_crash.sh <crash.dmp> [output.txt]
#

set -e

# Get the directory where this script lives
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Paths
MINIDUMP_STACKWALK="${SCRIPT_DIR}/minidump_stackwalk"
# Breakpad symbols directory (symbols/MODULE_NAME/MODULE_ID/)
# On macOS the script lives inside RV.app/Contents/MacOS/ (3 levels deep);
# on Linux it lives directly in bin/ (1 level deep).
if [[ "$OSTYPE" == "darwin"* ]]; then
    SYMBOLS_DIR="${SCRIPT_DIR}/../../../symbols"
else
    SYMBOLS_DIR="${SCRIPT_DIR}/../symbols"
fi

# Check arguments
if [ $# -lt 1 ]; then
    echo "Usage: $0 <crash.dmp> [output.txt]"
    echo ""
    echo "Examples:"
    echo "  $0 ~/Library/Logs/TweakSoftware/Crashes/pending/crash.dmp"
    echo "  $0 crash.dmp crash_report.txt"
    exit 1
fi

CRASH_DMP="$1"
OUTPUT="${2:-crash_report.txt}"

# Check if crash dump exists
if [ ! -f "$CRASH_DMP" ]; then
    echo "ERROR: Crash dump not found: $CRASH_DMP"
    exit 1
fi

# Check if minidump_stackwalk exists
if [ ! -f "$MINIDUMP_STACKWALK" ]; then
    echo "ERROR: minidump_stackwalk not found at: $MINIDUMP_STACKWALK"
    echo "Make sure Breakpad is built as part of the RV build."
    exit 1
fi

# Check if symbols directory exists
if [ ! -d "$SYMBOLS_DIR" ]; then
    echo "ERROR: Symbols directory not found at: $SYMBOLS_DIR"
    echo "Make sure RV was built with Breakpad symbol generation enabled."
    exit 1
fi

# Try to find minidump_dump tool
MINIDUMP_DUMP=""
# Check common locations relative to the script
for path in \
    "${SCRIPT_DIR}/minidump_dump" \
    "${SCRIPT_DIR}/../../../../RV_DEPS_BREAKPAD/install/bin/minidump_dump" \
    "${SCRIPT_DIR}/../../../_build_debug/RV_DEPS_BREAKPAD/install/bin/minidump_dump" \
    "${SCRIPT_DIR}/../../../../_build_debug/RV_DEPS_BREAKPAD/install/bin/minidump_dump" \
    "${SCRIPT_DIR}/../../../../../_build_debug/RV_DEPS_BREAKPAD/install/bin/minidump_dump"
do
    if [ -f "$path" ]; then
        MINIDUMP_DUMP="$path"
        break
    fi
done

if [ -z "$MINIDUMP_DUMP" ]; then
    echo "WARNING: minidump_dump not found - Crashpad annotations will not be extracted"
    echo ""
fi

# Symbolicate
echo "Symbolicating crash dump..."
echo "  Input:   $CRASH_DMP"
echo "  Output:  $OUTPUT"
echo "  Symbols: $SYMBOLS_DIR"
echo ""

"$MINIDUMP_STACKWALK" "$CRASH_DMP" "$SYMBOLS_DIR" > "$OUTPUT"

# Append Crashpad annotations if minidump_dump is available
if [ -n "$MINIDUMP_DUMP" ] && [ -f "$MINIDUMP_DUMP" ]; then
    echo "" >> "$OUTPUT"
    echo "========================================" >> "$OUTPUT"
    echo "Crashpad Annotations" >> "$OUTPUT"
    echo "========================================" >> "$OUTPUT"
    echo "" >> "$OUTPUT"

    # Extract crashpad_annotations from minidump
    ANNOTATIONS=$("$MINIDUMP_DUMP" "$CRASH_DMP" 2>/dev/null | \
        grep -E 'crashpad_annotations\[' | \
        sed 's/.*crashpad_annotations\["\([^"]*\)"\].*= \(.*\)/\1: \2/')

    if [ -n "$ANNOTATIONS" ]; then
        echo "$ANNOTATIONS" >> "$OUTPUT"
    else
        echo "No Crashpad annotations found in minidump" >> "$OUTPUT"
    fi

    # Also extract simple_annotations for context
    echo "" >> "$OUTPUT"
    echo "Simple Annotations:" >> "$OUTPUT"
    "$MINIDUMP_DUMP" "$CRASH_DMP" 2>/dev/null | \
        grep -E 'simple_annotations\[' | \
        sed 's/.*simple_annotations\["\([^"]*\)"\] = \(.*\)/\1: \2/' >> "$OUTPUT"
fi

echo "✅ Symbolication complete!"
echo ""
echo "View the report:"
echo "  cat $OUTPUT"
echo ""
echo "Or search for your crash:"
echo "  grep -A 20 'Thread 0 (crashed)' $OUTPUT"
echo ""
echo "View Mu script info:"
echo "  grep -A 10 'Crashpad Annotations' $OUTPUT"
