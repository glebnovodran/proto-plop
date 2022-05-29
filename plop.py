import xcore

class PlopCode: pass
PlopCode._BASE_ = 100
PlopCode.BEGIN = PlopCode._BASE_ + 0
PlopCode.END   = PlopCode._BASE_ + 1
PlopCode.VAR   = PlopCode._BASE_ + 2
PlopCode.SYM   = PlopCode._BASE_ + 3
PlopCode.SET   = PlopCode._BASE_ + 4
PlopCode.FVAL  = PlopCode._BASE_ + 5
PlopCode.SVAL  = PlopCode._BASE_ + 6
PlopCode.IF    = PlopCode._BASE_ + 7
PlopCode.ADD   = PlopCode._BASE_ + 8
PlopCode.SUB   = PlopCode._BASE_ + 9
PlopCode.MUL   = PlopCode._BASE_ + 10
PlopCode.DIV   = PlopCode._BASE_ + 11
PlopCode.EQ    = PlopCode._BASE_ + 12
PlopCode.NE    = PlopCode._BASE_ + 13
PlopCode.LT    = PlopCode._BASE_ + 14
PlopCode.GT    = PlopCode._BASE_ + 15
PlopCode.LE    = PlopCode._BASE_ + 16
PlopCode.GE    = PlopCode._BASE_ + 17
PlopCode.NOT   = PlopCode._BASE_ + 18
PlopCode.AND   = PlopCode._BASE_ + 19
PlopCode.OR    = PlopCode._BASE_ + 20
PlopCode.XOR   = PlopCode._BASE_ + 21
PlopCode.CALL  = PlopCode._BASE_ + 22

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

class PlopBlock:
	def __init__(self, plop): # PlopExporter as a param 
		self.plop = plop

	def emit_str(self, s):
		sid = plop.strLst.add(s)
		self.strs.append(len(self.code))
		self.code.append(sid)

	def compile(self, code):
		self.code = []
		self.strs = []
		self.stk = []
		self.compile_sub(code)

	def compile_sub(self, code):
		if isinstance(code, list):
			self.code.append(PlopCode.BEGIN)
			self.stk.append(len(self.code))
			self.code.append(0)
			op = code[0]
			#patchPos = len(self.code)
			if op == "defvar":
				self.code.append(PlopCode.VAR)
				self.emit_str(code[1]) # name
				self.compile_sub(code[2])
			elif op == "set":
				self.code.append(PlopCode.SET)
				self.emit_str(code[1])
				self.compile_sub(code[2])
			elif op == "if":
				self.code.append(PlopCode.IF)
				patchPos = len(self.code)
				self.code.append(0)
				self.code.append(0)
				# cond
				self.compile_sub(code[1])
				# yes
				self.code[patchPos] = len(self.code)
				self.compile_sub(code[2])
				# alt
				self.code[patchPos + 1] = len(self.code)
				self.compile_sub(code[3])
			else:
				# arithmetic, logic or call
				callFlg = False
				if op == "+":
					self.code.append(PlopCode.ADD)
				elif op == "-":
					self.code.append(PlopCode.SUB)
				elif op=="*":
					self.code.append(PlopCode.MUL)
				elif op=="/":
					self.code.append(PlopCode.DIV)
				elif op=="=":
					self.code.append(PlopCode.EQ)
				elif op=="/=":
					self.code.append(PlopCode.NE)
				elif op=="<":
					self.code.append(PlopCode.LT)
				elif op==">":
					self.code.append(PlopCode.GT)
				elif op=="<=":
					self.code.append(PlopCode.LE)
				elif op==">=":
					self.code.append(PlopCode.GE)
				elif op=="not":
					self.code.append(PlopCode.NOT)
				elif op=="and":
					self.code.append(PlopCode.AND)
				elif op=="or":
					self.code.append(PlopCode.OR)
				elif op=="xor":
					self.code.append(PlopCode.XOR)
				else:
					self.code.append(PlopCode.CALL)
					callFlg = True
				self.code.append(len(code) - 1)
				if callFlg: self.compile_sub(op)
				for exp in code[1:]:
					self.compile_sub(exp)

			patchPos = self.stk.pop()
			self.code[patchPos] = len(self.code)
			self.code.append(PlopCode.END)
		elif isinstance(code, str):
			if code[0] == '"':
				self.code.append(PlopCode.SVAL)
				self.emit_str(code[1:-1])
			else:
				self.code.append(PlopCode.SYM)
				self.emit_str(code)
		else:
			self.code.append(PlopCode.FVAL)
			self.code.append(xcore.getBitsF32(code))

def main(args):
	return 0

if __name__ == '__main__':
	import sys
	plop = PlopExporter()
	plop.from_file("test.pls")
	plop.save("test.plop")
	sys.exit(main(sys.argv))
