.intel_syntax noprefix
.section .bss
numbuf:
 .skip 64
tempbuf:
 .skip 2024
.section .data
  msg: .asciz  "Hello, World!"

.section .text
  .globl main
main:
 # push_int
  movabs rax, 1
  push rax

  # copy_int
  mov rax, [rsp + 0]
  push rax

 # push_int
  movabs rax, 1
  push rax

 # add_int_int
  pop rax
  pop rbx
  add rax, rbx
  push rax

  # set_int
  mov rax, [rsp]
  mov [rsp + 8], rax

  # copy_int
  mov rax, [rsp + 8]
  push rax

 # push_int
  movabs rax, 1
  push rax

 # add_int_int
  pop rax
  pop rbx
  add rax, rbx
  push rax

  # set_int
  mov rax, [rsp]
  mov [rsp + 16], rax

  # copy_int
  mov rax, [rsp + 16]
  push rax

 # push_int
  movabs rax, 1
  push rax

 # add_int_int
  pop rax
  pop rbx
  add rax, rbx
  push rax

  # set_int
  mov rax, [rsp]
  mov [rsp + 24], rax

  # copy_int
  mov rax, [rsp + 24]
  push rax

 # push_int
  movabs rax, 1
  push rax

 # add_int_int
  pop rax
  pop rbx
  add rax, rbx
  push rax

  # set_int
  mov rax, [rsp]
  mov [rsp + 32], rax

  # copy_int
  mov rax, [rsp + 32]
  push rax

 # push_int
  movabs rax, 1
  push rax

 # add_int_int
  pop rax
  pop rbx
  add rax, rbx
  push rax

  # set_int
  mov rax, [rsp]
  mov [rsp + 40], rax

  # copy_int
  mov rax, [rsp + 40]
  push rax

 # push_int
  movabs rax, 1
  push rax

 # add_int_int
  pop rax
  pop rbx
  add rax, rbx
  push rax

  # set_int
  mov rax, [rsp]
  mov [rsp + 48], rax

  # copy_int
  mov rax, [rsp + 48]
  push rax

  mov rax, [rsp]
  push rax
  pop rdi
  call int_to_str
  mov rax, 1
  mov rdi, 1
  lea rsi, [numbuf]
  mov rdx, 20
  syscall
  lea rdi, [numbuf]
  mov rcx, 20
  mov al, 0
  rep stosb


  movabs rax, 60
  pop rdi
  syscall


int_to_str:
    push rbx
    push rdi
    push rsi


    mov rax, rdi                    # Get input integer from RDI
    lea rdi, [numbuf + 20]          # Point to the end of numbuf
    mov BYTE PTR [rdi], 0           # Null-terminate string
    dec rdi                         # Move pointer back for storing digits

    test rax, rax
    jnz .convert_loop_int_to_str    
    mov BYTE PTR [rdi], '0'         # Special case for zero
    jmp .done_int_to_str

.convert_loop_int_to_str:
    xor rdx, rdx                    # Clear RDX for division
    mov rbx, 10                     # Divisor
    div rbx                         # RAX /= 10, remainder in RDX

    add dl, '0'                     # Convert remainder to ASCII
    mov BYTE PTR [rdi], dl          # Store ASCII digit
    dec rdi                         # Move backward

    test rax, rax
    jnz .convert_loop_int_to_str

.done_int_to_str:
    inc rdi                         # Adjust pointer to start of the number
    mov rax, rdi                    # Return pointer to the converted string

    pop rsi
    pop rdi
    pop rbx
    ret


