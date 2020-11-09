import os, sys, struct

MIN_PATTERN = 1<<15

def main():
	dirname = sys.argv[1]
	for dirname_, subdirs, fnames in os.walk(dirname):
		for fname in fnames:
			prev = 0
			count = 0
			print(f"Searching {fname} ...")
			with open(os.path.join(dirname_, fname), 'rb') as f:
				while True:
					next_ = f.read(4)
					if len(next_) != 4:
						print(f"Closing {fname}, {f.tell()}")
						f.close()
						break
					next_ = struct.unpack("<L", next_)[0]
					if next_ == prev - 1:
						count += 1
					else:
						count = 0
					prev = next_
					if count > MIN_PATTERN:
						print(f"Found at offset 0x{f.tell()-4*MIN_PATTERN:08x} in {fname}")
						count = 0

if __name__	== "__main__":
	main()