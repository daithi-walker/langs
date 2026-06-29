#!/usr/bin/env bash
# run_all.sh — build and run benchmarks for one or all examples.
#
# Usage:
#   ./run_all.sh                  run all known examples
#   ./run_all.sh bounce           run only the bounce example
#   ./run_all.sh wave_packet      run only the wave_packet example
#
# Output: results/<example>/<lang>.json  (one JSON object per file)
# Each binary prints one JSON line:
#   {"language":"<lang>","example":"<example>","ns_per_op":<float>,"iterations":<int>}

set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")" && pwd)"
LANGS_ROOT="${BENCH_DIR}/.."
CARGO="${HOME}/.cargo/bin/cargo"

# Ordered list of all known examples
ALL_EXAMPLES=(bounce wave_packet nbody)

# Which examples to run
if [ "${1:-}" != "" ]; then
  EXAMPLES=("$1")
else
  EXAMPLES=("${ALL_EXAMPLES[@]}")
fi

# ---------------------------------------------------------------------------
# Helper: find the example directory under any category (physics/, qc/, etc.)
# ---------------------------------------------------------------------------
find_dir() {
  local lang="$1" example="$2"
  local base="${LANGS_ROOT}/${lang}"
  # Search one level of subdirectories (physics/, qc/, ...)
  for d in "${base}"/*/; do
    if [ -d "${d}${example}" ]; then
      echo "${d}${example}"
      return
    fi
  done
}

# ---------------------------------------------------------------------------
# Run one example across all languages
# ---------------------------------------------------------------------------
run_example() {
  local EXAMPLE="$1"
  local RESULTS_DIR="${BENCH_DIR}/results/${EXAMPLE}"
  mkdir -p "$RESULTS_DIR"

  echo "==> Benchmarking: ${EXAMPLE}"
  echo ""

  # --- C ---
  C_DIR="$(find_dir c "$EXAMPLE")"
  if [ -n "$C_DIR" ] && [ -d "$C_DIR" ]; then
    echo "--- C ---"
    make -C "$C_DIR" bench --silent
    "${C_DIR}/bench_${EXAMPLE}" | tee "${RESULTS_DIR}/c.json"
  else
    echo "Skipping C (no directory found)"
  fi

  # --- Go ---
  GO_DIR="$(find_dir go "$EXAMPLE")"
  if [ -n "$GO_DIR" ] && [ -d "$GO_DIR" ]; then
    echo "--- Go ---"
    (cd "$GO_DIR" && go run . --bench) | tee "${RESULTS_DIR}/go.json"
  else
    echo "Skipping Go (no directory found)"
  fi

  # --- Rust ---
  RUST_DIR="$(find_dir rust "$EXAMPLE")"
  if [ -n "$RUST_DIR" ] && [ -d "$RUST_DIR" ]; then
    echo "--- Rust ---"
    (cd "$RUST_DIR" && "$CARGO" build --release --bin "bench_${EXAMPLE}" -q 2>/dev/null && \
     "./target/release/bench_${EXAMPLE}") | tee "${RESULTS_DIR}/rust.json"
  else
    echo "Skipping Rust (no directory found)"
  fi

  # --- Java ---
  JAVA_DIR="$(find_dir java "$EXAMPLE")"
  if [ -n "$JAVA_DIR" ] && [ -d "$JAVA_DIR" ]; then
    echo "--- Java ---"
    make -C "$JAVA_DIR" bench --silent | tee "${RESULTS_DIR}/java.json"
  else
    echo "Skipping Java (no directory found)"
  fi

  # --- Python (pure) ---
  PY_DIR="$(find_dir python "$EXAMPLE")"
  if [ -n "$PY_DIR" ] && [ -d "$PY_DIR" ] && [ -f "${PY_DIR}/bench.py" ]; then
    echo "--- Python (pure) ---"
    (cd "$PY_DIR" && python3 bench.py) | tee "${RESULTS_DIR}/python.json"
  else
    echo "Skipping Python/pure (no bench.py found)"
  fi

  # --- Python (numpy) ---
  if [ -n "${PY_DIR:-}" ] && [ -d "${PY_DIR:-}" ] && [ -f "${PY_DIR}/bench_numpy.py" ]; then
    echo "--- Python (numpy) ---"
    (cd "$PY_DIR" && python3 bench_numpy.py) | tee "${RESULTS_DIR}/python_numpy.json"
  fi

  # --- Node.js ---
  NODE_DIR="$(find_dir nodejs "$EXAMPLE")"
  if [ -n "$NODE_DIR" ] && [ -d "$NODE_DIR" ]; then
    echo "--- Node.js ---"
    (cd "$NODE_DIR" && npm install --silent 2>/dev/null && \
     npx ts-node src/bench.ts 2>/dev/null) | tee "${RESULTS_DIR}/nodejs.json"
  else
    echo "Skipping Node.js (no directory found)"
  fi

  echo ""
  echo "Results in ${RESULTS_DIR}/"
  echo ""
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
for ex in "${EXAMPLES[@]}"; do
  run_example "$ex"
done

echo "==> All done. Generate the report with:"
echo "    python3 ${BENCH_DIR}/report.py"
