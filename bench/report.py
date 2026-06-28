#!/usr/bin/env python3
"""Generate an HTML benchmark matrix report from JSON result files.

Usage:
    python3 report.py              generates matrix across all examples
    python3 report.py bounce       generates single-example report

Reads:  results/<example>/<lang>.json
Writes: results/report.html  (matrix)  or  results/<example>/report.html  (single)

Chart convention: longer bar = faster (throughput view).
"""

import json
import sys
import platform
import subprocess
from pathlib import Path

RESULTS_ROOT = Path(__file__).parent / "results"

LANG_ORDER = ["c", "rust", "go", "java", "python_numpy", "python", "nodejs"]

LANG_DISPLAY = {
    "c":            "C",
    "go":           "Go",
    "rust":         "Rust",
    "java":         "Java",
    "python":       "Python (pure)",
    "python_numpy": "Python (numpy)",
    "nodejs":       "Node.js",
}

LANG_COLOR = {
    "c":            "#5b9bd5",
    "rust":         "#ce422b",
    "go":           "#00acd7",
    "java":         "#f89820",
    "python_numpy": "#4caf50",
    "python":       "#ffd43b",
    "nodejs":       "#68a063",
}

EXAMPLE_LABEL = {
    "bounce":      "Bouncing ball physics",
    "wave_packet": "Wave packet TDSE step",
}


def cpu_name() -> str:
    try:
        return subprocess.check_output(
            ["sysctl", "-n", "machdep.cpu.brand_string"], text=True
        ).strip()
    except Exception:
        return platform.processor() or "unknown"


def load_example(example: str) -> dict[str, dict]:
    """Return {lang_key: result_dict} for one example."""
    d = RESULTS_ROOT / example
    if not d.is_dir():
        return {}
    results = {}
    for path in d.glob("*.json"):
        if path.stem == "report":
            continue
        try:
            data = json.loads(path.read_text())
            # Support both key names
            ns = data.get("ns_per_op") or data.get("mean_ns")
            if ns is not None:
                results[path.stem] = {"ns": ns, "raw": data}
        except Exception as e:
            print(f"Warning: {path}: {e}", file=sys.stderr)
    return results


def bar_row(lang: str, ns: float, fastest_ns: float, color: str) -> str:
    pct      = fastest_ns / ns * 100
    slowdown = ns / fastest_ns
    mult_str = "fastest" if slowdown < 1.01 else f"{slowdown:.1f}×"
    label    = f"{ns:,.0f} ns"
    return f"""
      <tr>
        <td class="lang">{LANG_DISPLAY.get(lang, lang)}</td>
        <td class="bar-cell">
          <div class="bar-track">
            <div class="bar" style="width:{pct:.1f}%;background:{color}">
              <span class="bar-label">{label}</span>
            </div>
          </div>
        </td>
        <td class="mult">{mult_str}</td>
      </tr>"""


def example_section(example: str, results: dict[str, dict]) -> str:
    if not results:
        return f"<p class='no-data'>No data for <code>{example}</code>.</p>"

    fastest_ns = min(r["ns"] for r in results.values())
    label = EXAMPLE_LABEL.get(example, example)

    rows = ""
    for lang in LANG_ORDER:
        if lang not in results:
            continue
        r = results[lang]
        rows += bar_row(lang, r["ns"], fastest_ns, LANG_COLOR.get(lang, "#aaa"))

    # Also render any langs not in LANG_ORDER
    for lang, r in sorted(results.items()):
        if lang not in LANG_ORDER:
            rows += bar_row(lang, r["ns"], fastest_ns, "#aaa")

    return f"""
  <section>
    <h2>{label} <code class="ex-tag">{example}</code></h2>
    <p class="chart-note">Longer bar = faster &nbsp;|&nbsp; sorted by definition order</p>
    <table class="results">{rows}
    </table>
  </section>"""


