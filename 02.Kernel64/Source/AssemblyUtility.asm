[BITS 64]
SECTION .text
extern kReadMemory
global kInPortByte,kOutPortByte,kLoadGDTR,kLoadTR,kLoadIDTR
global kEnableInterrupt,kDisableInterrupt,kReadRFLAGS
global kSoftInterrupt,kReadTSC
global kContextSwitch, kHlt;
; BYTE kInPortByte(WORD wPort);
;
kInPortByte:
	push rdx;
	mov rdx,rdi;
	mov rax, 0;
	in al,dx;
	pop rdx;
	ret
;
; void kOutPortByte(WORD wPort,BYTE bData);
;
kOutPortByte:
	push rdx;
	push rax;
	mov rdx,rdi;
	mov rax,rsi;
	out dx,al
	pop rax
	pop rdx
	ret;

kLoadGDTR:
	lgdt [rdi]
	ret

kLoadTR:
	ltr	di
	ret

kLoadIDTR:
	lidt [rdi]
	ret
kEnableInterrupt:
	sti
	ret
kDisableInterrupt:
	cli
	ret
kReadRFLAGS:
	pushfq
	pop rax
	ret
kSoftInterrupt:
	int 80
kReadTSC:
	push 	rdx;
	rdtsc; edx:eax
	shl 	rdx,32
	or 		rax,rdx
	pop 	rdx
	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CONTEXTSWITCH 관련 함수
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro KSAVECONTEXT 0
    push rbp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov ax, ds
    push rax
    mov ax, es
    push rax
    push fs
    push gs
%endmacro

%macro KLOADCONTEXT 0
	pop gs
    pop fs
    pop rax
    mov es, ax
    pop rax
    mov ds, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
%endmacro

;rdi: CONTEXT* pstCurrentContext, rsi: CONTEXT* pstNextContext
kContextSwitch:
	push 	rbp
	mov		rbp, rsp

	pushfq
	cmp		rdi, 0
	je		.ContextLoad
	popfq
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; ss, rsp, rflags, cs, rip 저장
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push	rax
	mov		rax, ss
	mov		qword[rdi + 23 * 8], rax
	mov		rax, rsp
	add		rax, 16
	mov		qword[rdi + 22 * 8], rax
	pushfq
	pop		rax
	mov		qword[rdi + 21 * 8], rax
	mov		ax, cs
	mov		qword[rdi + 20 * 8], rax
	mov		rax, qword[rbp + 8]
	mov		qword[rdi + 19 * 8], rax
	pop		rax
	pop		rbp
	add		rdi, 19 * 8
	mov		rsp, rdi
	sub		rdi, 19 * 8
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;나머지 레지스터 저장
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	KSAVECONTEXT
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;나머지 레지스터 로드
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.ContextLoad:
	mov		rsp, rsi
	KLOADCONTEXT
	iretq
kHlt:
	hlt
	hlt
	ret
