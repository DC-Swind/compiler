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
main:
move $30, $29
addi $29, $29, -52
li $8, 3
li $9, 4
mul $10, $8, $8
mul $11, $9, $9
add $12, $10, $11
move $4, $12
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
add $13, $8, $12
div $13, $9
mflo $12
move $4, $12
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
add $8, $8, $9
sub $9, $8, $9
sub $8, $8, $9
sub $14, $8, $9
li $15, 2
mul $16, $14, $15
addi $17, $16, -1
div $8, $9
mflo $18
li $19, 3
add $20, $19, $9
mul $21, $15, $20
mul $22, $18, $21
add $9, $17, $22
move $4, $8
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
move $4, $9
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
move $29, $30
li $2, 0
jr $31
