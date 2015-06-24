.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
li $2, 4
la $4, _prompt
syscall
li $2, 5
syscall
jr $31
write:
li $2, 1
syscall
li $2, 4
la $4, _ret
syscall
move $2, $0
jr $31
addfunc:
move $30, $29
addi $29, $29, -12
move $8, $5
move $9, $4
add $10, $8, $9
move $29, $30
move $2, $10
jr $31
subfunc:
move $30, $29
addi $29, $29, -60
addi $24, $30, 0
lw $8, 0($24)
addi $24, $30, 4
lw $9, 0($24)
addi $24, $30, 8
lw $10, 0($24)
addi $24, $30, 12
lw $11, 0($24)
move $12, $7
move $13, $6
move $14, $5
move $15, $4
sub $16, $8, $9
sub $17, $16, $10
sub $18, $17, $11
sub $19, $18, $12
sub $20, $19, $13
sub $21, $20, $14
sub $22, $21, $15
move $29, $30
move $2, $22
jr $31
main:
move $30, $29
addi $29, $29, -28
li $8, 0
li $9, 1
li $10, 0
addi $29, $29, -4
sw $31, 0($29)
jal read
lw $31, 0($29)
addi $29, $29, 4
move $11, $2
move $12, $11
addi $24, $30, -4
sw $8, 0($24)
addi $24, $30, -8
sw $9, 0($24)
addi $24, $30, -12
sw $10, 0($24)
addi $24, $30, -16
sw $11, 0($24)
addi $24, $30, -20
sw $12, 0($24)
label1:
addi $24, $30, -12
lw $8, 0($24)
addi $24, $30, -20
lw $9, 0($24)
addi $24, $30, -12
sw $8, 0($24)
addi $24, $30, -20
sw $9, 0($24)
bge $8, $9, label3
addi $29, $29, -4
sw $31, 0($29)
addi $29, $29, -4
sw $30, 0($29)
addi $24, $30, -8
lw $8, 0($24)
move $4, $8
addi $24, $30, -4
lw $9, 0($24)
move $5, $9
addi $24, $30, -4
sw $9, 0($24)
addi $24, $30, -8
sw $8, 0($24)
jal addfunc
lw $30, 0($29)
addi $29, $29, 4
lw $31, 0($29)
addi $29, $29, 4
move $8, $2
move $9, $8
addi $24, $30, -8
lw $10, 0($24)
move $4, $10
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
move $11, $10
move $10, $9
addi $24, $30, -12
lw $12, 0($24)
addi $12, $12, 1
addi $24, $30, -4
sw $11, 0($24)
addi $24, $30, -8
sw $10, 0($24)
addi $24, $30, -12
sw $12, 0($24)
addi $24, $30, -24
sw $8, 0($24)
addi $24, $30, -28
sw $9, 0($24)
j label1
label3:
addi $24, $30, -20
lw $8, 0($24)
move $4, $8
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
move $29, $30
li $2, 0
jr $31
