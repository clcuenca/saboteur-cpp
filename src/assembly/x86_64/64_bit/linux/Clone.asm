;; Clone() implementation. (Slightly) modified version of libgc clone().
;; Required: clone() is licensed by GLPL 2.1 (or greater), so this disclaimer
;; is here to acknowledge that.
;;
;; THIS PROGRAM WAS BUILT WITH YASM; X86-64 (64-bit linux)
;;
;; Included comment:
;;
;; clone() is even more special than fork() as it mucks with stacks
;; and invokes a function in the right context after its all over.
;; Move this:
;; void*   - 8 Bytes
;; int32_t - 4 Bytes
;;
;; Method Stub (Userland):
;;
;; int32_t clone (void* function, void* childStack, int32_t flags,
;;                void* arguments, int32_t* parentThreadId, void* threadLocalStorage,
;;                int32_t* childThreadId);
;;
;; function             - At least 8 Bytes
;; childStack           - At least 8 Bytes
;; flags                - At least 4 Bytes
;; arguments            - At least 8 Bytes
;; parentThreadId       - At least 8 Bytes
;; threadLocalStorage   - At least 8 Bytes
;; childThreadId        - At least 8 Bytes
;;
;; Per the standard calling convention (x86-64 ABI) The parameters are set as
;; expected by the kernel:
;;
;; rdi    : function
;; rsi    : childStack
;; rdx    : flags
;; rcx    : arguments
;; r8     : parentThreadId
;; r9     : threadLocalStorage
;; %rsp+8 : childThreadId
;;
;; Method Stub (Kernel):
;;
;; int32_t clone (int32_t flags, void* childStack, int32_t* parentThreadId,
;;                int32_t* childThreadId, void* threadLocalStorage);
;;
;; flags                - At least 4 Bytes
;; childStack           - At least 8 Bytes
;; parentThreadId       - At least 8 Bytes
;; childThreadId        - At least 8 Bytes
;; threadLocalStorage   - At least 8 Bytes
;;
;; Per the standard calling convention (x86-64 ABI) The parameters are set as
;; expected by the kernel:
;;
;; rax : System Call Number
;; rdi : flags
;; rsi : childStack
;; rdx : parentThreadId
;; r10 : childThreadId
;; r8  : threadLocalStorage
;;
;; TODO: We want an .eh_frame

;; ------------
;; Data Section

section .data

    ;; -----------
    ;; Error Codes

    EINVAL  equ 0x40000016  ; Invalid Argument

    ;; ------------
    ;; Status Codes

    SUCCESS equ 0x00000000  ; Operation success

    ;; ----------------
    ;; File Descriptors

    STDOUT  equ 1           ; Standard Output

    ;; -------------------
    ;; System Call Numbers

    WRITE   equ 1           ; System call code for write
    CLONE   equ 56          ; System call code for clone
    EXIT    equ 60          ; System call code for exit

    ;; ----------
    ;; Characters

    LF      equ 0x0a        ; Linefeed
    NULL    equ 0x00        ; NULL

    ;; ---------------------
    ;; Error message lengths

    NO_EXECUTION_ADDRESS_LENGTH equ 46
    NO_STACK_ADDRESS_LENGTH     equ 42
    CLONE_FAILED_LENGTH         equ 28

    ;; --------------
    ;; Error Messages

    NO_EXECUTION_ADDRESS db 'Clone Error: No execution address specified.' , LF, NULL
    NO_STACK_ADDRESS     db 'Clone Error: No stack address specified.'     , LF, NULL
    CLONE_FAILED         db 'Clone Error: Clone failed.'                   , LF, NULL

;; -----------
;; Clone Start

section .text
global _clone
_clone:

    ;; ---------------------
    ;; Register Preservation

    push rbx                                ; Save the rbx register per the psABI

    ;; ----------------------------
    ;; Check NULL execution address

    test  rdi, rdi                          ; Check if the execution address is null
    jz    error_no_execution                ; If the address is null, go to the error

    ;; ---------------------------
    ;; Align the stack to 16 bytes
    ;; per the x86-64 psABI (SCC)

    and  rsi, $-16                          ; Align the stack to 16 bytes
    jz   error_no_stack                     ; If the stack is null, go to the error

    ;; ---------------
    ;; Setup execution

    mov  qword[rsi - 8], rcx                ; Load the argument onto the stack
    sub     rsi        , $16                ; Set the new top of the stack (We account for the first argument we just pushed)
    mov  qword[rsi]    , rdi                ; Load the execution address onto the stack (it gets popped later)

    ;; -------------
    ;; Perform Clone

    mov  rdi, rdx                           ; Set the flag parameter as expected by the kernel
    mov  rdx, r8                            ; Set the parentThreadId as expected by the kernel
    mov  r8 , r9                            ; Set the threadLocalStorage as expected by the kernel
    mov  r10, 8(rsp)                        ; Set the childThreadId as expected by the kernel
    mov  rax, CLONE                         ; Set the system call number
    syscall                                 ; Invoke the syscall
    test rax, rax                           ; The child will resume execution here
    jl   clone_failure                      ; Error code returned, If clone failed, go to clone failure
    jz   thread_execute                     ; Child receives no pid, so begin execution
    pop  rbx                                ; Restore the register
    ret                                     ; If we're here, we must be in the parent, return

;; ------------------
;; Address Null error

error_no_execution:

    ;; ---------------
    ;; Display Message

    mov  rax, WRITE                         ; Load the write system call number
    mov  rbx, STDOUT                        ; Load the file descriptor, stdout
    mov  rcx, NO_EXECUTION_ADDRESS          ; Load the address of the error message
    mov  rdx, NO_EXECUTION_ADDRESS_LENGTH   ; Load the message length
    syscall                                 ; Print the error message
    mov  rax, $-EINVAL                      ; Set EINVAL
    pop  rbx                                ; Restore the register
    ret                                     ; Return

;; ----------------
;; Stack NULL error

error_no_stack:

    ;; ---------------
    ;; Display Message

    mov  rax, WRITE                         ; Load the write system call number
    mov  rbx, STDOUT                        ; Load the file descriptor, stdout
    mov  rcx, NO_STACK_ADDRESS              ; Load the address of the error message
    mov  rdx, NO_STACK_ADDRESS_LENGTH       ; Load the message length
    syscall                                 ; Print the error message
    mov  rax, $-EINVAL                      ; Set EINVAL
    pop  rbx                                ; Restore the register
    ret                                     ; Return

;; -------------
;; Clone Failure

clone_failure:

    ;; ---------------
    ;; Display Message

    push rax                                ; Save the error code from clone
    mov  rax, WRITE                         ; Load the write system call number
    mov  rbx, STDOUT                        ; Load the file descriptor, stdout
    mov  rcx, NO_STACK_ADDRESS              ; Load the address of the error message
    mov  rdx, NO_STACK_ADDRESS_LENGTH       ; Load the message length
    syscall                                 ; Print the error message
    pop  rax                                ; Retrieve the error code
    pop  rbx                                ; Restore the register
    ret                                     ; Return

;; ----------------
;; Thread Execution

thread_execute:

    ;; -----------
    ;; Child setup

    xor rbp, rbp                            ; Clear the frame pointer, the ABI suggets this be done to mark the end of the stack

    ;; -----------------
    ;; Extract arguments

    pop  rax                                ; Retrieve the function
    pop  rdi                                ; Retrieve the argument
    call rax                                ; Invoke the function

    ;; -------------
    ;; Thread Finish

    mov rdi, rax                            ; Set the return code as the first argument
    mov rax, EXIT                           ; Set the system call number
    syscall                                 ; Invoke the system call
