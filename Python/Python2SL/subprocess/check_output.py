import subprocess

output = subprocess.check_output(['ls', '-l'])
print('Have %d bytes in output' % len(output))
print(output)
