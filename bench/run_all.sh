#!/usr/bin/env bash
# run_all.sh — build and run all language benchmarks for a given example.
#
# Usage:  ./run_all.sh [example]     (default: bounce)
# Output: results/<example>/<lang>.json  (one JSON object per language)
#
# Each language binary is expected to print one JSON line to stdout:
#   {"lang":"<lang>","example":"<example>","mean_ns":<float>,"n":<int>}

set -euo pipefail

EXAMPLE="${1:-bounce}"
RESULTS_DIR="$(dirname "$0")/results/${EXAMPLE}"
LANGS_ROOT="$(dirname "$0")/.."

mkdir -p "$RESULTS_DIR"

echo "==> Benchmarking example: ${EXAMPLE}"
echo ""

# ---------------------------------------------------------------------------
# C
# ---------------------------------------------------------------------------
C_DIR="${LANGS_ROOT}/c/physics/${EXAMPLE}"
if [ -d "$C_DIR" ]; then
  echo "--- C ---"
  make -C "$C_DIR" bench --silent
  "$C_DIR/bench_${EXAMPLE}" | tee "${RESULTS_DIR}/c.json"
else
  echo "Skipping C (no directory: $C_DIR)"
fi

# ---------------------------------------------------------------------------
# Go
# ---------------------------------------------------------------------------
GO_DIR="${LANGS_ROOT}/go/physics/${EXAMPLE}"
if [ -d "$GO_DIR" ]; then
  echo "--- Go ---"
  (cd "$GO_DIR" && go run . --bench) | tee "${RESULTS_DIR}/go.json"
else
  echo "Skipping Go (no directory: $GO_DIR)"
fi

# ---------------------------------------------------------------------------
# Rust
# ---------------------------------------------------------------------------
RUST_DIR="${LANGS_ROOT}/rust/physics/${EXAMPLE}"
if [ -d "$RUST_DIR" ]; then
  echo "--- Rust ---"
  (cd "$RUST_DIR" && cargo build --release --bin "bench_${EXAMPLE}" -q 2>/dev/null && \
   "./target/release/bench_${EXAMPLE}") | tee "${RESULTS_DIR}/rust.json"
else
  echo "Skipping Rust (no directory: $RUST_DIR)"
fi

# ---------------------------------------------------------------------------
# Java
# ---------------------------------------------------------------------------
JAVA_DIR="${LANGS_ROOT}/java/physics/${EXAMPLE}"
if [ -d "$JAVA_DIR" ]; then
  echo "--- Java ---"
  (cd "$JAVA_DIR" && javac -d out src/BallPhysics.java src/BenchBounce.java 2>/dev/null && java -cp out BenchBounce) | tee "${RESULTS_DIR}/java.json"
else
  echo "Skipping Java (no directory: $JAVA_DIR)"
fi

# ---------------------------------------------------------------------------
# Python (pure)
# ---------------------------------------------------------------------------
PY_DIR="${LANGS_ROOT}/python/physics/${EXAMPLE}"
if [ -d "$PY_DIR" ]; then
  echo "--- Python (pure) ---"
  (cd "$PY_DIR" && python3 bench.py) | tee "${RESULTS_DIR}/python.json"
else
  echo "Skipping Python (no directory: $PY_DIR)"
fi

# ---------------------------------------------------------------------------
# Python (numpy)
# ---------------------------------------------------------------------------
if [ -d "$PY_DIR" ] && [ -f "${PY_DIR}/bench_numpy.py" ]; then
  echo "--- Python (numpy) ---"
  (cd "$PY_DIR" && python3 bench_numpy.py) | tee "${RESULTS_DIR}/python_numpy.json"
fi

# ---------------------------------------------------------------------------
# Node.js
# ---------------------------------------------------------------------------
NODE_DIR="${LANGS_ROOT}/nodejs/physics/${EXAMPLE}"
if [ -d "$NODE_DIR" ]; then
  echo "--- Node.js ---"
  (cd "$NODE_DIR" && npx --yes ts-node src/bench.ts 2>/dev/null) | tee "${RESULTS_DIR}/nodejs.json"
else
  echo "Skipping Node.js (no directory: $NODE_DIR)"
fi

echo ""
echo "==> Results written to ${RESULTS_DIR}/"
echo "==> Run: python3 $(dirname "$0")/report.py ${EXAMPLE}  to generate HTML"
