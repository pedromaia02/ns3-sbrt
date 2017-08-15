import matplotlib.pyplot as plt

with open('Tput_Lte_0') as f:
	lines = f.read().splitlines()

lines = map(float, lines);

plt.plot(lines)
plt.show()
