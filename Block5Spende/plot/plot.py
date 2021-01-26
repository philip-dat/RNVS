import numpy as np
import matplotlib.pyplot as plt


data = np.genfromtxt("data.csv", delimiter=";", name=["dns","n","root_disp","avg_disp","delay","offset"])


plt.figure()
plt.plot(data['n'], data['root_disp'])
plt.savefig("root_disp.pdf")

plt.figure();
plt.plot(data['n'], data['delay'])
plt.savefig("delay.pdf")

plt.figure();
plt.plot(data['n'], data['offset'])
plt.savefig("offset.pdf")
