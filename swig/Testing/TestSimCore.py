import os, sys

# Try to use SIMDIS_DIR to set up import paths
if 'SIMDIS_DIR' in os.environ:
	# For _module shared object:
	if os.name == "nt":
		sys.path.insert(0, os.environ['SIMDIS_DIR'] + '/lib/amd64-nt/python3.8')
		try:
			# Python 3.8 does not want to respect PATH for loading dependent DLLs.  It introduces
			# a new method to attempt to fix the problem.  Try/except ignores errors in older Python.
			# Without this, the _simCore.pyd needs to go in the same place as simNotify/simCore.
			os.add_dll_directory(os.environ['SIMDIS_DIR'] + '/bin/amd64-nt')
		except:
			pass
		pass
	else:
		sys.path.insert(0, os.environ['SIMDIS_DIR'] + '/lib/amd64-linux/python3.8/lib-dynload')
	# For module wrapper:
	sys.path.insert(0, os.environ['SIMDIS_DIR'] + '/bin/pythonScripts')

import simCore

assert(simCore.Vec3() is not None)
v = simCore.Vec3(1, 2, 3)
assert(v is not None)
assert(v.x() == 1)
assert(v.y() == 2)
assert(v.z() == 3)
v.set(3, 1, 2)
assert(v.x() == 3)
assert(v.y() == 1)
assert(v.z() == 2)
v.scale(2)
assert(v.x() == 6)
assert(v.y() == 2)
assert(v.z() == 4)

# TODO: More testing here

print("Success!");
