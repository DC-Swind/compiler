FUNCTION fact :
PARAM v4
t1 := v4
t2 := #1
IF t1 == t2 GOTO label1
GOTO label2
LABEL label1 :
t3 := v4
RETURN t3
GOTO label3
LABEL label2 :
t6 := v4
t9 := v4
t10 := #1
t8 := t9 - t10 :
t7 := CALL fact
t5 := t6 * t7 :
RETURN t4
LABEL label3 :
FUNCTION main :
DEC v8 320
t11 := v8
t12 := #3
t12 := t12 * #4 :
t11 := t11 + t12 :
t13 := #2
t13 := t13 * #32 :
t11 := t11 + t13 :
t14 := #1
*t11 := t14
READ t15
v5 := t15
t16 := v5
t17 := #1
IF t16 > t17 GOTO label4
GOTO label5
LABEL label4 :
t19 := v5
t18 := CALL fact
v6 := t18
GOTO label6
LABEL label5 :
t20 := #1
v6 := t20
LABEL label6 :
t21 := v6
WRITE t21
t22 := #0
RETURN t22
