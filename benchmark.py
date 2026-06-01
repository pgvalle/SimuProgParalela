import subprocess
import statistics
import sys
import os
import glob
import csv

def compile_programs():
    os.makedirs("bin", exist_ok=True)
    print("Compiling seq.c...")
    subprocess.run(["gcc", "-O3", "-D", "SCRIPT",
                    "src/seq.c", "-o", "bin/seq"], check=True)
    print("Compiling omp.c...")
    subprocess.run(["gcc", "-O3", "-fopenmp", "-D", "SCRIPT",
                    "src/omp.c", "-o", "bin/omp"], check=True)
    print("Compiling mpi.c...")
    subprocess.run(["mpicc", "-O3", "-D", "SCRIPT",
                    "src/mpi.c", "-o", "bin/mpi"], check=True)

def run_benchmark(command, input_file, n):
    if isinstance(command, str):
        command = [command]

    times = []
    command_label = " ".join(command)

    for i in range(n):
        with open(input_file, "r") as f:
            result = subprocess.run(command, stdin=f, capture_output=True, text=True)
            if result.returncode == 0:
                try:
                    # The program outputs the time in seconds
                    output = result.stdout.strip()
                    if output:
                        exec_time = float(output)
                        times.append(exec_time)
                except ValueError:
                    pass # Ignore if not a float
            else:
                print(f"Error running {command_label} with {input_file}: {result.stderr}")
        print(i + 1, end=" ", flush=True)
    print()
    return times


def summarize(values):
    if not values:
        return 0.0, 0.0
    return statistics.mean(values), statistics.stdev(values) if len(values) > 1 else 0.0


def calculate_speedups(seq_times, parallel_times):
    return [
        seq_time / parallel_time
        for seq_time, parallel_time in zip(seq_times, parallel_times)
        if parallel_time > 0
    ]

def main():
    if len(sys.argv) < 2:
        print("Usage: python benchmark.py <N_RUNS> [N_PARALLEL=2]")
        sys.exit(1)

    try:
        n_runs = int(sys.argv[1])
        assert(n_runs >= 2)
    except:
        print("N_RUNS must be an integer greater than 1.")
        sys.exit(1)

    try:
        n_parallel = int(sys.argv[2]) if len(sys.argv) >= 3 else 2
        assert(n_parallel >= 1)
    except:
        print("N_PARALLEL must be an integer greater than 0")
        sys.exit(1)

    input_files = sorted(glob.glob("inputs/*.txt"))
    if not input_files:
        print("No input files found in inputs/")
        sys.exit(1)

    # Keep the OpenMP thread count comparable with MPI process count.
    os.environ["OMP_NUM_THREADS"] = str(n_parallel)

    compile_programs()

    print(f"\nStarting benchmark with {n_runs} runs per input...")
    print(f"OpenMP runs will use {n_parallel} thread(s).")
    print(f"MPI runs will use {n_parallel} process(es).")
    
    with open("results.csv", "w") as file:
        writer = csv.writer(file)
        writer.writerow([
            "input",
            "seq_avg", "seq_std",
            "omp_avg", "omp_std", "omp_speedup", "omp_speedup_std",
            "mpi_avg", "mpi_std", "mpi_speedup", "mpi_speedup_std",
        ])

        for input_file in input_files:
            basename = os.path.basename(input_file)
            
            print(f"\nrunning seq.c on {basename}...")
            seq_times = run_benchmark("./bin/seq", input_file, n_runs)
            print(f"running omp.c on {basename}...")
            omp_times = run_benchmark("./bin/omp", input_file, n_runs)
            print(f"running mpi.c on {basename}...")
            mpi_times = run_benchmark([
                "mpirun",
                "--oversubscribe",
                "-np", str(n_parallel),
                "./bin/mpi"
            ], input_file, n_runs)

            if seq_times and omp_times and mpi_times:
                seq_avg, seq_std = summarize(seq_times)
                omp_avg, omp_std = summarize(omp_times)
                mpi_avg, mpi_std = summarize(mpi_times)
                omp_speedups = calculate_speedups(seq_times, omp_times)
                mpi_speedups = calculate_speedups(seq_times, mpi_times)
                omp_speedup, omp_speedup_std = summarize(omp_speedups)
                mpi_speedup, mpi_speedup_std = summarize(mpi_speedups)

                writer.writerow([
                    basename,
                    seq_avg, seq_std,
                    omp_avg, omp_std, omp_speedup, omp_speedup_std,
                    mpi_avg, mpi_std, mpi_speedup, mpi_speedup_std,
                ])
                file.flush()


if __name__ == "__main__":
    main()
