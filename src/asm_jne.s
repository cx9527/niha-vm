SETA 0
SETB 1
CMP
JNE win
STRA failed !
SETA 0
GETS
PUTT
EXIT
LABEL win
STRA first test passed !
SETA 0
GETS
PUTT
SETA 42
SETB 42
CMP
JNE failed
STRA you won everything !
SETA 1
GETS
PUTT
EXIT
LABEL failed
STRA second test failed !
SETA 1
GETS
PUTT
EXIT
