import xcore
import plop

from xml.dom.minidom import parse, parseString
from array import array

try: xrange
except: xrange = range

def get_text(parent, nodename):
	txt = ""
	xmlnodes = parent.getElementsByTagName(nodename)
	if (len(xmlnodes)) :
		textnode = xmlnodes[0].firstChild
		if (textnode.nodeType == textnode.TEXT_NODE):
			txt = textnode.nodeValue
	return txt

class Node:
	def __init__(self, strLst, plopLst):
		self.before = -1
		self.after = -1
		self.plsay = -1
		self.say = -1

		self.strLst = strLst
		self.plopLst = plopLst

	def from_xml(self, xmlnode):
		txt = get_text(xmlnode, "before")
		if txt != "" :
			self.before = len(self.plopLst)
			self.plopLst.append(txt)

		txt = get_text(xmlnode, "after")
		if txt != "" :
			self.after = len(self.plopLst)
			self.plopLst.append(txt)

		txt = get_text(xmlnode, "plsay")
		self.plsay = self.strLst.add(txt) if txt != "" else -1

		txt = get_text(xmlnode, "say")
		self.say = self.strLst.add(txt) if txt != "" else -1


	def write(self, bw, top):
		bw.writeI32(self.strLst.getWriteId(self.plsay))
		bw.writeI32(self.strLst.getWriteId(self.say))
		bw.writeI32(self.before)
		bw.writeI32(self.after)

class DramaExporter(xcore.BaseExporter):
	def __init__(self):
		xcore.BaseExporter.__init__(self)
		self.sig = "DRAC"
		self.xml = ""
		self.nodeLst = []
		self.plopLst = []
		self.plopIdOffs = 0

	def from_file(self, fname):
		self.toks = None
		f = open(fname)
		if not f: return
		src = f.readlines()
		f.close()
		self.prepare_xml(src)

	def prepare_xml(self, src):
		xml = []
		xml.append("<drama>\n")
		for line in src:
			line = line.replace('\t',' ').lstrip()
			line = line[line.startswith(";$") and 2:]
			line = line.replace('\t',' ').lstrip()
			if line: xml.append(line)
		xml.append("</drama>\n")
		for line in xml:
			xcore.dbgmsg(line)
		prepared = ''.join(xml);
		tmpfile = open("prepared.xml", "w")
		tmpfile.writelines(prepared)
		tmpfile.close()
		xmldom = parseString(prepared)
		self.process_xml(xmldom)

	def process_xml(self, xmldom):
		xmlnodes = xmldom.getElementsByTagName("node")
		xcore.dbgmsg("Found %d node(s)" % len(xmlnodes))
		for xmlnode in xmlnodes:
			node = Node(self.strLst, self.plopLst)
			node.from_xml(xmlnode)
			self.nodeLst.append(node)

	def writeHead(self, bw, top):
		bw.writeFOURCC("head")
		nnodes = len(self.nodeLst)
		bw.writeU32(nnodes)
		nplops = len(self.plopLst)
		bw.writeU32(nplops)
		self.bodyOffsPos = bw.getPos()
		bw.writeU32(0) # -> body
		self.plopCatPos = bw.getPos()
		for plstr in self.plopLst:
			bw.writeU32(0) # --> plop prog

	def writeData(self, bw, top):
		bw.align(0x10)
		bw.patch(self.bodyOffsPos, bw.getPos() - top)
		bw.writeFOURCC("body")
		for node in self.nodeLst:
			node.write(bw, top)

		nplop = len(self.plopLst)
		for i, val in enumerate(self.plopLst):
			plopExp = plop.PlopExporter()
			plopExp.from_str(val)

			bw.align(0x10)
			pos = self.plopCatPos + i * 4
			bw.patch(pos, bw.getPos() - top)
			plopExp.write(bw)

if __name__ == '__main__':
	drama = DramaExporter()
	drama.from_file("starboard.drama")
	drama.save("starboard.drac")