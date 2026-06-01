#!/usr/bin/env python3
import argparse
import csv
import os
import sys


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate benchmark plots with standard-deviation error bars."
    )
    parser.add_argument("csv_file", nargs="?", default="results.csv",
                        help="benchmark CSV file (default: results.csv)")
    parser.add_argument("-o", "--output-dir", default="plots",
                        help="directory for generated plots (default: plots)")
    parser.add_argument("--linear", action="store_true",
                        help="use a linear y-axis for runtime plot instead of log scale")
    parser.add_argument("--format", default="pdf", choices=["png", "pdf", "svg"],
                        help="output image format (default: pdf)")
    return parser.parse_args()


def import_matplotlib():
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
        return plt, np
    except ImportError:
        print("matplotlib is required to generate plots.", file=sys.stderr)
        print("Install it with: python3 -m pip install matplotlib", file=sys.stderr)
        sys.exit(1)


def read_results(path):
    with open(path, newline="") as file:
        rows = list(csv.DictReader(file))

    if not rows:
        print(f"No rows found in {path}", file=sys.stderr)
        sys.exit(1)

    return rows


def to_float(row, key):
    value = row.get(key, "")
    if value == "" or value is None:
        return None
    return float(value)


def available_implementations(rows):
    implementations = []
    for name in ("seq", "omp", "mpi"):
        if f"{name}_avg" in rows[0] and f"{name}_std" in rows[0]:
            implementations.append(name)
    return implementations


def grouped_errorbar_plot(plt, np, labels, series, title, ylabel, output_path, log_scale=False):
    x = np.arange(len(labels))
    width = 0.8 / len(series)

    fig, ax = plt.subplots(figsize=(max(10, len(labels) * 1.2), 6))

    legend_labels = {
        "seq": "Sequencial",
        "omp": "OpenMP",
        "mpi": "MPI",
    }

    for idx, (name, values, errors) in enumerate(series):
        offset = (idx - (len(series) - 1) / 2) * width
        bar_kwargs = {"label": legend_labels.get(name, name.upper())}
        if errors is not None:
            bar_kwargs.update({
                "yerr": errors,
                "capsize": 4,
                "error_kw": {"elinewidth": 1, "capthick": 1},
            })

        ax.bar(x + offset, values, width, **bar_kwargs)

    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=35, ha="right")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend()

    if log_scale:
        ax.set_yscale("log")

    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)



def main():
    args = parse_args()
    plt, np = import_matplotlib()
    rows = read_results(args.csv_file)
    os.makedirs(args.output_dir, exist_ok=True)

    labels = [row["input"] for row in rows]
    implementations = available_implementations(rows)

    runtime_series = []
    for name in implementations:
        runtime_series.append((
            name,
            [to_float(row, f"{name}_avg") for row in rows],
            [to_float(row, f"{name}_std") for row in rows],
        ))

    runtime_path = os.path.join(args.output_dir, f"runtime_errorbars.{args.format}")
    grouped_errorbar_plot(
        plt,
        np,
        labels,
        runtime_series,
        "Tempo médio de execução com barras de erro de desvio padrão",
        "Tempo de execução (s)",
        runtime_path,
        log_scale=not args.linear,
    )

    speedup_series = []
    for name in ("omp", "mpi"):
        if name not in implementations:
            continue

        speedup_key = f"{name}_speedup"
        # Backwards compatibility with the previous benchmark CSV format.
        if name == "omp" and speedup_key not in rows[0] and "speedup" in rows[0]:
            speedup_key = "speedup"
        if speedup_key not in rows[0]:
            continue

        values = [to_float(row, speedup_key) for row in rows]
        speedup_std_key = f"{name}_speedup_std"
        errors = None
        if speedup_std_key in rows[0]:
            errors = [to_float(row, speedup_std_key) for row in rows]

        speedup_series.append((name, values, errors))

    if speedup_series:
        has_speedup_errors = any(errors is not None for _, _, errors in speedup_series)
        speedup_name = "speedup_errorbars" if has_speedup_errors else "speedup"
        speedup_path = os.path.join(args.output_dir, f"{speedup_name}.{args.format}")
        grouped_errorbar_plot(
            plt,
            np,
            labels,
            speedup_series,
            "Speedup em relação ao sequencial" + (" com barras de erro de desvio padrão" if has_speedup_errors else ""),
            "Speedup",
            speedup_path,
        )
        print(f"Wrote {runtime_path} and {speedup_path}")
    else:
        print(f"Wrote {runtime_path}")


if __name__ == "__main__":
    main()
