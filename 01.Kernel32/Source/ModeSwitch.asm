[BITS 32]
;void kReadCPUID( DWORD dwEAX, DWORD* pdwEAX, DWORD* pdwEBX, DWORD* pdwECX, DWORD* pdwEDX);
;void kSwitchAndExecute64bitKernel(void);
global kReadCPUID, 	kSwitchAndExecute64bitKernel
SECTION .text
kReadCPUID:
	push ebp;
	mov	 ebp,esp;
	push eax;
	push ebx;
	push ecx;
	push edx;
	push esi;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, dword[ebp +  8] ; DWORD dwEAX
	cpuid

	mov esi, dword[ebp + 12] ; DWORD* pdwEAX
	mov dword [esi], eax;

	mov esi, dword[ebp + 16] ; DWORD* pdwEBX
	mov dword [esi], ebx;

	mov esi, dword[ebp + 20] ; DWORD* pdwECX
	mov dword [esi], ecx;

	mov esi, dword[ebp + 24] ; DWORD* pdwEDX
	mov dword [esi], edx;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	pop esi;
	pop edx;
	pop ecx;
	pop ebx;
	pop eax;
	pop ebp;
	ret;
kSwitchAndExecute64bitKernel:
	mov eax, cr4;
	or eax, 0x620;
	mov cr4, eax;

	mov eax, 0x100000;
	mov cr3, eax;

	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x0100
	wrmsr

	mov eax, cr0
	or  eax, 0xE000000E
	xor eax, 0x60000004

	mov cr0, eax
	;ret ;;;;;;;;;;;;;

	jmp 0x08:0x200000 ;;;;;;;;;;;;;;;;;change

