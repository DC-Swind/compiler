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
li $-1, 0
li $-1, 1
li $-1, 0
addi $29, $29, -4
sw $31, 0($29)
jal read
lw $31, 0($29)
addi $29, $29, 4
move $-1, $-1
label1:
add $-1, $-1, $-1
move $4, $-1
addi $29, $29, -4
sw $31, 0($29)
jal write
lw $31, 0($29)
addi $29, $29, 4
move $-1, $-1
move $-1, $-1
addi $-1, $-1, 1
j label1
label3:
li $2, 0
jr $31
