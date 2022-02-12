;; Wait() implementation. (Slightly) modified version of libgc waitpid()/wait4().
;; Required: wait() is licensed by GLPL 2.1 (or greater), so this disclaimer
;; is here to acknowledge that.
;;
;; This implementation just accepts the processId and is for reference
;;
;; THIS PROGRAM WAS BUILT WITH YASM; X86-64 (64-bit linux)
;;
;; Formal Stub:
;;
;; wait(int32_t processId, int32_t* statusLocation, int32_t options, struct __rusage64 *usage)
;;
;; Custom Stub:
;;
;; wait(int32_t processId)
;;
;; processId            - At least 4 Bytes
;; statusLocation       - At least 4 Bytes (Gets written to by system call)
;; options              - At least 4 Bytes
;; arguments            - At least 8 Bytes
;; parentThreadId       - At least 8 Bytes
;; threadLocalStorage   - At least 8 Bytes
;; childThreadId        - At least 8 Bytes
;;
;; Resource Usage - (struct_rusage.h) 128 Bytes long, first two 8-Byte sections point to 16 Bytes each
;;
;; pid can be:
;;
;;      - < -1 (Wait for any child process whose process group id is equal to the absolute value of pid)
;;      - = -1 (Wait for any child process)
;;      - =  0 Wait for any child process whose process group id is equal to that of the calling process at the time of the invoation
;;      - > 0 Wait for the child whose process is equal to the value of pid
;;
;; options can be 0 or a mask of:
;;
;;      - WNOHANG    1 (Return immediately if no child has exited)
;;      - WUNTRACED  2 (Untraced child stop)
;;      - WCONTINUED 4 (when a child has been resumed by SIGCONT)
;;
;; Per the standard calling convention (x86-64 ABI) The parameters are set as
;; expected by the kernel:
;;
;; rdi    : processId
;; rsi    : statusLocation
;; rdx    : options
;; rcx    : resourceUsage (as listed above)
;;
;; TODO: We want an .eh_frame

;; ------------
;; Data Section

section .data

    ;; -------------------
    ;; System Call Numbers

    WAIT equ 61          ; System call code for wait

;; -----------
;; Wait Start

section .text
global _wait
_wait:

    xor rsi, rsi                            ; Clear statusLocation
    xor rdx, rdx                            ; Clear options
    xor rcx, rcx                            ; Clear resourceUsage
    mov rax, WAIT                           ; Set the system call number
    syscall                                 ; Invoke the system call
    ret                                     ; Return to the call site
