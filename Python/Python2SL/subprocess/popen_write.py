import subprocess

print '\nwrite:'
proc = subprocess.Popen(['cat', '-'], 
	stdin=subprocess.PIPE)
proc.communicate('\tstdin: to stdin\n')
