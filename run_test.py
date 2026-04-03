import os

if os.path.exists("res") and os.path.isfile("res"):
	os.remove("res")

_TEST_DIR_ = "tests/"
files = os.listdir(_TEST_DIR_)

files.sort()

for file in files:
	file = os.path.join(_TEST_DIR_, file);	
	print(f"running {file}")
	os.system(f"./a.out {file} >> res")
