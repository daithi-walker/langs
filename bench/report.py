#!/usr/bin/env python3
"""Generate an HTML benchmark report from JSON result files.

Usage:
    python3 report.py [example]     (default: bounce)

Reads:  results/<example>/<lang>.json
Writes: results/<example>/report.html

Chart convention: longer bar = faster (more calls/sec).
Multiplier shown as slowdown relative to the fastest language.
"""

import json
import sys
from pathlib import Path

EXAMPLE = sys.argv[1] if len(sys.argv) > 1 else "bounce"
RESULTS = Path(__file__).parent / "results" / EXAMPLE

LANG_DISPLAY = {
    "c":            "C",
    "go":           "Go",
    "rust":         "Rust",
    "java":         "Java",
    "python":       "Python (pure)",
    "python_numpy": "Python (numpy)",
    "nodejs":       "Node.js (TS)",
}

LANG_COLOR = {
    "c":            "#5b9bd5",
    "go":           "#00acd7",
    "rust":         "#ce422b",
    "java":         "#f89820",
    "python":       "#ffd43b",
    "python_numpy": "#4caf50",
    "nodejs":       "#68a063",
}


def load_results() -> list[dict]:
    results = []
    for path in sorted(RESULTS.glob("*.json")):
        if path.stem == "report":
            continue
        lang = path.stem
        try:
            data = json.loads(path.read_text())
            data["display"] = LANG_DISPLAY.get(lang, data.get("lang", lang))
            data["color"]   = LANG_COLOR.get(lang, "#aaaaaa")
            results.append(data)
        except Exception as e:
            print(f"Warning: could not read {path}: {e}", file=sys.stderr)
    return results


def bar_html(results: list[dict]) -> str:
    if not results:
        return "<p>No results found.</p>"

    # Sort fastest first (lowest ns = fastest)
    sorted_results = sorted(results, key=lambda r: r["mean_ns"])
    fastest_ns = sorted_results[0]["mean_ns"]

    # Bar width: fastest gets 100%, others scaled proportionally
    # (longer bar = faster — throughput view)
    rows = ""
    for r in sorted_results:
        pct      = fastest_ns / r["mean_ns"] * 100   # inverted: fastest = 100%
        slowdown = r["mean_ns"] / fastest_ns
        label    = f"{r['mean_ns']:.2f} ns/call"
        note     = r.get("note", "")
        mult_str = "fastest" if slowdown < 1.01 else f"{slowdown:.1f}× slower"
        note_html = f' <span class="note-tag">{note}</span>' if note else ""

        rows += f"""
        <tr>
          <td class="lang">{r['display']}{note_html}</td>
          <td class="bar-cell">
            <div class="bar-track">
              <div class="bar" style="width:{pct:.1f}%;background:{r['color']}">
                <span class="bar-label">{label}</span>
              </div>
            </div>
          </td>
          <td class="mult">{mult_str}</td>
        </tr>"""

    return f"""
    <p class="chart-note">Longer bar = faster &nbsp;|&nbsp; sorted fastest → slowest</p>
    <table class='results'>{rows}</table>"""


def generate_html(results: list[dict]) -> str:
    import platform
    import subprocess
    try:
        cpu = subprocess.check_output(
            ["sysctl", "-n", "machdep.cpu.brand_string"], text=True
        ).strip()
    except Exception:
        cpu = platform.processor() or "unknown"

    bars = bar_html(results)

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Benchmark — {EXAMPLE}</title>
<style>
  body        {{ font-family: -apple-system, sans-serif; max-width: 920px;
                margin: 40px auto; padding: 0 20px; color: #222; }}
  h1          {{ font-size: 1.6rem; margin-bottom: 0.2rem; }}
  .meta       {{ color: #666; font-size: 0.9rem; margin-bottom: 2rem; }}
  table.results {{ width: 100%; border-collapse: collapse; margin-top: 0.5rem; }}
  td          {{ padding: 7px 8px; vertical-align: middle; }}
  td.lang     {{ font-weight: 600; white-space: nowrap; width: 160px; font-size: 0.95rem; }}
  td.bar-cell {{ width: 100%; }}
  td.mult     {{ white-space: nowrap; color: #888; font-size: 0.82rem;
                text-align: right; width: 110px; }}
  .bar-track  {{ background: #f0f0f0; border-radius: 4px; width: 100%; height: 30px;
                display: flex; align-items: center; }}
  .bar        {{ height: 30px; border-radius: 4px; display: flex; align-items: center;
                min-width: 4px; transition: width 0.3s; }}
  .bar-label  {{ color: #fff; font-size: 0.78rem; padding: 0 10px;
                text-shadow: 0 1px 2px rgba(0,0,0,.5); white-space: nowrap; }}
  .chart-note {{ font-size: 0.8rem; color: #999; margin-bottom: 0.3rem; }}
  .note-tag   {{ font-weight: normal; font-size: 0.75rem; color: #888;
                background: #f0f0f0; border-radius: 3px; padding: 1px 5px;
                margin-left: 4px; }}
  .notes      {{ margin-top: 2rem; font-size: 0.85rem; color: #555;
                border-top: 1px solid #eee; padding-top: 1rem; }}
  .notes li   {{ margin-bottom: 0.4rem; }}
  hr          {{ border: none; border-top: 1px solid #eee; margin: 1.5rem 0; }}
</style>
</head>
<body>
<h1>Benchmark: <code>{EXAMPLE}</code></h1>
<p class="meta">
  Function: <code>ball.update()</code> — position + wall reflection per call<br>
  Machine: {cpu}
</p>
<hr>
{bars}
<ul class="notes">
  <li><strong>C, Rust</strong>: compiled to native ARM64 at <code>-O2</code>.</li>
  <li><strong>Rust</strong>: uses <code>std::hint::black_box</code> to prevent loop elimination.</li>
  <li><strong>Java</strong>: 100,000 warm-up iterations before timing to let the JIT compile.</li>
  <li><strong>Go, Node.js</strong>: also JIT-compiled at runtime; Go's is simpler, V8 (Node) is highly optimised.</li>
  <li><strong>Python (pure)</strong>: CPython interpreter, plain Python loop — one ball, 1M iterations.</li>
  <li><strong>Python (numpy)</strong>: 10,000 balls updated simultaneously per call via numpy's
      compiled C backend. Time shown is ns <em>per ball</em>, making it comparable to other rows.</li>
</ul>
</body>
</html>"""


def main() -> None:
    results = load_results()
    if not results:
        print(f"No result files found in {RESULTS}", file=sys.stderr)
        sys.exit(1)

    html = generate_html(results)
    out = RESULTS / "report.html"
    out.write_text(html)
    print(f"Report written to {out}")
    print(f"Open with: open {out}")


if __name__ == "__main__":
    main()
