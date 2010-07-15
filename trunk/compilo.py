#!/usr/bin/env python

"""
compiler for niha-vm virtual machine

s1gma
"""

import re
import sys
import struct
import os
import random

COMP_SUCCESS    = 0x0
COMP_FAILED     = 0x1
DEBUG           = True
MAGIC           = "\x21\x45\x4c\x46"
OVERWRITE       = True

MNEMONICS = (
                "exit",
                "seta",
                "setb",
                "setc",
                "cmp",
                "jmp",
                "gett",
                "putt",
                "jne",
                "stra",
                "gets",
                "unc",
                "sto"
            )

GRAMMAR = {
            "seta":     "^(SETA) ([0-9]+)$",
            "setb":     "^(SETB) ([0-9]+)$",
            "setc":     "^(SETC) ([0-9]+)$",
            "cmp":      "^(CMP)$",
            "jmp":      "^(JMP) ([a-zA-Z0-9_]+)+$",
            "jne":      "^(JNE) ([a-zA-Z0-9_]+)+$",
            "gett":     "^(GETT)$",
            "gets":     "^(GETS)$",
            "putt":     "^(PUTT)$",
            "unc":      "^(UNC)$",
            "ciph":     "^(CIPH) ([\S ]+)$",
            "stra":     "^(STRA) ([\S ]+)$",
            "sto":      "^(STO)$",
            "print":    "^(PRINT) ([\S ]+)",
            "label":    "^(LABEL) ([a-zA-Z0-9_]+)$",
            "exit":     "^(EXIT)$"
          }

OPCODES = {
            "seta":     "\xf0",
            "setb":     "\xf1",
            "setc":     "\xf2",
            "cmp":      "\xf3",
            "jmp":      "\xf4",
            "gett":     "\xf5",
            "putt":     "\xf6",
            "jne":      "\xf7",
            "stra":     "\xe0",
            "gets":     "\xe1",
            "unc":      "\xe2",
            "sto":      "\xe3",
            "exit":     "\xff"
          }

class Instruction:

    def __init__(self):
        self.mnemonic = ""
        self.operands = []
        self.byte_len = 0

class Parser:

    def __init__(self):
        self.code = []
        self.bytecode = ""
        self.instructions = []
        self.opcodes = {}
        self.last_str_idx = 0
        self.pc = 0
        self.labels = {}
        self.jmps = {}

    def gen_opcodes(self):
        """
        generate random opcodes
        fills self.opcodes and writes to opcodes.h
        """
        dev_rand = open("/dev/urandom", "r")
        rand_seed = dev_rand.read(42)
        if DEBUG:
            # pure sex !!!
            print "random seed: %s" % (" ".join(map(str,[hex(ord(c)) for c in rand_seed])))
        random.seed(rand_seed)
        dev_rand.close()
        f = open("opcodes.h", "w")
        for mne in MNEMONICS:
            self.opcodes[mne] = chr(random.randint(0x00, 0xff))
            f.write("#define OP_%s\t\t0x%02x\n" % (mne,ord(self.opcodes[mne])))
        f.close()
        if DEBUG:
            f = open("opcodes.h", "r")
            print "generated opcodes"
            for line in f:
                sys.stdout.write("\t" + line)
            f.close()

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
                print "\tcurrent instruction: '%s'" % instr.rstrip("\n")
            for mne in GRAMMAR:
                instr_match = re.match(GRAMMAR[mne],instr)
                if instr_match:
                    instr_is_valid = True
                    if DEBUG:
                        print "\tfound match: '%s' is of type %s" % (instr.rstrip("\n"), mne)
                    curr_instr.mnemonic = mne
                    if len(instr_match.groups()) == 2:
                        if DEBUG:
                            print "\tfound an operand: %s" % instr_match.group(2)
                        curr_instr.operands.append(instr_match.group(2))
                    self.instructions.append(curr_instr)
                    break
            if not instr_is_valid:
                print "\tinvalid instruction %s" % instr.rstrip("\n")
                return COMP_FAILED

        if DEBUG:
            print "parsing results"
            for i in self.instructions:
                print "\tmnemonic: %s\t operand(s): %s" % (i.mnemonic, i.operands)
        return COMP_SUCCESS

    def resolve_jmps(self):
        """
        resolve jumps (they were left blank by the render_bytecode() funtion)
        """
        if DEBUG:
            print "jumps resolution"
        buff = []
        if DEBUG:
            print "\tjumps to resolve: %s" % self.jmps
            print "\tlabels found: %s" % self.labels
        for jmp in self.jmps:
            if jmp in self.labels:
                addr    = self.jmps[jmp]
                offset  = int(self.labels[jmp]) - int(self.jmps[jmp])
                if DEBUG:
                    print "\tresolved one jump (relative offset: %d)" % (offset)

                ################################
                # TODO: THIS CODE GIVES CANCER #
                ################################
                #                              #
                # ---------------------------- #
                #       JMP OFFSET      LABEL  #
                # ---------------------------- #
                #           ^     ^            #
                #           |     |            #
                #        addr+1   |            #
                #               addr+5         #
                #                              #
                ################################

                if offset > 0:
                    buff[0:addr+1] = self.bytecode[0:addr+1]
                    buff[addr+1:addr+5] = struct.pack("i", offset-1)
                    buff[addr+5:] = self.bytecode[addr+5:]
                else:
                    buff[0:addr+1] = self.bytecode[0:addr+1]
                    buff[addr+1:addr+5] = struct.pack("i", offset-1)
                    buff[addr+5:] = self.bytecode[addr+5:]
                    pass
                self.bytecode = "".join(buff)
            else:
                return COMP_FAILED
        return COMP_SUCCESS

