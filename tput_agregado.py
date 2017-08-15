import matplotlib.pyplot as plt
import operator

f=open('laa_wifi_simple_default_operatorA',"r")
lines=f.readlines()
flows_A=[]
for x in lines:
    flows_A.append(x.split(' ')[7])
f.close()

Tput_A = map(float, flows_A);

f=open('laa_wifi_simple_default_operatorB',"r")
lines=f.readlines()
flows_B=[]
for x in lines:
    flows_B.append(x.split(' ')[7])
f.close()

Tput_B = map(float, flows_B);

Tput_agregado = map(operator.add, Tput_A,Tput_B)

delta_t = map(operator.sub, Tput_A,Tput_B)


############################# PLOTS #####################################

#plt.plot(delta_t,Tput_agregado)
#plt.ylabel('Tput Agregado')
#plt.xlabel('Delta')
#plt.show()

#x = [0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1]
#plt.plot(x,Tput_A,'r',label="LTE")
#plt.plot(x,Tput_B,'g',label="Wi-Fi")
#plt.legend(loc=4)
#plt.show()
