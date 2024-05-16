#!/usr/bin/env python3

from dataclasses import dataclass
from typing import TypedDict
import numpy as np
import matplotlib.pyplot as plt
import subprocess
import pathlib
from pathos import multiprocessing
import itertools
import random
import os
import tempfile

@dataclass
class BenchmarkParametrization:
    class TD(TypedDict):
        operator_f: str
        haystack_sz: int
        needle_sz: int

    operator_f: str
    haystack_sz: int
    needle_sz: int

    def to_dict(self) -> "BenchmarkParametrization.TD":
        return {
            "operator_f": self.operator_f,
            "haystack_sz": self.haystack_sz,
            "needle_sz": self.needle_sz,
        }


@dataclass
class BenchmarkResults:
    avg_time_ns: int
    confidence_relative: float

    @classmethod
    def from_csv(
        cls: type["BenchmarkResults"],
        csv_row: str
    ) -> "BenchmarkResults":
        split = csv_row.split(",")
        return BenchmarkResults(int(split[1]), float(split[3]))

    def to_dict(self) -> dict[str, float]:
        return {
            "avg_time_ns": float(self.avg_time_ns),
            "confidence_relative": float(self.confidence_relative)
        }


def execute_benchmark(
    parametrization: BenchmarkParametrization
) -> BenchmarkResults:
    def _compile_benchmark(bin_name: str):
        subprocess.run([
            "gcc",
            "-o", bin_name,
            "src/strstrbench.c",
            "-Ithird-party/ubench/include",
            "-lm",
            "-Ofast",
            "-UNDEBUG",
            "-flto",
            "-fomit-frame-pointer",
            "-march=native",
            f"-DBENCH_NAME={parametrization.operator_f}",
            f"-DBENCH_HAY_SZ={parametrization.haystack_sz}",
            f"-DBENCH_NEEDLE_SZ={parametrization.needle_sz}",
            "-Istringzilla",
            "-Ibuild/_deps/stringzilla-src/include"
        ], check=True, shell=False, text=True)

    bin_name = os.path.join(tempfile.mkdtemp(), 'something')

    _compile_benchmark(bin_name)

    results_name = os.path.join(tempfile.mkdtemp(), 'something')

    subprocess.run([
        bin_name,
        "--confidence=5",
        f"--output={results_name}",
    ], check=True, shell=False, text=True)

    with pathlib.Path(results_name).open("r", encoding="utf-8") as f:
        results = BenchmarkResults.from_csv(f.readlines()[-1])

    pathlib.Path(results_name).unlink(missing_ok=False)
    pathlib.Path(bin_name).unlink(missing_ok=False)

    return results


def main():
    haysizes = np.linspace(0, 128 * 1024, 10)
    needlesizes = np.linspace(1, 12, 6)
    domain = list(itertools.product(haysizes, needlesizes))

    def _do_sample(function: str):
        with multiprocessing.Pool(8) as threadpool:
            results = threadpool.map(
                lambda cartesian_pair: (
                    cartesian_pair[0],
                    cartesian_pair[1],
                    (res := execute_benchmark(
                        BenchmarkParametrization(
                            function,
                            cartesian_pair[0],
                            cartesian_pair[1],
                        )
                    )).avg_time_ns,
                    res.confidence_relative
                ),
                domain
            )

        xs, ys, zs, _ = zip(*results)

        return (xs, ys, zs)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection="3d")
    ax.scatter(*_do_sample("memmem"), label="memmem")
    ax.scatter(*_do_sample("sz_find"), label="sz_find")
    #ax.scatter(*_do_sample("BM"), label="BM")
    ax.set_xlabel("Haystack Size")
    ax.set_ylabel("Needle Size")
    ax.set_zlabel("Time processing (ns)")
    ax.legend()
    fig.tight_layout()
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
