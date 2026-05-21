#!/usr/bin/env python3
"""
track_visualiser.py — Visualise a picoControl animation track file.

Usage:
    python3 track_visualiser.py Track-animalove.h channel_map.yaml [options]

Options:
    --animation  default|expo     Which animation to plot (default: default)
    --start N                     First step to show (default: 0)
    --end N                       Last step to show (default: all)
    --output FILE                 Save to PNG instead of showing interactively
"""

import re
import sys
import argparse
import yaml
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
parser = argparse.ArgumentParser(description="Visualise a picoControl animation track.")
parser.add_argument("track",       help="Path to Track-xxx.h file")
parser.add_argument("map",         help="Path to channel_map.yaml file")
parser.add_argument("--animation", default="default", choices=["default","expo"],
                    help="Which animation array to visualise (default: default)")
parser.add_argument("--start",     type=int, default=0,    help="First step to show")
parser.add_argument("--end",       type=int, default=None, help="Last step to show")
parser.add_argument("--output",    default=None,           help="Save plot to file")
args = parser.parse_args()

# ---------------------------------------------------------------------------
# Parse the track file
# ---------------------------------------------------------------------------
def parse_track(path, which="default"):
    with open(path) as f:
        src = f.read()
    pattern = (r"expoAnimation\s*\[.*?\]\s*PROGMEM\s*=\s*\{"
               if which == "expo" else
               r"defaultAnimation\s*\[.*?\]\s*PROGMEM\s*=\s*\{")
    m = re.search(pattern, src)
    if not m:
        sys.exit(f"ERROR: could not find {which}Animation array in {path}")
    body_start = m.end()
    depth = 1; i = body_start
    while i < len(src) and depth > 0:
        if src[i] == '{': depth += 1
        elif src[i] == '}': depth -= 1
        i += 1
    body = src[body_start:i-1]
    steps = []
    for row in re.findall(r'\{([^}]+)\}', body):
        vals = [int(v.strip()) for v in row.split(',') if v.strip()]
        if len(vals) == 9:
            steps.append(vals)
    return steps

steps = parse_track(args.track, args.animation)
start = args.start
end   = min(args.end if args.end is not None else len(steps)-1, len(steps)-1)
steps = steps[start:end+1]
times = [s / 20.0 for s in range(start, start + len(steps))]

print(f"Loaded {len(steps)} steps ({times[0]:.1f}s – {times[-1]:.1f}s) from {args.animation}Animation")

# ---------------------------------------------------------------------------
# Parse channel map
# ---------------------------------------------------------------------------
with open(args.map) as f:
    cmap = yaml.safe_load(f)

# ---------------------------------------------------------------------------
# Decode mux state
# ---------------------------------------------------------------------------
def get_mux_state(step, mux_ch):
    field = 5 + mux_ch // 4
    shift = (mux_ch % 4) * 2
    return (step[field] >> shift) & 0x03

# ---------------------------------------------------------------------------
# Build rows — one row per (mux, state) pair so states never share a row
# ---------------------------------------------------------------------------
analog_configs = cmap.get("analog", [])
switch_configs = cmap.get("switches", [])

# Analog rows: one per field, only if active
active_analogs = []
for an in analog_configs:
    fi   = an["field"]
    vals = [s[fi] for s in steps]
    if any(v not in (0, 127) for v in vals):
        active_analogs.append(an)

# Switch rows: one per (mux, state_value) pair, only if active
# Each row = dict with keys: mux, state, label, color
STATE_COLORS = {1: "#66BB6A", 2: "#FF7043", 3: "#42A5F5"}
ANALOG_COLORS = ["#2196F3","#4CAF50","#FF9800","#9C27B0","#F44336"]

active_rows = []
for sw in switch_configs:
    mux          = sw["mux"]
    chan_label   = sw.get("label", f"mux{mux}")
    state_labels = sw.get("states", {})
    for state_val, state_label in sorted(state_labels.items()):
        # Check if this specific state ever occurs
        active = [get_mux_state(s, mux) == state_val for s in steps]
        if any(active):
            active_rows.append({
                "mux":        mux,
                "state":      state_val,
                "row_label":  f"mux{mux} s{state_val}\n{state_label}",
                "act_label":  state_label,
                "color":      STATE_COLORS.get(state_val, "#B0BEC5"),
                "active":     active,
            })

n_analog = len(active_analogs)
n_switch = len(active_rows)
n_rows   = n_analog + n_switch

if n_rows == 0:
    sys.exit("No active channels found in this range.")

