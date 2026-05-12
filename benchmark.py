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
    times = []
    for i in range(n):
        with open(input_file, "r") as f:
            result = subprocess.run([f"{command}"], stdin=f, capture_output=True, text=True)
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
                print(f"Error running {command} with {input_file}: {result.stderr}")
        print(i + 1, end=" ", flush=True)
    print()
    return times

def main():
    if len(sys.argv) < 2:
        print("Usage: python benchmark.py <N>")
        sys.exit(1)

    try:
        n = int(sys.argv[1])
        assert(n >= 2)
    except:
        print("N must be an integer greater than 1.")
        sys.exit(1)

    input_files = sorted(glob.glob("inputs/*.txt"))
    if not input_files:
        print("No input files found in inputs/")
        sys.exit(1)

    compile_programs()

    print(f"\nStarting benchmark with {n} runs per input...")
    
    with open("results.csv", "w") as file:
        writer = csv.writer(file)
        writer.writerow(["input", "seq_avg", "seq_std", "omp_avg", "omp_std", "speedup"])

        for input_file in input_files:
            basename = os.path.basename(input_file)
            
            print(f"\nrunning seq.c on {basename}...")
            seq_times = run_benchmark("./bin/seq", input_file, n)
            print(f"running omp.c on {basename}...")
            omp_times = run_benchmark("./bin/omp", input_file, n)

            if seq_times and omp_times:
                seq_avg = statistics.mean(seq_times)
                seq_std = statistics.stdev(seq_times)
                omp_avg = statistics.mean(omp_times)
                omp_std = statistics.stdev(omp_times)
                speedup = seq_avg / omp_avg if omp_avg > 0 else 0

                writer.writerow([basename, seq_avg, seq_std, omp_avg, omp_std, speedup])
                file.flush()


if __name__ == "__main__":
    main()
