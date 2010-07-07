#!/usr/bin/env python

import re
import sys
import struct

COMP_SUCCESS    = 0x0
COMP_FAILED     = 0x1
DEBUG           = True

GRAMMAR = {
            "seta":     "^(SETA) ([0-9]+)$",
            "setb":     "^(SETB) ([0-9]+)$",
            "setc":     "^(SETC) ([0-9]+)$",
            "cmp":      "^(CMP)$",
            "jmp":      "^(JMP) ([0-9])+$",
            "gett":     "^(GETT)$",
            "gets":     "^(GETS) ([0-9]+)$",
            "putt":     "^(PUTT)$",
            "unc":      "^(UNC)$",
            "stra":     "^(STRA) ([a-zA-Z0-9 ]+)$"
          }

OPCODES = {
            "seta":     "\xf0",
            "setb":     "\xf1",
            "setc":     "\xf2",
            "cmp":      "\xf3",
            "jmp":     "\xf4",
            "gett":     "\xf5",
            "putt":     "\xf6",
            "stra":     "\xe0",
            "gets":     "\xe1",
            "unc":      "\xe2"
          }

class Instruction:

    def __init__(self):
        self.mnemonic = ""
        self.operands = []
        self.bytes_len = 0

class Parser:

    def __init__(self):
        if DEBUG:
            print "initializing parser"
        self.code = []
        self.bytecode = []
        self.instructions = []
        self.opcodes = {}

    def load_code(self, code_file):
        if DEBUG:
            print "loading code from file %s" % code_file
        try:
            f = open(code_file, "r")
            self.code = f.readlines()
            f.close()
        except:
            return COMP_SUCCESS

        return COMP_FAILED

    def parse_code(self):
        if DEBUG:
            print "parsing code"
        for instr in self.code:
            curr_instr = Instruction()
            if DEBUG:
                print "current instruction: %s" % instr.rstrip("\n")
            for mne in GRAMMAR:
                instr_match = re.match(GRAMMAR[mne],instr)
                if instr_match:
                    if DEBUG:
                        print "found match: %s is of type %s" % (instr.rstrip("\n"), mne)
                    curr_instr.mnemonic = mne
                    if len(instr_match.groups()) == 2:
                        if DEBUG:
                            print "found an operand: %s" % instr_match.group(2)
                        curr_instr.operands.append(instr_match.group(2))
                    self.instructions.append(curr_instr)
        if DEBUG:
            print "parsing results"
            for i in self.instructions:
                print "\tmnemonic: %s\t operand(s): %s" % (i.mnemonic, i.operands)

    def render_seta(self, operand):
        return OPCODES["seta"] + struct.pack("I", int(operand))
    def render_setb(self, operand):
        return OPCODES["setb"] + struct.pack("I", int(operand))
    def render_setc(self, operand):
        return OPCODES["setc"] + struct.pack("I", int(operand))
    def render_cmp(self):
        return OPCODES["cmp"]
    def render_jmp(self, operand):
        return OPCODES["jmp"] + struct.pack("I", int(operand))
    def render_gets(self, operand):
        return OPCODES["gets"] + struct.pack("I", int(operand))
    def render_gett(self):
        return OPCODES["gett"]
    def render_unc(self):
        return OPCODES["unc"]
    def render_putt(self):
        return OPCODES["putt"]
    def render_stra(self,operand):
        print "string: %s" %operand
        return OPCODES["stra"] + struct.pack("I", len(operand)) + operand + "\0"

    def render_bytecode(self):
        if len(self.opcodes) == 0:
            print "opcodes not initialized"
            return COMP_FAILED
        bc = ""
        for i in self.instructions:
            if i.mnemonic == "seta":
                bc = bc + self.render_seta(i.operands[0])
            elif i.mnemonic == "setb":
                bc = bc + self.render_setb(i.operands[0])
            elif i.mnemonic == "setc":
                bc = bc + self.render_setc(i.operands[0])
            elif i.mnemonic == "jmp":
                bc = bc + self.render_jmp(i.operands[0])
            elif i.mnemonic == "cmp":
                bc = bc + self.render_cmp()
            elif i.mnemonic == "gett":
                bc = bc + self.render_gett()
            elif i.mnemonic == "gets":
                bc = bc + self.render_gets(i.operands[0])
            elif i.mnemonic == "putt":
                bc = bc + self.render_putt()
            elif i.mnemonic == "unc":
                bc = bc + self.render_unc()
            elif i.mnemonic == "stra":
                bc = bc + self.render_stra(i.operands[0])
            else:
                print "unknown opcode: %s" % i.mnemonic
                return COMP_FAILED

        return bc

if __name__ == "__main__":
    if len(sys.argv) == 2:
        parser = Parser()
        parser.opcodes = OPCODES
        parser.load_code(sys.argv[1])
        parser.parse_code()
        bc = parser.render_bytecode()
        out = open("bytecode_manu", "w")
        out.write("\x21\x45\x4c\x46")
        out.write(bc)
        out.close()
        for c in bc:
            sys.stdout.write("%02x " % ord(c))
        print
