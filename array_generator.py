import random
import sys

if len(sys.argv) != 2:
    print("Usage: python3 array_generator -n")
    print("n : number of arr elements")
    exit()
size = int(sys.argv[1])
random_arr = random.sample(range(1, size * 5), size)

with open("input.txt", "w") as f:
    f.write("\n".join(str(item) for item in random_arr))