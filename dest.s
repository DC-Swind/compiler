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
li $8, 0
li $9, 1
li $10, 0
addi $29, $29, -40
addi $24, $30, -16
lw $11, 0($24)
move $11, $29
addi $24, $30, -4
sw $8, 0($24)
move $8, $11
addi $24, $30, -8
sw $9, 0($24)
li $9, 3
addi $24, $30, -12
sw $10, 0($24)
li $10, 4
addi $24, $30, -16
sw $11, 0($24)
mul $11, $9, $10
addi $24, $30, -20
sw $8, 0($24)
add $8, $8, $11
li $9, 1
sw $9, 0($8)
addi $24, $30, -16
lw $10, 0($24)
addi $24, $30, -24
sw $11, 0($24)
move $11, $10
addi $24, $30, -28
sw $8, 0($24)
li $8, 3
li $9, 4
addi $24, $30, -16
sw $10, 0($24)
mul $10, $8, $9
addi $24, $30, -32
sw $11, 0($24)
add $11, $11, $10
lw $11, 0($11)
move $4, $11
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
addi $29, $29, -4
sw $31, 0($29)
jal read
lw $31, 0($29)
addi $29, $29, 4
move $8, $2
move $9, $8
addi $24, $30, -36
sw $10, 0($24)
addi $24, $30, -40
sw $11, 0($24)
addi $24, $30, -44
sw $8, 0($24)
addi $24, $30, -48
sw $9, 0($24)
label1:
addi $24, $30, -12
lw $8, 0($24)
addi $24, $30, -48
lw $9, 0($24)
bge $8, $9, label3
addi $24, $30, -4
lw $10, 0($24)
addi $24, $30, -8
lw $11, 0($24)
addi $24, $30, -12
sw $8, 0($24)
add $8, $10, $11
move $4, $11
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
move $10, $11
move $11, $8
addi $24, $30, -48
sw $9, 0($24)
addi $24, $30, -12
lw $9, 0($24)
addi $9, $9, 1
addi $24, $30, -4
sw $10, 0($24)
addi $24, $30, -8
sw $11, 0($24)
addi $24, $30, -12
sw $9, 0($24)
addi $24, $30, -52
sw $8, 0($24)
j label1
label3:
li $2, 0
jr $31