#################################
#
# RENDERING FUNCTIONS
#
#################################
    def render_seta(self, operand):
        return self.opcodes["seta"] + struct.pack("I", int(operand))
    def render_setb(self, operand):
        return self.opcodes["setb"] + struct.pack("I", int(operand))
    def render_setc(self, operand):
        return self.opcodes["setc"] + struct.pack("I", int(operand))
    def render_cmp(self):
        return self.opcodes["cmp"]
    def render_jmp(self, operand):
        # we will resolve jmp adresse later, for now, offset == 0x0
        return self.opcodes["jmp"] + struct.pack("I", 0)
    def render_jne(self, operand):
        # we will resolve jne adresse later, for now, offset == 0x0
        return self.opcodes["jne"] + struct.pack("I", 0)
    def render_gets(self):
        return self.opcodes["gets"]
    def render_gett(self):
        return self.opcodes["gett"]
    def render_sto(self):
        self.last_str_idx = self.last_str_idx+1
        return self.opcodes["sto"]
    def render_unc(self):
        return self.opcodes["unc"]
    def render_exit(self):
        return self.opcodes["exit"]
    def render_putt(self):
        return self.opcodes["putt"]
    def render_stra(self,operand):
        self.last_str_idx = self.last_str_idx+1
        return self.opcodes["stra"] + \
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
        fills the bytecode of the parser, and resove jumps
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
                self.jmps[i.operands[0]] = len(bc)
                bc = bc + self.render_jmp(i.operands[0])
            elif i.mnemonic == "jne":
                self.jmps[i.operands[0]] = len(bc)
                bc = bc + self.render_jne(i.operands[0])
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
            elif i.mnemonic == "exit":
                bc = bc + self.render_exit()
            elif i.mnemonic == "stra":
                bc = bc + self.render_stra(i.operands[0])
            elif i.mnemonic == "ciph":
                bc = bc + self.render_ciph(i.operands[0])
            elif i.mnemonic == "print":
                bc = bc + self.render_print(i.operands[0])
            elif i.mnemonic == "sto":
                bc = bc + self.render_sto()
            elif i.mnemonic == "label":
                self.labels[i.operands[0]] = len(bc)
            else:
                print "unknown opcode: %s" % i.mnemonic
                return COMP_FAILED

            i.byte_len = len(bc) - prev_bc_len
            prev_bc_len = len(bc)
            if DEBUG:
                print "(size:%d):\t%s %s" % (i.byte_len, i.mnemonic, i.operands)

        self.bytecode = bc
        self.resolve_jmps()
        if DEBUG:
            colls = 20
            count = 0
            print "final bytecode"
            sys.stdout.write(" ")
            for c in parser.bytecode:
                sys.stdout.write("%02x " % ord(c))
                count += 1
                if count % colls == 0:
                    sys.stdout.write("\n")
                if count % 4 == 0:
                    sys.stdout.write(" ")
            print
        return COMP_SUCCESS

class Packager:
    def __init__(self, bf=""):
        self.bin_file = bf

    def wbc(self, bytecode):
        """
        write bytecode into self.bin_file
        """
        if len(self.bin_file) == 0:
            print "compiler not initialized"
            return COMP_FAILED
        if os.path.exists(self.bin_file): 
            if not OVERWRITE:
                print "%s already exists"
                return COMP_FAILED

        #############################
        #                           #
        # MAGIC | bytecode | "\xff" #
        #                           #
        #############################
        if DEBUG:
            print "packaging bytecode to %s" % self.bin_file
        f = open(self.bin_file, "w")
        f.write(MAGIC)
        f.write(bytecode)
        f.write("\xff")
        f.close()
        return COMP_SUCCESS

if __name__ == "__main__":
    if len(sys.argv) == 2:
        parser = Parser()
        pack = Packager("%s.bc" % sys.argv[1])
        #parser.opcodes = OPCODES
        parser.gen_opcodes()
        if parser.load_code(sys.argv[1]) == COMP_SUCCESS and \
           parser.parse_code() == COMP_SUCCESS and \
           parser.render_bytecode() == COMP_SUCCESS and \
           pack.wbc(parser.bytecode) == COMP_SUCCESS:

            print "compilation succeeded"
            
        else:
            print "compilation failed"
