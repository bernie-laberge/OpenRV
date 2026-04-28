#!/bin/sh
# Thin wrapper around crashpad_handler that redirects its stderr to a log file.
#
# On Linux, Crashpad logs benign ELF-reader diagnostics ("tag not found",
# "read out of range") for every VDSO / anonymous mapping it inspects when
# generating a crash dump.  These messages are harmless but flood the
# terminal.  Redirecting stderr here keeps the terminal clean without losing
# the information — the log file is placed next to the crash dumps so it
# is available for debugging when needed.
#
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG_DIR="${RV_CRASH_DUMPS_DIR:-${XDG_DATA_HOME:-$HOME/.local/share}/RV/Crashes}"
mkdir -p "$LOG_DIR"
exec "${SCRIPT_DIR}/crashpad_handler" "$@" 2>>"${LOG_DIR}/crashpad_handler.log"