def generate_matrix_html(examples: list[str]) -> str:
    sections = ""
    for ex in examples:
        data = load_example(ex)
        sections += example_section(ex, data)

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Benchmark Matrix</title>
<style>
  body        {{ font-family: -apple-system, sans-serif; max-width: 960px;
                margin: 40px auto; padding: 0 24px; color: #222; }}
  h1          {{ font-size: 1.7rem; margin-bottom: 0.2rem; }}
  h2          {{ font-size: 1.1rem; margin: 2rem 0 0.3rem; color: #333; }}
  .meta       {{ color: #777; font-size: 0.88rem; margin-bottom: 2rem; }}
  .ex-tag     {{ font-size: 0.8rem; color: #888; background: #f3f3f3;
                border-radius: 4px; padding: 2px 7px; margin-left: 6px; }}
  section     {{ border-top: 1px solid #eee; padding-top: 0.5rem; margin-bottom: 1rem; }}
  table.results {{ width: 100%; border-collapse: collapse; margin-top: 0.4rem; }}
  td          {{ padding: 6px 8px; vertical-align: middle; }}
  td.lang     {{ font-weight: 600; white-space: nowrap; width: 150px; font-size: 0.9rem; }}
  td.bar-cell {{ width: 100%; }}
  td.mult     {{ white-space: nowrap; color: #999; font-size: 0.8rem;
                text-align: right; width: 80px; }}
  .bar-track  {{ background: #f0f0f0; border-radius: 4px; width: 100%; height: 28px;
                display: flex; align-items: center; }}
  .bar        {{ height: 28px; border-radius: 4px; display: flex; align-items: center;
                min-width: 4px; }}
  .bar-label  {{ color: #fff; font-size: 0.76rem; padding: 0 8px;
                text-shadow: 0 1px 2px rgba(0,0,0,.4); white-space: nowrap; }}
  .chart-note {{ font-size: 0.78rem; color: #bbb; margin: 0 0 0.2rem; }}
  .no-data    {{ color: #aaa; font-size: 0.85rem; }}
  footer      {{ margin-top: 3rem; font-size: 0.78rem; color: #bbb;
                border-top: 1px solid #eee; padding-top: 1rem; }}
</style>
</head>
<body>
<h1>Benchmark Matrix</h1>
<p class="meta">Machine: {cpu_name()}<br>
Longer bar = faster. Each cell shows ns/op for that language × example combination.</p>
{sections}
<footer>
  Generated by bench/report.py &nbsp;|&nbsp;
  Languages: C (clang -O2), Rust (release), Go, Java (JIT), Python numpy, Node.js (V8)
</footer>
</body>
</html>"""


def generate_markdown(examples: list[str]) -> str:
    """Generate a BENCHMARKS.md table for GitHub rendering."""
    cpu = cpu_name()
    lines = [
        "# Benchmark Results",
        "",
        f"Machine: `{cpu}`  ",
        "All times in **ns/op** (nanoseconds per operation). Lower is faster.",
        "",
        "> Generated by `bench/report.py`. Re-run with `bash bench/run_all.sh`.",
        "",
    ]

    for example in examples:
        results = load_example(example)
        if not results:
            continue
        label = EXAMPLE_LABEL.get(example, example)
        lines.append(f"## {label} (`{example}`)")
        lines.append("")

        # Header
        header_langs = [l for l in LANG_ORDER if l in results]
        header_langs += [l for l in sorted(results) if l not in LANG_ORDER]
        lines.append("| Language | ns/op | vs fastest |")
        lines.append("|----------|------:|-----------:|")

        fastest_ns = min(r["ns"] for r in results.values())
        for lang in header_langs:
            if lang not in results:
                continue
            ns = results[lang]["ns"]
            slowdown = ns / fastest_ns
            mult = "fastest" if slowdown < 1.01 else f"{slowdown:.1f}×"
            bar = "█" * int(fastest_ns / ns * 20)  # simple visual bar, max 20 chars
            fmt = f"{ns:,.1f}" if ns < 100 else f"{ns:,.0f}"
            lines.append(f"| {LANG_DISPLAY.get(lang, lang)} | {fmt} | {mult} |")

        lines.append("")

    lines += [
        "---",
        "",
        "### Notes",
        "- **C, Rust**: compiled to native ARM64 at `-O2` / `--release`.",
        "- **Rust** (`wave_packet`): uses `rustfft` with SIMD mixed-radix algorithm.",
        "- **C** (`wave_packet`): uses FFTW single-precision (`fftwf`) with `FFTW_MEASURE`.",
        "- **Python (numpy)**: vectorized via numpy's `pocketfft` C backend — no Python loops in hot path.",
        "- **Java, Go, Node.js** (`wave_packet`): pure Cooley-Tukey FFT, no SIMD.",
        "- **Java**: JIT-compiled; 200 warmup iterations before timing.",
        "- **Python (pure)** (`bounce`): CPython interpreter loop — one ball, 1M iterations.",
        "",
    ]
    return "\n".join(lines)


def main() -> None:
    if len(sys.argv) > 1:
        # Single example mode
        example = sys.argv[1]
        data = load_example(example)
        if not data:
            print(f"No results found for {example}", file=sys.stderr)
            sys.exit(1)
        examples = [example]
        html = generate_matrix_html(examples)
        out = RESULTS_ROOT / example / "report.html"
        out.parent.mkdir(parents=True, exist_ok=True)
    else:
        # Matrix mode — all examples that have results
        all_found = sorted(
            d.name for d in RESULTS_ROOT.iterdir()
            if d.is_dir() and any(d.glob("*.json"))
        )
        if not all_found:
            print(f"No results found in {RESULTS_ROOT}", file=sys.stderr)
            sys.exit(1)
        # Respect preferred order
        examples = [e for e in ["bounce", "wave_packet"] if e in all_found]
        examples += [e for e in all_found if e not in examples]
        html = generate_matrix_html(examples)
        out = RESULTS_ROOT / "report.html"

        # Write BENCHMARKS.md to repo root
        md_out = RESULTS_ROOT.parent.parent / "BENCHMARKS.md"
        md_out.write_text(generate_markdown(examples))
        print(f"Markdown written to {md_out}")

    out.write_text(html)
    print(f"HTML report written to {out}")
    print(f"Open with: open {out}")


if __name__ == "__main__":
    main()
