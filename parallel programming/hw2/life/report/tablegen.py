import re


f = open("results")

samples = {}

for string in f:
	sample = re.search("nodes: (\d+), iterations: (\d+), size: (\d+), time: ([\d.]+)", string)
	samples[(int(sample.group(2)),int(sample.group(3)),int(sample.group(1)))] = sample.group(4)



string = ""
for it in range(50, 501, 50):
	for s in range(1024, 8912, 1024):
		for n in [1,2,4,8,16]:
			secs, msecs = samples[(it,s,n)].split(".")
			mins, secs = divmod(int(secs), 60)
			string += "{:01}:{:02}.{}".format(mins, secs, int(msecs[0:2]))
			if n != 16:
				string += "-"
		string += ";"
	string += "\n"


print(string)