import os
import time

from matplotlib import pyplot as plt

# compile executable "performance_test" using cmake
# run executable "performance_test" with different parameters

if __name__ == '__main__':
    os.system("rm -rf performance_test_build && mkdir performance_test_build")
    os.system("cmake . -B performance_test_build")
    os.system("cmake --build performance_test_build --target performance_test")

    # run executable with different parameters and show graph of time complexity

    results = []
    for i in range(0, 1000):
        curr_time = time.time()
        os.system("./performance_test_build/performance_test insertion " + str(i))
        results.append(time.time() - curr_time)
        print()
        print("Time in seconds: " + str(results[-1]))

    plt.plot(results)
