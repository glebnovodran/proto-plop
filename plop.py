import xcore

def atom(tok):
	a = None
	try:
		a = float(tok)
	except ValueError:
		a = str(tok)
	return a

def parse_block(toks):
	tok = toks.pop(0)
	if tok == "(":
		lst = []
		while toks[0] != ")":
			lst.append(parse_block(toks))
		toks.pop(0)
		return lst
	else:
		return atom(tok)

class PlopExporter(xcore.BaseExporter):
	def __init__(self):
		xcore.BaseExporter.__init__(self)
		self.sig = "PLOP"

	def writeHead(self, bw, top):
		bw.writeFOURCC("info")

	def writeData(self, bw, top):
		bw.writeFOURCC("code")

	def from_file(self, fname):
		self.toks = None
		f = open(fname)
		if not f: return
		src = f.readlines()
		f.close()
		self.compile(src)

	def compile(self, src):
		self.toks = []
		for line in src:
			icomment = line.find(";")
			if icomment >= 0: line = line[:icomment]
			line = line.replace("\t", " ").replace("(", " ( ").replace(")", " ) ")
			blkToks = line.split()
			#xcore.dbgmsg("<" + str(blkToks) + ">")
			if len(blkToks) > 0: self.toks.append(blkToks)

		self.blocks = None
		if len(self.toks) > 0:
			self.blocks = []
			for i, blockToks in enumerate(self.toks):
				parsed = parse_block(blockToks)
				xcore.dbgmsg(str(i) + ": " + str(parsed))

def main(args):
	return 0

if __name__ == '__main__':
	import sys
	plop = PlopExporter()
	plop.from_file("test.pls")
	plop.save("test.plop")
	sys.exit(main(sys.argv))
