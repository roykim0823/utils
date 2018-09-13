import subprocess

output = subprocess.check_output(
	'echo to stdout; echo to stderr 1>&2; exit 1',
	shell=True,
	stderr=subprocess.STDOUT)
print('Have %d bytes in output' % len(output))
print(output)
