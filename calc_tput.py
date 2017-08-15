import numpy as np

f=open('laa_wifi_simple_default_operatorA',"r")
lines=f.readlines()
flows_A=[]
for x in lines:
    flows_A.append(x.split(' ')[7])
f.close()

Tput_A = map(float, flows_A);
print ("Avr_Tput A: {0}".format(np.mean(Tput_A)))

f=open('laa_wifi_simple_default_operatorB',"r")
lines=f.readlines()
flows_B=[]
for x in lines:
    flows_B.append(x.split(' ')[7])
f.close()

Tput_B = map(float, flows_B);
print ("Avr_Tput B: {0}".format(np.mean(Tput_B)))

Total_aggr_Tput = np.mean(Tput_B) + np.mean(Tput_A)

print ("Total Avr_Tput A+B: {0}".format(Total_aggr_Tput))
