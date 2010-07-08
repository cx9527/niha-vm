#!/usr/bin/env python

"""
compiler for niha-vm virtual machine

s1gma
"""

import re
import sys
import struct

COMP_SUCCESS    = 0x0
COMP_FAILED     = 0x1
DEBUG           = True


# TODO
# JUMP

GRAMMAR = {
            "seta":     "^(SETA) ([0-9]+)$",
            "setb":     "^(SETB) ([0-9]+)$",
            "setc":     "^(SETC) ([0-9]+)$",
            "cmp":      "^(CMP)$",
            "jmp":      "^(JMP) ([0-9])+$",
            "gett":     "^(GETT)$",
            "gets":     "^(GETS)$",
            "putt":     "^(PUTT)$",
            "unc":      "^(UNC)$",
            "ciph":     "^(CIPH) ([\S ]+)$",
            "stra":     "^(STRA) ([\S ]+)$",
            "sto":      "^(STO)$",
            "print":    "^(PRINT) ([\S ]+)",
            "label":    "^([a-zA-Z0-9_]+):$"
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
            "unc":      "\xe2",
            "sto":      "\xe3"
          }

class Instruction:

    def __init__(self):
        self.mnemonic = ""
        self.operands = []
        self.byte_len = 0

class Parser:

    def __init__(self):
        if DEBUG:
            print "initializing parser"
        self.code = []
        self.bytecode = ""
        self.instructions = []
        self.opcodes = {}
        self.last_str_idx = 0
        self.pc = 0

    def load_code(self, code_file):
        """
        load source file in the parser
        """

        if DEBUG:
            print "loading code from file %s" % code_file
        try:
            f = open(code_file, "r")
            self.code = f.readlines()
            f.close()
        except:
            return COMP_FAILED

        return COMP_SUCCESS

    def parse_code(self):
        """
        parse source code, and fills the instructions of the parser
        """
        if DEBUG:
            print "parsing code"
        for instr in self.code:
            curr_instr = Instruction()
            instr_is_valid = False
            if DEBUG:
                print "current instruction: '%s'" % instr.rstrip("\n")
            for mne in GRAMMAR:
                instr_match = re.match(GRAMMAR[mne],instr)
                if instr_match:
                    instr_is_valid = True
                    if DEBUG:
                        print "found match: '%s' is of type %s" % (instr.rstrip("\n"), mne)
                    curr_instr.mnemonic = mne
                    if len(instr_match.groups()) == 2:
                        if DEBUG:
                            print "found an operand: %s" % instr_match.group(2)
                        curr_instr.operands.append(instr_match.group(2))
                    self.instructions.append(curr_instr)
                    break
            if not instr_is_valid:
                print "invalid instruction %s" % instr.rstrip("\n")
                return COMP_FAILED

        if DEBUG:
            print "parsing results"
            for i in self.instructions:
                print "\tmnemonic: %s\t operand(s): %s" % (i.mnemonic, i.operands)
        return COMP_SUCCESS

#################################
#
# RENDERING FUNCTIONS
#
#################################
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
    def render_gets(self):
        return OPCODES["gets"]
    def render_gett(self):
        return OPCODES["gett"]
    def render_sto(self):
        self.last_str_idx = self.last_str_idx+1
        return OPCODES["sto"]
    def render_unc(self):
        return OPCODES["unc"]
    def render_putt(self):
        return OPCODES["putt"]
    def render_stra(self,operand):
        self.last_str_idx = self.last_str_idx+1
        return OPCODES["stra"] + \
               struct.pack("I", len(operand)+1) + \
               operand + \
               "\x00"
    def render_print(self,operand):
        return self.render_stra(operand) + \
               self.render_seta(self.last_str_idx-1) + \
               self.render_gets() + \
               self.render_putt()
    def render_ciph(self,operand):
        ciphered_operand = ""
        for c in operand:
            ciphered_operand = ciphered_operand + chr((ord(c) - 42) % 255)
        return self.render_stra(ciphered_operand)

    def render_bytecode(self):
        """
        translate the parser instructions into bytecode
        """

        if len(self.opcodes) == 0:
            print "opcodes not initialized"
            return COMP_FAILED
        bc = ""
        prev_bc_len = 0
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
                bc = bc + self.render_gets()
            elif i.mnemonic == "putt":
                bc = bc + self.render_putt()
            elif i.mnemonic == "unc":
                bc = bc + self.render_unc()
            elif i.mnemonic == "stra":
                bc = bc + self.render_stra(i.operands[0])
            elif i.mnemonic == "ciph":
                bc = bc + self.render_ciph(i.operands[0])
            elif i.mnemonic == "print":
                bc = bc + self.render_print(i.operands[0])
            elif i.mnemonic == "sto":
                bc = bc + self.render_sto()
            elif i.mnemonic == "label":
                pass #TODO
            else:
                print "unknown opcode: %s" % i.mnemonic
                return COMP_FAILED

            i.byte_len = len(bc) - prev_bc_len
            prev_bc_len = len(bc)
            if DEBUG:
                print "(size:%d):\t%s %s" % (i.byte_len, i.mnemonic, i.operands)

        self.bytecode = bc + "\xff"
        return COMP_SUCCESS

if __name__ == "__main__":
    if len(sys.argv) == 2:
        parser = Parser()
        parser.opcodes = OPCODES
        if parser.load_code(sys.argv[1]) == COMP_SUCCESS and \
           parser.parse_code() == COMP_SUCCESS and \
           parser.render_bytecode() == COMP_SUCCESS:
            out = open("%s.bc" % sys.argv[1], "w")
            out.write("\x21\x45\x4c\x46")
            out.write(parser.bytecode)
            out.close()
            print "wrote bytecode to %s.bc:" % sys.argv[1]
            for c in parser.bytecode:
                sys.stdout.write("%02x " % ord(c))
            print
        else:
            print "compilation failed"
