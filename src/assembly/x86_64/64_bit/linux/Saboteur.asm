;; Saboteur Execution. This program implements the startup and lifecycle
;; of a form of Saboteur thread (it dynamically injects, extracts, delays
;; the thread's executing code). This is by no means complete. There is
;; several behaviors that need to be tested and analyzed.
;;
;; Version 0.1.0
;;
;; THIS PROGRAM WAS BUILT WITH YASM; X86-64 (64-bit linux)
;;
;; Method Stub:
;;
;; int32_t Saboteur(Saboter* saboteur);
;;
;; saboteur             - At least 8 Bytes
;;
;; Per the standard calling convention (x86-64 ABI) The parameters are set as
;; expected by the kernel:
;;
;; rdi    : saboteur
;; rsi    : n/a
;; rdx    : n/a
;; rcx    : n/a
;; r8     : n/a
;; r9     : n/a
;; %rsp+8 : n/a
;;
;; Kernel Arguments:
;;
;; Per linux The parameters are set as
;; expected by the kernel:
;;
;; rax : n/a
;; rdi : n/a
;; rsi : n/a
;; rdx : n/a
;; r10 : n/a
;; r8  : n/a
;;
;; TODO: We want an .eh_frame

;; ------------
;; Data Section

section .data

    ;; -----------
    ;; Error Codes

    EINVAL                  equ 0x40000016  ; Invalid Argument

    ;; ------------
    ;; Status Codes

    SUCCESS                 equ 0x00000000  ; Operation success

    ;; ----------------
    ;; File Descriptors

    STDOUT                  equ 1           ; Standard Output

    ;; ------
    ;; Values

    PAGE_SIZE               equ 4096; 16384       ; 16 kB Page Size. So much memory; so robust!

    ;; -------------------
    ;; System Call Numbers

    WRITE                   equ 1           ; System call code for write
    MMAP                    equ 9           ; System call code for mmap
    CLONE                   equ 56          ; System call code for clone
    EXIT                    equ 60          ; System call code for exit
    _WAIT                   equ 61          ; System call code for wait
    PTRACE                  equ 101         ; System call code for ptrace

    ;; ---------------
    ;; Ptrace Requests
PTRACE_SYSCALL              equ 24
    PTRACE_SEIZE            equ 0x4206      ; Traces the child with the given processId

    ;; ---------
    ;; Mmap Flag

    PROT_EXEC               equ 0x01        ; Page can be executed
    PROT_WRITE              equ 0x02        ; Page can be written
    PROT_READ               equ 0x04        ; Page can be read
    MAP_SHARED              equ 0x0010      ; Share changes.
    MAP_PRIVATE             equ 0x0002      ; Changes private; copy pages on write.
    MAP_ANON                equ 0x0020      ; Allocated from anonymous virtual memory.

    ;; -----------
    ;; Clone Flags

    CSIGNAL                 equ 0x000000ff  ; Signal mask to be sent at exit.
    CLONE_VM                equ 0x00000100  ; Set if VM shared between processes.
    CLONE_FS                equ 0x00000200  ; Set if fs info shared between processes.
    CLONE_FILES             equ 0x00000400  ; Set if open files shared between processes.
    CLONE_SIGHAND           equ 0x00000800  ; Set if signal handlers shared.
    CLONE_PIDFD             equ 0x00001000  ; Set if a pidfd should be placed in parent.
    CLONE_PTRACE            equ 0x00002000  ; Set if tracing continues on the child.
    CLONE_VFORK             equ 0x00004000  ; Set if the parent wants the child to wake it up on mm_release.
    CLONE_PARENT            equ 0x00008000  ; Set if we want to have the same  parent as the cloner.
    CLONE_THREAD            equ 0x00010000  ; Set to add to same thread group.
    CLONE_NEWNS             equ 0x00020000  ; Set to create new namespace.
    CLONE_SYSVSEM           equ 0x00040000  ; Set to shared SVID SEM_UNDO semantics.
    CLONE_SETTLS            equ 0x00080000  ; Set TLS info.
    CLONE_PARENT_SETTID     equ 0x00100000  ; Store TID in userlevel buffer before MM copy.
    CLONE_CHILD_CLEARTID    equ 0x00200000  ; Register exit futex and memory location to clear.
    CLONE_DETACHED          equ 0x00400000  ; Create clone detached.
    CLONE_UNTRACED          equ 0x00800000  ; Set if the tracing process can't force CLONE_PTRACE on this clone.
    CLONE_CHILD_SETTID      equ 0x01000000  ; Store TID in userlevel buffer in the child.
    CLONE_NEWCGROUP         equ 0x02000000  ; New cgroup namespace.
    CLONE_NEWUTS            equ 0x04000000  ; New utsname group.
    CLONE_NEWIPC            equ 0x08000000  ; New ipcs.
    CLONE_NEWUSER           equ 0x10000000  ; New user namespace.
    CLONE_NEWPID            equ 0x20000000  ; New pid namespace.
    CLONE_NEWNET            equ 0x40000000  ; New network namespace.
    CLONE_IO                equ 0x80000000  ; Clone I/O context.

    ;; -------------
    ;; POSIX Signals

    SIGURG                  equ 16          ; Urgent data is available at a socket.  */
    SIGSTOP                 equ 17          ; Stop, unblockable.  */
    SIGTSTP                 equ 18          ; Keyboard stop.  */
    SIGCONT                 equ 19          ; Continue.  */
    SIGCHLD                 equ 20          ; Child terminated or stopped.  */
    SIGTTIN                 equ 21          ; Background read from control terminal.  */
    SIGTTOU                 equ 22          ; Background write to control terminal.  */
    SIGPOLL                 equ 23          ; Pollable event occurred (System V).  */
    SIGXCPU                 equ 24          ; CPU time limit exceeded.  */
    SIGVTALRM               equ 26          ; Virtual timer expired.  */
    SIGPROF                 equ 27          ; Profiling timer expired.  */
    SIGXFSZ                 equ 25          ; File size limit exceeded.  */
    SIGUSR1                 equ 30          ; User-defined signal 1.  */
    SIGUSR2                 equ 31          ; User-defined signal 2.  */

    CREATED                 equ 0x0000000000000801
    WAITING                 equ 0x0000000000000803
    STARTED                 equ 0x0000000000000805
    TERMINATED              equ 0x0000000000000809
    TERMINATE               equ 0x0000000000000811
    SUSPENDED               equ 0x0000000000000821
    RESUMING                equ 0x0000000000000841
    SWAPPING                equ 0x0000000000000881
    SELF_SWAP               equ 0x0000000000000901
    SELF_SUSPEND            equ 0x0000000000000a01
    SUICIDE                 equ 0x0000000000000c09
    ATTACHED                equ 0x0000000000000801

    ;; ----------
    ;; Characters

    LF              equ 0x0a        ; Linefeed
    NULL            equ 0x00        ; NULL

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
global Lifecycle
Lifecycle:

    ;; ---------------------
    ;; Register Preservation

    push rbx                                ; Save the rbx register per the psABI
    mov  rbx, rdi                           ; Load the thread handle

    ;; ----------------------
    ;; Process Stack Creation

    xor rdi, rdi                            ; Clear address argument; we want a new one
    mov rsi, PAGE_SIZE                      ; Load the page size
    xor rdx, rdx                            ; Clear the flag argument
    or  rdx, PROT_READ                      ; Set the page as readable
    or  rdx, PROT_WRITE                     ; Set the page as writeable
    xor r10, r10                            ; Clear the flag argument
    or  r10, MAP_PRIVATE                    ; Don't share pages
    or  r10, MAP_ANON                       ; No fd
    mov r8 , -1                             ; Clear the file descriptor argument
    mov r9 , 0                              ; Clear the offset
    mov rax, MMAP                           ; Set the system call number
    syscall                                 ; Invoke it

    ;; ---------------------
    ;; Setup the child stack

    mov qword[rbx + 16], rax                ; Set the stack address member in our Saboteur
    mov qword[rax]     , 0                  ; Delimit the stack
    add rax            , 8                  ; Create some space for our thread handle (we're upside down)
    mov qword[rax]     , rbx                ; Load the thread handle onto the stack
    add rax            , 8                  ; Create some space for the thread observer
    mov rcx            , qword[rbx + 32]    ; Set the address of the thread observer
    mov qword[rax]     , rcx                ; Set the address of the thread observer
    sub rax            , 16                 ; Create some space for the next item

    ;; ---------------------------
    ;; Thread Creation - Set Flags

    xor rdi, rdi                            ; Clear the flag argument
    or  rdi, CLONE_PARENT_SETTID            ; All threads share the same parent
    or  rdi, CLONE_PARENT                   ; Copy parent
    or  rdi, CLONE_VM                       ; Share the address space
    or  rdi, CLONE_SIGHAND                  ; Share signal handlers
    or  rdi, CLONE_FILES                    ; Share File descriptor
    or  rdi, CLONE_FS                       ; Share File system
    or  rdi, CLONE_IO                       ; Share I/O time

    ;; ---------------
    ;; Thread Creation

    mov rsi, rax                            ; Load the child stack address to the correct argument
    xor rcx, rcx                            ; We used this recently, let's clear it just in case
    xor rdx, rdx                            ; Clear the parent thread id (this process)
    xor r10, r10                            ; Clear the child thread id (new)
    xor r8 , r8                             ; Clear TLS (create new)
    mov rax, CLONE                          ; Load the system call number
    syscall                                 ; DUALITY
    jl clone_failure                        ; We failed, go handle the error
    jz thread_execute                       ; The system call returned 0, we're the child
    mov qword[rbx], rax                     ; Load the thread id

    ;; -------------
    ;; Thread Attach

    xor rdx, rdx                            ; Clear data (address)
    xor rcx, rcx                            ; Clear address
    xor r10, r10                            ; Clear address2
    mov rsi, rax                            ; Load the process id argument
    mov rdi, PTRACE_SEIZE                   ; Load the request
    mov rax, PTRACE                         ; Set the system call number
    syscall                                 ; Invoke the system call

    ;; -----------
    ;; Thread Wait

    xor rsi, rsi                            ; Clear statusLocation
    xor rdx, rdx                            ; Clear options
    xor rcx, rcx                            ; Clear resourceUsage
    mov rdi, qword[rbx]                     ; Set the process id (the thread we just created)
    mov rax, _WAIT                          ; Set the system call number
    syscall                                 ; Invoke the system call
    ret                                     ; Return to the call site

    ;; ---------------
    ;; Thread Continue

    xor rdx, rdx                            ; Clear data (address)
    xor rcx, rcx                            ; Clear address
    xor r10, r10                            ; Clear address2
    mov rsi, qword[rbx]                     ; Load the process id argument
    mov rdi, PTRACE_SEIZE                   ; Load the request
    mov rax, PTRACE                         ; Set the system call number
    syscall                                 ; Invoke the system call

    ;; -------------
    ;; Parent Finish

    pop  rbx                                ; Restore the register
    ret                                     ; If we're here, we must be in the parent, return

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

    mov  rbx, rbp                           ; Load the thread handle
    add  rbx, 16                            ; Point to the thread handle. Not sure why it's offset but ok

    ;; ----------------
    ;; Set syscall stop

    xor rdx, rdx                            ; Clear data (address)
    xor rcx, rcx                            ; Clear address
    xor r10, r10                            ; Clear address2
    xor rsi, rsi                            ; Load the process id argument
    mov rdi, PTRACE_SYSCALL                 ; Load the request
    mov rax, PTRACE                         ; Set the system call number
    syscall                                 ; Invoke the system call

    ;; ---------
    ;; Thread Id

    mov rax, 0x00ba                         ; Load the system call
    syscall                                 ; Invoke it
    mov [rbx], rax                          ; Set the thread id

    ;; ----------
    ;; Child stop

    ;mov  rax, qword[rbx + 48]               ; Load kill()
    ;mov  rdi, qword[rbx]                    ; Load the thread id
    ;mov  rsi, SIGSTOP                       ; Set the stop signal
    ;call rax                                ; Invoke stop to let the parent know we're alive

    ;; ---------------
    ;; Create Dispatch

    mov  rax, qword[rbx + 8]                 ; Load the thread state
    xor  rax, rax                            ; Clear state
    or   rax, CREATED                        ; Set the state as created
    mov  rax, qword[rsp + 32]                ; Load the observer
    mov  rax, qword[rax + 8]                 ; Retrieve OnCreated THE OBSERVER HAS A VTABLE
    mov  rdi, rbx                            ; Load the thread handle as the first argument
    call rax                                 ; Notify the observer of the state mutation

    ;; ----------------
    ;; Thread LifeCycle

thread_lifecycle:


    ;; -------------
    ;; Thread Finish

thread_finish:

    mov rdi, rax                            ; Set the return code as the first argument
    mov rax, EXIT                           ; Set the system call number
    syscall                                 ; Invoke the system call
