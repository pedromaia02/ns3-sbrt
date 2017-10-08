import numpy as np
import matplotlib.pyplot as plt

f=open('sinr.txt',"r")
lines=f.readlines()
flows_A=[]
for x in lines:
    flows_A.append(x.split(' ')[0])
f.close()

sinr = map(float, flows_A);

print min(sinr)
#plt.plot(sinr)
#plt.show()
