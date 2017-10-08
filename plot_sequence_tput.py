import matplotlib.pyplot as plt


###### OPERATOR A ######

f=open('laa_wifi_simple_default_operatorA',"r")
lines=f.readlines()
Tput_A=[]
for x in lines:
    Tput_A.append(x.split(' ')[7])
f.close()

Tput_A = map(float, Tput_A);


###### OPERATOR B ######

f=open('laa_wifi_simple_default_operatorB',"r")
lines=f.readlines()
Tput_B=[]
for x in lines:
    Tput_B.append(x.split(' ')[7])
f.close()

Tput_B = map(float, Tput_B);

dc = [0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0]

plt.plot(dc,Tput_A, label="Tput_A")
plt.plot(dc,Tput_B, label="Tput_B")
plt.legend(loc=5)

plt.show()
