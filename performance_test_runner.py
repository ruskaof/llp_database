import os
import time

from matplotlib import pyplot as plt

# insertions, updates, selects
test_type = "selects"

if __name__ == '__main__':
    os.system("rm -rf performance_test_build && mkdir performance_test_build")
    os.system("cmake . -B performance_test_build")
    os.system("cmake --build performance_test_build --target performance_test")

    results = []
    for i in range(0, 200):
        curr_time = time.time()
        os.system(f'./performance_test_build/performance_test {test_type} ' + str(i))
        results.append(time.time() - curr_time)
        print()
        print("Time in seconds: " + str(results[-1]))

    plt.plot(results)
    plt.ylabel('Time in seconds')
    plt.xlabel('Number of elements')
    plt.show()
