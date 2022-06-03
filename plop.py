import xcore

class FMT: pass
FMT.ebabled = True
FMT.ESC="\33" if FMT.ebabled else ""
FMT.BOLD = FMT.ESC + "[1m" if FMT.ebabled else ""
FMT.UNDER = FMT.ESC + "[4m" if FMT.ebabled else ""
FMT.RED = FMT.ESC + "[31m" if FMT.ebabled else ""
FMT.GREEN = FMT.ESC + "[32m" if FMT.ebabled else ""
FMT.YELLOW = FMT.ESC + "[33m" if FMT.ebabled else ""
FMT.BLUE = FMT.ESC + "[34m" if FMT.ebabled else ""
FMT.MAGENTA = FMT.ESC + "[35m" if FMT.ebabled else ""
FMT.CYAN = FMT.ESC + "[36m" if FMT.ebabled else ""
FMT.OFF = FMT.ESC + "[0m" if FMT.ebabled else ""

def indentation(lvl):
    s = ""
    for i in range(lvl): s += "  "
    return s

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
PlopCode.MIN   = PlopCode._BASE_ + 22
PlopCode.MAX   = PlopCode._BASE_ + 23
PlopCode.CALL  = PlopCode._BASE_ + 24

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
		nblk = len(self.blocks)
		bw.writeU32(nblk) # nblk
		self.bodyOffsPos = bw.getPos()
		bw.writeU32(0) # -> body
		self.blkCatTop = bw.getPos()
		for i in range(nblk):
			bw.writeU32(0) # -> blk code
			bw.writeU32(len(self.blocks[i].code))

	def writeData(self, bw, top):
		bw.patchCur(self.bodyOffsPos)
		bw.writeFOURCC("code")
		for i, blk in enumerate(self.blocks):
			bw.align(0x10)
			bw.patchCur(self.blkCatTop + i*8)
			blk.write(bw)

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
				blk = PlopBlock(self)
				blk.compile(parsed)
				blk.disasm()
				self.blocks.append(blk)

class PlopBlock:
	def __init__(self, plop): # PlopExporter as a param 
		self.plop = plop

	def emit_str(self, s):
		sid = self.plop.strLst.add(s)
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
				elif op == "/":
					self.code.append(PlopCode.DIV)
				elif op == "=":
					self.code.append(PlopCode.EQ)
				elif op == "/=":
					self.code.append(PlopCode.NE)
				elif op == "<":
					self.code.append(PlopCode.LT)
				elif op == ">":
					self.code.append(PlopCode.GT)
				elif op == "<=":
					self.code.append(PlopCode.LE)
				elif op == ">=":
					self.code.append(PlopCode.GE)
				elif op == "not":
					self.code.append(PlopCode.NOT)
				elif op == "and":
					self.code.append(PlopCode.AND)
				elif op == "or":
					self.code.append(PlopCode.OR)
				elif op == "xor":
					self.code.append(PlopCode.XOR)
				elif op == "min":
					self.code.append(PlopCode.MIN)
				elif op == "max":
					self.code.append(PlopCode.MAX)
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

	def disasm(self):
		if not self.code: return
		lvl = 0
		sl = self.plop.strLst
		ip = 0
		while ip < len(self.code):
			s = "---"
			ips = FMT.GREEN + "{0:3d}".format(ip) + FMT.OFF
			op = self.code[ip]
			ip += 1
			if op == PlopCode.BEGIN:
				eloc = self.code[ip]
				ip += 1
				s = "BEGIN <" + FMT.GREEN + str(eloc) + FMT.OFF + ">"
				lvl += 1
			elif op == PlopCode.END:
				s = "END"
			elif op == PlopCode.VAR:
				sid = self.code[ip]
				ip += 1
				s = "VAR " + str(sid) + " ; " + FMT.MAGENTA + sl.get(sid) + FMT.OFF
			elif op == PlopCode.SYM:
				sid = self.code[ip]
				ip += 1
				s = "SYM " + str(sid) + " ; " + FMT.BOLD + FMT.MAGENTA + sl.get(sid) + FMT.OFF
			elif op == PlopCode.SET:
				sid = self.code[ip]
				ip += 1
				s = "SET " + str(sid) +" ; " + sl.get(sid)
			elif op == PlopCode.FVAL:
				fbits = self.code[ip]
				ip += 1
				try:
					fstr = str(xcore.setBitsF32(fbits))
				except:
					fstr = "<" + hex(fbits) + ">"
				s = "FVAL " + fstr
			elif op == PlopCode.SVAL:
				sid = self.code[ip]
				ip += 1
				s = "SVAL " + str(sid) + " ; " + FMT.YELLOW + sl.get(sid) + FMT.OFF
			elif op == PlopCode.IF:
				yes = self.code[ip]
				alt = self.code[ip + 1]
				ip += 2
				s = "IF (" + str(yes) + ", " + str(alt) + ")"
			else:
				if op == PlopCode.ADD:
					s = "+"
				elif op == PlopCode.SUB:
					s = "-"
				elif op == PlopCode.MUL:
					s = "*"
				elif op == PlopCode.DIV:
					s = "/"
				elif op == PlopCode.EQ:
					s = "="
				elif op == PlopCode.NE:
					s = "!="
				elif op == PlopCode.LT:
					s = "<"
				elif op == PlopCode.GT:
					s = ">"
				elif op == PlopCode.LE:
					s = "<="
				elif op == PlopCode.GE:
					s = ">="
				elif op == PlopCode.NOT:
					s = "NOT"
				elif op == PlopCode.AND:
					s = "AND"
				elif op == PlopCode.OR:
					s = "OR"
				elif op == PlopCode.XOR:
					s = "XOR"
				elif op == PlopCode.MIN:
					s = "MIN"
				elif op == PlopCode.MAX:
					s = "MAX"
				elif op == PlopCode.CALL:
					s = "CALL"
				narg = self.code[ip]
				ip += 1
				s += " (" + str(narg) + ")"
			xcore.dbgmsg(indentation(lvl) + ips + ": " + s)
			if op == PlopCode.END: lvl -= 1

	def write(self, bw):
		if not self.code: return
		sl = self.plop.strLst
		code = [op for op in self.code]
		for offs in self.strs:
			code[offs] = sl.getWriteId(code[offs])
		for op in code:
			bw.writeU32(op)

if __name__ == '__main__':
	plop = PlopExporter()
	plop.from_file("test.pls")
	plop.save("test.plop")
