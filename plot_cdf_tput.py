import matplotlib.pyplot as plt
import operator
import numpy as np

f=open('laa_wifi_simple_default_operatorA',"r")
lines=f.readlines()
flows_A=[]
for x in lines:
    flows_A.append(x.split(' ')[7])
f.close()

Tput_A = map(float, flows_A);

sorted_Tput_A = np.sort(Tput_A)

cdf_Tput_A = np.arange(len(sorted_Tput_A))/float(len(sorted_Tput_A)-1) 

############################################################################

f=open('laa_wifi_simple_default_operatorB',"r")
lines=f.readlines()
flows_B=[]
for x in lines:
    flows_B.append(x.split(' ')[7])
f.close()

Tput_B = map(float, flows_B);

sorted_Tput_B = np.sort(Tput_B)

cdf_Tput_B = np.arange(len(sorted_Tput_B))/float(len(sorted_Tput_B)-1) 

#############################################################################

plt.plot(sorted_Tput_A,cdf_Tput_A,'r',label='op_A')
plt.plot(sorted_Tput_B,cdf_Tput_B,'g',label='op_B')
plt.legend(loc=4)
plt.show()