# ---------------------------------------------------------------------------
# Plot
# ---------------------------------------------------------------------------
fig_height = max(6, n_rows * 0.65 + 2)
fig, axes  = plt.subplots(n_rows, 1, figsize=(16, fig_height), sharex=True)
if n_rows == 1:
    axes = [axes]

vehicle = cmap.get("vehicle", "unknown")
fig.suptitle(
    f"{vehicle} — {args.animation}Animation  [{times[0]:.1f}s – {times[-1]:.1f}s]",
    fontsize=12, fontweight="bold"
)

row = 0

# ── Analog rows ──────────────────────────────────────────────────────────────
for idx, an in enumerate(active_analogs):
    ax    = axes[row]; row += 1
    fi    = an["field"]
    label = an["label"]
    vals  = [s[fi] for s in steps]
    color = ANALOG_COLORS[idx % len(ANALOG_COLORS)]

    ax.plot(times, vals, color=color, linewidth=0.8)
    ax.axhline(127, color="gray", linewidth=0.4, linestyle=":")
    ax.set_ylabel(label, fontsize=7, rotation=0, ha="right", va="center", labelpad=70)
    ax.set_ylim(-5, 260)
    ax.set_yticks([0, 127, 255])
    ax.tick_params(axis="y", labelsize=6)
    ax.grid(axis="x", alpha=0.3)

# ── Switch rows — one per (mux, state) ──────────────────────────────────────
for sw_row in active_rows:
    ax     = axes[row]; row += 1
    active = sw_row["active"]
    color  = sw_row["color"]
    label  = sw_row["act_label"]

    # Draw filled spans
    i = 0
    while i < len(active):
        if active[i]:
            j = i
            while j < len(active) and active[j]:
                j += 1
            t0 = times[i]
            t1 = times[j] if j < len(times) else times[-1] + 0.05

            ax.axvspan(t0, t1, alpha=0.75, color=color, linewidth=0)

            # Label inside span if wide enough
            width = t1 - t0
            mid   = (t0 + t1) / 2
            if width > 0.3:
                ax.text(mid, 0.5, label,
                        ha="center", va="center", fontsize=6.5, fontweight="bold",
                        transform=ax.get_xaxis_transform(), clip_on=True)

            # Start marker + annotation
            ax.axvline(t0, color="darkgreen", linewidth=0.8, alpha=0.7)
            ax.annotate(f"▶ {t0:.1f}s",
                        xy=(t0, 0.97), xycoords=("data","axes fraction"),
                        fontsize=5, color="darkgreen", rotation=90,
                        ha="right", va="top")

            # Stop marker + annotation
            ax.axvline(t1, color="darkred", linewidth=0.6, alpha=0.5, linestyle="--")
            ax.annotate(f"◀ {t1:.1f}s",
                        xy=(t1, 0.97), xycoords=("data","axes fraction"),
                        fontsize=5, color="darkred", rotation=90,
                        ha="right", va="top")
            i = j
        else:
            i += 1

    ax.set_ylabel(sw_row["row_label"], fontsize=6.5, rotation=0,
                  ha="right", va="center", labelpad=70)
    ax.set_ylim(0, 1)
    ax.set_yticks([])
    ax.grid(axis="x", alpha=0.3)
    # Light background to separate rows visually
    ax.set_facecolor("#FAFAFA")

# ── Alternating row shading ──────────────────────────────────────────────────
for i, ax in enumerate(axes[n_analog:], 0):
    if i % 2 == 1:
        ax.set_facecolor("#F0F0F0")

# ── X axis ───────────────────────────────────────────────────────────────────
axes[-1].set_xlabel("Time (seconds)", fontsize=8)
axes[-1].xaxis.set_minor_locator(matplotlib.ticker.MultipleLocator(0.5))
axes[-1].xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(5))
axes[-1].tick_params(axis="x", labelsize=7)

plt.tight_layout(rect=[0, 0, 1, 0.97])

# Legend
legend_elements = [
    mpatches.Patch(facecolor=STATE_COLORS[1], alpha=0.75, label="state 1 (mid/up)"),
    mpatches.Patch(facecolor=STATE_COLORS[2], alpha=0.75, label="state 2 (high/down)"),
]
fig.legend(handles=legend_elements, loc="lower right", fontsize=7, framealpha=0.8)

if args.output:
    plt.savefig(args.output, dpi=150, bbox_inches="tight")
    print(f"Saved to {args.output}")
else:
    plt.show()