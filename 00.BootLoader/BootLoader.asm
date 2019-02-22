[ORG 0x00]
[BITS 16]
SECTION .text
jmp 	0x07c0:START		;;cs �������Ϳ� 0x07c0 ����
TOTALSECTORCOUNT: dw    0x02	;;OS �̹��� ���� ũ��
KERNEL32SECTORCOUNT: dw 0x02 ;
START:						;;������ ���׸�Ʈ �������� ����
	mov ax, 0x07c0			;;ds ���׸�Ʈ ����
	mov ds, ax 				;;
	mov ax, 0xb800			;es ���׸�Ʈ ����(���� �޸�)
	mov es, ax				;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;; ss ��������(����) ����						;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ax, 0x00			;����(0x0000~0xffff)
	mov ss, ax				;
	mov bp, 0xfffe			;bp �ʱ�ȭ
	mov sp, 0xfffe			;sp �ʱ�ȭ

	mov si,0x00				;si 0
.SCREENCLEARLOOP:
	mov byte[es:si],0		;���� ����
	mov byte[es:si+1],0x0A	;�Ӽ� ��������.��� �۾�
	add si,0x02				;���� ĭ����
	cmp si,80*25*2
	jl .SCREENCLEARLOOP

	push MESSAGE
	push 0
	push 0
	call PRINTMESSAGE
	add sp, 6

	push IMAGELOADINGMESSAGE
	push 0
	push 1
	call PRINTMESSAGE
	add sp, 6
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;��ũ ����/�б�								;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RESETDISK:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;BIOS service call					;;;;;;;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ax, 0
	mov dl, 0
	int 0x13
	jc HANDLERESETERROR

	mov si, 0x1000
	mov es, si
	mov bx, 0x0000
	mov di, word[TOTALSECTORCOUNT]
READDATA:
	cmp di, 0
	je READEND
	sub di, 0x01
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;BIOS read service call
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ah, 0x02	;service = read
	mov al, 0x01	;how many?
	mov ch, byte[ TRACKNUMBER ]	;
	mov cl, byte[ SECTORNUMBER ];
	mov dh, byte[ HEADNUMBER ]	;
	mov dl, 0x00				;floppy
	int 0x13
	jc HANDLEDISKERROR
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;address, sector, track, head �缳��
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	add si, 0x20
	mov es, si
	mov al, byte[SECTORNUMBER]
	add al, 0x01
	mov byte[SECTORNUMBER], al
	cmp al,19
	jl READDATA

	xor byte[HEADNUMBER], 0x01
	mov byte[SECTORNUMBER], 0x01
	cmp byte[HEADNUMBER], 0x00
	jne READDATA

	add byte[TRACKNUMBER], 0x01
	jmp READDATA
READEND:
	push LOADINGCOMPLETE
	push 20
	push 1
	call PRINTMESSAGE
	add sp, 6
	jmp 0x1000:0x0000
HANDLEDISKERROR:
	push DISKERRORMESSAGE
	push 20
	push 1
	call PRINTMESSAGE
	jmp $
HANDLERESETERROR:
	push RESETERRORMESSAGE
	push 20
	push 1
	call PRINTMESSAGE
	jmp $
PRINTMESSAGE:				;char *, short x, short y
	push bp					;bp ���� �߰�
	mov bp,sp				;
	push ax
	push bx
	push si
	push di
	push es

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ax, 0xb800			;
	mov es, ax				;es ���׸�Ʈ ����

	mov ax,word[ss:bp+4]	;ax=y
	mov si, 160
	mul si					;ax=y*80
	mov di,ax				;

	mov ax,word[ss:bp+6]	;ax=x
	mov si,	2
	mul si
	add di,ax				;dx�� ��巹�� ����
	mov si,word[ss:bp+8]	;si=char*
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.LOOP:
	mov bl,byte[ds:si]		;
	cmp bl,0x00
	je .PRINTEND			;
	mov byte[es: di], bl	;
	add si,0x01				;
	add di,0x02				;
	jmp .LOOP
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.PRINTEND:
	pop es
	pop di
	pop si
	pop bx
	pop ax
	pop bp					;
	ret						;
MESSAGE: db "Mint64 OS Boot Loader Starting...",0x00
RESETERRORMESSAGE: db "DISK reset error...",0x00
DISKERRORMESSAGE: db "DISK read error...",0x00
IMAGELOADINGMESSAGE: db "OS IMAGE Loading...",0x00
LOADINGCOMPLETE:db "Loading Successed...",0x00
SECTORNUMBER: db 0x02
HEADNUMBER: db 0x00
TRACKNUMBER: db 0x00
times 510-($-$$) db 0x00
db 0x55
db 0xAA
