import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(0.0, 2.0 * np.pi, 200)
y = np.sin(x)

plt.plot(x, y)
plt.xlabel("x")
plt.ylabel("sin(x)")
plt.title("Numerical Methods in Physics - Plot Test")
plt.grid(True)

plt.savefig("plot_test.pdf")
plt.show()
