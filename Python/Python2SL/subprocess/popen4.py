import subprocess

print '\npopen4:'
proc = subprocess.Popen('cat -; echo "to stderr" 1>&2',
	shell=True,
	stdin=subprocess.PIPE,
	stdout=subprocess.PIPE,
	stderr=subprocess.STDOUT)
stdout_value, stderr_value = proc.communicate('through stdin to stdout')
print '\tcombined output:', repr(stdout_value)
print '\tstderr         :', repr(stderr_value)
