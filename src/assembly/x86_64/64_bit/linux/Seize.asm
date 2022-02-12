;; Seize() implementation. (Slightly) modified version of libgc ptrace(SEIZE)
;; Required: ptrace() is licensed by GLPL 2.1 (or greater), so this disclaimer
;; is here to acknowledge that.
;;
;; This implementation just accepts the processId and is for reference
;;
;; THIS PROGRAM WAS BUILT WITH YASM; X86-64 (64-bit linux)
;;
;; Formal Stub:
;;
;; ptrace(int32_t request, int32_t processId, void* address, int32_t data, void* address2)
;;
;; Custom Stub:
;;
;; Seize(int32_t processId);
;;
;; request                      - At least 4 Bytes
;; processId                    - At least 4 Bytes
;; address                      - At least 8 Bytes (Gets written to by system call)
;; data(return code address)    - At least 4 Bytes
;; address2                     - At least 8 Bytes
;;
;; Per the standard calling convention (x86-64 ABI) The parameters are set as
;; expected by the kernel:
;;
;; rdi    : request
;; rsi    : processId
;; rdx    : address
;; rcx    : data (address)
;; r10    : address2
;;
;; TODO: We want an .eh_frame

;; ------------
;; Data Section

section .data

    ;; -------------------
    ;; System Call Numbers

    PTRACE    equ 101          ; System call code for ptrace

    ;; --------
    ;; Requests

    ;; Indicate that the process making this request should be traced.
    ;; All signals received by this process can be intercepted by its
    ;;  parent, and its parent can use the other `ptrace' requests.
    ;; PTRACE_TRACEME                  equ 0

    ;; Return the word in the process's text space at address ADDR.
    ;; PTRACE_PEEKTEXT                 equ 1

    ;; Return the word in the process's data space at address ADDR.
    ;; PTRACE_PEEKDATA                 equ 2

    ;; Return the word in the process's user area at offset ADDR.
    ;; PTRACE_PEEKUSER                 equ 3

    ;; Write the word DATA into the process's text space at address ADDR.
    ;; PTRACE_POKETEXT                 equ 4

    ;; Write the word DATA into the process's data space at address ADDR.
    ;; PTRACE_POKEDATA                 equ 5

    ;; Write the word DATA into the process's user space at offset ADDR.
    ;; PTRACE_POKEUSER                 equ 6

    ;; Continue the process.
    ;; PTRACE_CONT                     equ 7

    ;; Kill the process.
    ;; PTRACE_KILL                     equ 8

    ;; Single step the process.
    ;; This is not supported on all machines
    ;; PTRACE_SINGLESTEP               equ 9

    ;; Attach to a process that is already running.
    ;; PTRACE_ATTACH                   equ 10

    ;; Detach from a process attached to with   PTRACE_ATTACH.
    ;; PTRACE_DETACH                   equ 11

    ;; Get the process's registers (not including floating-point registers)
    ;; and put them in the `struct regs' (see <machine/regs.h>) at ADDR.
    ;; PTRACE_GETREGS                  equ 12

    ;; Set the process's registers (not including floating-point registers)
    ;; to the contents of the `struct regs' (see <machine/regs.h>) at ADDR.
    ;; PTRACE_SETREGS                  equ 13

    ;; Get the process's floating point registers and put them
    ;; in the `struct fp_status' (see <machine/regs.h>) at ADDR.
    ;; PTRACE_GETFPREGS                equ 14

    ;; Set the process's floating point registers to the contents
    ;; of the `struct fp_status' (see <machine/regs.h>) at ADDR.
    ;; PTRACE_SETFPREGS                equ 15

    ;; Read DATA bytes from the process's data space at address ADDR.
    ;; Put the result starting at address ADDR2 in the caller's
    ;; address space.
    ;; PTRACE_READDATA                 equ 16

    ;; Write DATA bytes from ADDR2 in the caller's address space into
    ;; the process's data space at address ADDR.
    ;; PTRACE_WRITEDATA                equ 17 ;; Or maybe PTRACE_DETACH?

    ;; Read DATA bytes from the process's text space at address ADDR.
    ;; Put the result starting at address ADDR2 in the caller's
    ;; address space.
    ;; PTRACE_READTEXT                 equ 18 ;; Ormaybe PTRACE_GETFPXREGS

    ;; Write DATA bytes from ADDR2 in the caller's address space into
    ;; the process's text space at address ADDR.
    ;; PTRACE_WRITETEXT                equ 19 ;; Or maybe PTRACE SETFPXREGS

    ;; Read the floating-point accelerator unit registers and
    ;; put them into the `struct fpa_regs' (see <machine/regs.h>) at ADDR.
    ;; PTRACE_GETFPAREGS               equ 20

    ;; Write the floating-point accelerator unit registers from
    ;; the contents of the `struct fpa_regs' at ADDR.
    ;; PTRACE_SETFPAREGS               equ 21

    ;; Continue and stop at the next entry to or return from syscall.
    ;; PTRACE_SYSCALL                  equ 24

    ;; Get a TLS entry in the GDT.
    ;; PTRACE_GET_THREAD_AREA          equ 25

    ;; Change a TLS entry in the GDT.
    ;; PTRACE_SET_THREAD_AREA          equ 26

    ;; PTRACE_ARCH_PRCTL               equ 30

    ;; Continue and stop at the next syscall, it will not be executed.
    ;; PTRACE_SYSEMU                   equ 31

    ;; Single step the process, the next syscall will not be executed.
    ;; PTRACE_SYSEMU_SINGLESTEP        equ 32

    ;; Execute process until next taken branch.
    ;; PTRACE_SINGLEBLOCK              equ 33

    ;; Set ptrace filter options.
    ;; PTRACE_SETOPTIONS               equ 0x4200

    ;; Get last ptrace message.
    ;; PTRACE_GETEVENTMSG              equ 0x4201

    ;; Get siginfo for process.
    ;; PTRACE_GETSIGINFO               equ 0x4202

    ;; Set new siginfo for process.
    ;; PTRACE_SETSIGINFO               equ 0x4203

    ;; Get register content.
    ;; PTRACE_GETREGSET                equ 0x4204

    ;; Set register content.
    ;; PTRACE_SETREGSET                equ 0x4205

    ;; Like PTRACE_ATTACH, but do not force tracee to trap and do not affect
    ;; signal or group stop state.
    PTRACE_SEIZE                    equ 0x4206

    ;; Trap seized tracee.
    ;; PTRACE_INTERRUPT                equ 0x4207

    ;; Wait for next group event.
    ;; PTRACE_LISTEN                   equ 0x4208

    ;; Retrieve siginfo_t structures without removing signals from a queue.
    ;; PTRACE_PEEKSIGINFO              equ 0x4209

    ;; Get the mask of blocked signals.
    ;; PTRACE_GETSIGMASK               equ 0x420a

    ;; Change the mask of blocked signals.
    ;; PTRACE_SETSIGMASK               equ 0x420b

    ;; Get seccomp BPF filters.
    ;; PTRACE_SECCOMP_GET_FILTER       equ 0x420c

    ;; Get seccomp BPF filter metadata.
    ;; PTRACE_SECCOMP_GET_METADATA     equ 0x420d

    ;; Get information about system call.
    ;; PTRACE_GET_SYSCALL_INFO         equ 0x420e

    ;; Get rseq configuration information.
    ;; PTRACE_GET_RSEQ_CONFIGURATION   equ 0x420f

;; -----------
;; Seize Start

section .text
global __seize
__seize:

    xor rcx, rcx            ; Clear address
    xor rdx, rdx            ; Clear data (address)
    xor r10, r10            ; Clear address2
    mov rsi, rdi            ; Shift the processId argument
    mov rdi, PTRACE_SEIZE   ; Load the request
    mov rax, PTRACE         ; Set the system call number
    syscall                 ; Invoke the system call
    ret                     ; Return to the call site
