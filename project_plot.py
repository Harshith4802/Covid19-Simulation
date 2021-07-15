#First run the project.c file and run this file for plot

import matplotlib.pyplot as plt

X, Y1, Y2, Y3 = [], [], [], []
for line in open('output.dat', 'r'):
	values = [float(s) for s in line.split()]
	X.append(values[0])
	Y1.append(values[1])
	Y2.append(values[2])
	Y3.append(values[3])
  

plt.plot(X, Y1, label='$Susceptible$')
plt.plot(X, Y2, label='$Infected$')
plt.plot(X, Y3, label='$Recovered$')

plt.legend(loc='best')
plt.grid()
plt.show()
