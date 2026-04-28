#!/bin/bash
#
# Copyright (C) 2026  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Organize Breakpad .sym file into proper directory structure
#
# Usage: ./organize_symbols.sh <rv.sym> <output_dir>
#

set -e

SYM_FILE="$1"
OUTPUT_DIR="$2"

if [ ! -f "$SYM_FILE" ]; then
    echo "ERROR: Symbol file not found: $SYM_FILE"
    exit 1
fi

# Extract MODULE_ID and MODULE_NAME from first line of .sym file
# Format: MODULE mac arm64 MODULE_ID MODULE_NAME
MODULE_ID=$(head -1 "$SYM_FILE" | awk '{print $4}')
MODULE_NAME=$(head -1 "$SYM_FILE" | awk '{print $5}')

# Create directory structure
SYMBOL_DIR="$OUTPUT_DIR/$MODULE_NAME/$MODULE_ID"
mkdir -p "$SYMBOL_DIR"

# Copy symbol file
cp "$SYM_FILE" "$SYMBOL_DIR/$MODULE_NAME.sym"

echo "Organized symbols: $SYMBOL_DIR/$MODULE_NAME.sym"
