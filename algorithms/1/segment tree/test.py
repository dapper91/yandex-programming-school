from random import randint
from collections import Counter
import subprocess

start = -1000000000
end = 1000000000
pcount = randint(1, 100000)

start = -1000000000
end = 1000000000
pcount = 100
mcount = int(pcount/3)


def get_sum(ops):
	segments = Counter()

	for op in ops:
		if op[0] == 0:
			segments[(op[1], op[2])] -= 1
		else:
			segments[(op[1], op[2])] += 1
	
	segments = [segment for segment in segments if segments[segment] != 0]
	segments = sorted(segments, key=lambda seg: seg[0])

	i = 0
	while i < len(segments)-1:
		left = segments[i][0]
		right = max(segments[i+1][1], segments[i][1])		
		if segments[i+1][0] <= segments[i][1]:
			segments.pop(i)
			segments.pop(i)	
			segments.insert(i, (left, right))			
		else:
			i += 1

	return sum( [r-l for l,r in segments] )
	
def gen_ops():
	ops = [ [1, min(a,b), max(a,b), 0] for (a,b) in [ (randint(start, end), randint(start, end)) for _ in range(pcount)] ]
	
	for index in [randint(0, len(ops)-2) for _ in range(mcount)]:		
		if ops[index][0] == 1 and ops[index][3] == 0: 
			toind = randint(index + 1, len(ops)-1)
			if (ops[toind][3] == 0):
				ops[index][3] = 1
				ops[toind] = [0] + ops[index][1:3] + [1]	

	return ops



def test():
	ops = gen_ops()	

	right_ans = [get_sum(ops[0:n]) for n in range(1, len(ops)+1)]
	
	args = str(len(ops)) + " " + " ".join([ "%s %s %s" % ("+" if op[0]==1 else "-", op[1], op[2]) for op in ops])
	bytes = args.encode()

	cmd = ['./SegmentTree_v2.1']
	p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
	               			  stderr=subprocess.PIPE,
	               			  stdin=subprocess.PIPE)

	out, err = p.communicate(bytes)

	out = [int(ans) for ans in out.decode().split()]

	for i in range(len(right_ans)):
		if right_ans[i] != out[i]:
			print("wrong answer")
			print("in:  " + args)
			print("right: " + str(right_ans))
			print("out:   " + str(out))
			break

for _ in range(100):
	test()