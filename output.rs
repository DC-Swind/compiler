FUNCTION FunDec :
READ t1
v1 := t1
t2 := v1
t3 := #0
IF t2 > t3 GOTO label1
GOTO label2
LABEL label1 :
GOTO label3
LABEL label2 :
t4 := v1
t5 := #0
IF t4 < t5 GOTO label4
GOTO label5
LABEL label4 :
GOTO label6
LABEL label5 :
LABEL label6 :
LABEL label3 :
t6 := #0
RETURN t6
---------------------------------------
