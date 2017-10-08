import matplotlib.pyplot as plt

with open('Tput_Lte0') as f:
	lines = f.read().splitlines()

lines = map(float, lines);

with open('Tput_Wifi0') as f:
	lines2 = f.read().splitlines()

lines2 = map(float, lines2);

plt.plot(lines,'r',label='Lte')
plt.plot(lines2,'g',label='Wi-Fi')
plt.legend(loc=4)
plt.show()

#plt.plot(lines)
#plt.show()
