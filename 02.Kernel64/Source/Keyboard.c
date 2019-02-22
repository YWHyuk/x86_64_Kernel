#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"
#include "Queue.h"
BOOL kIsOutputBufferFull(void){
	if(kInPortByte(0x64)&0x01){
		return TRUE;
	}
	return FALSE;
}
BOOL kIsInputBufferFull(void){
	if(kInPortByte(0x64)&0x02){
			return TRUE;
		}
		return FALSE;
}
BOOL kWaitForACKAndPutOtherScanCode(void){
	int i,j;
	int bData;
	BOOL bResult=FALSE;
	for ( j=0;j<100;j++){
		for(i=0;i<0xFFFF;i++){
			if(kIsOutputBufferFull()==TRUE){
				break;
			}
		}
		bData= kInPortByte(0x60);
		if(bData==0xFA)
		{
			bResult = TRUE;
			break;
		}else
		{
			kConvertScanCodeAndPutQueue(bData);
		}
	}
	return bResult;
}
BOOL kActivateKeyboard(void){ //Ŀ�ǵ� 0xAE�� �Է� Ű���忡 0XF4�����ϰ� ACK ����..
	int i,j;
	BOOL bPreviousInterrupt;
	BOOL bResult;
	bPreviousInterrupt = kSetInterruptFlag(FALSE);
	//Ű���� ��Ʈ�ѷ��� Ŀ�ǵ� ����
	kOutPortByte(0x64, 0xAE);
	//�Է� ���۰� �� ������ ��ٷȴٰ� Ű���忡 Ȱ��ȭ Ŀ�ǵ带 ����
	//0xFFFF��ŭ �� ���� �Է� ���۰� ����� ���� ��ٸ�
	for(i=0;i<0xFFFF;i++){
		if(kIsInputBufferFull()==FALSE){
			break;
		}
	}
	//Ű���� Ȱ��ȭ Ŀ�ǵ� ����
	kOutPortByte(0x60, 0xF4);
	bResult = kWaitForACKAndPutOtherScanCode();
	kSetInterruptFlag(bPreviousInterrupt);
	return bResult;
}
BYTE kGetKeyboardScanCode(void){
	while(kIsOutputBufferFull()==FALSE){
		;
	}
	return kInPortByte(0x60);
}
BOOL kChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn){
	int i,j;
	BOOL bPreviousInterrupt;
	BOOL bResult;
	BOOL bData;

	bPreviousInterrupt = kSetInterruptFlag(FALSE);
	for(i=0;i<0xFFFF;i++){
		if(kIsInputBufferFull()==FALSE){
			break;
		}
	}
	kOutPortByte(0x60, 0xED);
	for(i=0;i<0xFFFF;i++){//input ���۰� ����ִٸ� Ű���尡 ������ ��...
		if(kIsInputBufferFull()==FALSE){
			break;
		}
	}

	bResult = kWaitForACKAndPutOtherScanCode();
	if(bResult == FALSE){
		kSetInterruptFlag(bPreviousInterrupt);
		return FALSE;
	}

	kOutPortByte(0x60, (bCapsLockOn<<2)|(bNumLockOn<<1)|bScrollLockOn);
	for(i=0;i<0xFFFF;i++){
		if(kIsInputBufferFull()==FALSE){
			break;
		}
	}
	bResult = kWaitForACKAndPutOtherScanCode();
	kSetInterruptFlag(bPreviousInterrupt);
	return bResult;
}
void kEnableA20Gate(void){
	BYTE bOutputPortData;
	int i;

	kOutPortByte(0x64,0xD0);
	//output buffer�� ä������ ��ٸ���...
	for(i=0;i<0xFFFF;i++){
		if(kIsOutputBufferFull()==TRUE){
			break;
		}
	}
	bOutputPortData=kInPortByte(0x60);
	bOutputPortData|=0x02; //1�� ��Ʈ�� ����
	for(i=0;i<0xFFFF;i++){
		if(kIsInputBufferFull()==FALSE){
			break;
		}
	}
	kOutPortByte(0x64, 0xD1);
	kOutPortByte(0x60, bOutputPortData);
}
void kReboot(void){
	int i;
	//output buffer�� ������� ��ٸ���...
	for(i=0;i<0xFFFF;i++){
		if(kIsInputBufferFull()==FALSE){
			break;
		}
	}
	kOutPortByte(0x64, 0xD1);
	kOutPortByte(0x60, 0x00);
	while(1);
}
//Ű�� �����ϴ� ť�� ����
static QUEUE gs_stKeyQueue;
static KEYDATA gs_vstKeyQueueBuffer[KEY_MAXQUEUECOUNT];

static KEYBOARDMANAGER gs_stKeyboardManager={0,};
static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT]=
{
			/*  0   */  {   KEY_NONE        ,   KEY_NONE        },
		    /*  1   */  {   KEY_ESC         ,   KEY_ESC         },
		    /*  2   */  {   '1'             ,   '!'             },
		    /*  3   */  {   '2'             ,   '@'             },
		    /*  4   */  {   '3'             ,   '#'             },
		    /*  5   */  {   '4'             ,   '$'             },
		    /*  6   */  {   '5'             ,   '%'             },
		    /*  7   */  {   '6'             ,   '^'             },
		    /*  8   */  {   '7'             ,   '&'             },
		    /*  9   */  {   '8'             ,   '*'             },
		    /*  10  */  {   '9'             ,   '('             },
		    /*  11  */  {   '0'             ,   ')'             },
		    /*  12  */  {   '-'             ,   '_'             },
		    /*  13  */  {   '='             ,   '+'             },
		    /*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
		    /*  15  */  {   KEY_TAB         ,   KEY_TAB         },
		    /*  16  */  {   'q'             ,   'Q'             },
		    /*  17  */  {   'w'             ,   'W'             },
		    /*  18  */  {   'e'             ,   'E'             },
		    /*  19  */  {   'r'             ,   'R'             },
		    /*  20  */  {   't'             ,   'T'             },
		    /*  21  */  {   'y'             ,   'Y'             },
		    /*  22  */  {   'u'             ,   'U'             },
		    /*  23  */  {   'i'             ,   'I'             },
		    /*  24  */  {   'o'             ,   'O'             },
		    /*  25  */  {   'p'             ,   'P'             },
		    /*  26  */  {   '['             ,   '{'             },
		    /*  27  */  {   ']'             ,   '}'             },
		    /*  28  */  {   '\n'            ,   '\n'            },
		    /*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
		    /*  30  */  {   'a'             ,   'A'             },
		    /*  31  */  {   's'             ,   'S'             },
		    /*  32  */  {   'd'             ,   'D'             },
		    /*  33  */  {   'f'             ,   'F'             },
		    /*  34  */  {   'g'             ,   'G'             },
		    /*  35  */  {   'h'             ,   'H'             },
		    /*  36  */  {   'j'             ,   'J'             },
		    /*  37  */  {   'k'             ,   'K'             },
		    /*  38  */  {   'l'             ,   'L'             },
		    /*  39  */  {   ';'             ,   ':'             },
		    /*  40  */  {   '\''            ,   '\"'            },
		    /*  41  */  {   '`'             ,   '~'             },
		    /*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
		    /*  43  */  {   '\\'            ,   '|'             },
		    /*  44  */  {   'z'             ,   'Z'             },
		    /*  45  */  {   'x'             ,   'X'             },
		    /*  46  */  {   'c'             ,   'C'             },
		    /*  47  */  {   'v'             ,   'V'             },
		    /*  48  */  {   'b'             ,   'B'             },
		    /*  49  */  {   'n'             ,   'N'             },
		    /*  50  */  {   'm'             ,   'M'             },
		    /*  51  */  {   ','             ,   '<'             },
		    /*  52  */  {   '.'             ,   '>'             },
		    /*  53  */  {   '/'             ,   '?'             },
		    /*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
		    /*  55  */  {   '*'             ,   '*'             },
		    /*  56  */  {   KEY_LALT        ,   KEY_LALT        },
		    /*  57  */  {   ' '             ,   ' '             },
		    /*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
		    /*  59  */  {   KEY_F1          ,   KEY_F1          },
		    /*  60  */  {   KEY_F2          ,   KEY_F2          },
		    /*  61  */  {   KEY_F3          ,   KEY_F3          },
		    /*  62  */  {   KEY_F4          ,   KEY_F4          },
		    /*  63  */  {   KEY_F5          ,   KEY_F5          },
		    /*  64  */  {   KEY_F6          ,   KEY_F6          },
		    /*  65  */  {   KEY_F7          ,   KEY_F7          },
		    /*  66  */  {   KEY_F8          ,   KEY_F8          },
		    /*  67  */  {   KEY_F9          ,   KEY_F9          },
		    /*  68  */  {   KEY_F10         ,   KEY_F10         },
		    /*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
		    /*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },

		    /*  71  */  {   KEY_HOME        ,   '7'             },
		    /*  72  */  {   KEY_UP          ,   '8'             },
		    /*  73  */  {   KEY_PAGEUP      ,   '9'             },
		    /*  74  */  {   '-'             ,   '-'             },
		    /*  75  */  {   KEY_LEFT        ,   '4'             },
		    /*  76  */  {   KEY_CENTER      ,   '5'             },
		    /*  77  */  {   KEY_RIGHT       ,   '6'             },
		    /*  78  */  {   '+'             ,   '+'             },
		    /*  79  */  {   KEY_END         ,   '1'             },
		    /*  80  */  {   KEY_DOWN        ,   '2'             },
		    /*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
		    /*  82  */  {   KEY_INS         ,   '0'             },
		    /*  83  */  {   KEY_DEL         ,   '.'             },
		    /*  84  */  {   KEY_NONE        ,   KEY_NONE        },
		    /*  85  */  {   KEY_NONE        ,   KEY_NONE        },
		    /*  86  */  {   KEY_NONE        ,   KEY_NONE        },
		    /*  87  */  {   KEY_F11         ,   KEY_F11         },
		    /*  88  */  {   KEY_F12         ,   KEY_F12         }
};
BOOL kIsAlphabetScanCode(BYTE bScanCode){
	if(('a'<=gs_vstKeyMappingTable[bScanCode].bNormalCode)&&
			(gs_vstKeyMappingTable[bScanCode].bNormalCode<='z')){
		return TRUE;
	}
	return FALSE;
}
BOOL kIsNumberOrSymbolScanCode(BYTE bScanCode){
	if((2<=bScanCode)&&(bScanCode<=53)&&
			(kIsAlphabetScanCode(bScanCode)==FALSE)){
		return TRUE;
	}
	return FALSE;
}
BOOL kIsNumberPadScanCode(BYTE bScanCode){
	if((71<=bScanCode)&&(bScanCode<=83)){
		return TRUE;
	}
	return FALSE;
}
BOOL kIsUseCombinedCode( BOOL bScanCode )
{
    BYTE bDownScanCode;
    BOOL bUseCombinedKey;

    bDownScanCode = bScanCode & 0x7F;

    // ���ĺ� Ű��� Shift Ű�� Caps Lock�� ������ ����
    if( kIsAlphabetScanCode( bDownScanCode ) == TRUE )
    {
        // ���� Shift Ű�� Caps Lock Ű �߿� �ϳ��� ������������ ���յ� Ű�� �ǵ��� ��
        if( gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn )
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    // ���ڿ� ��ȣ Ű��� Shift Ű�� ������ ����
    else if( kIsNumberOrSymbolScanCode( bDownScanCode ) == TRUE )
    {
        // Shift Ű�� ������������ ���յ� Ű�� �ǵ��� ��
        if( gs_stKeyboardManager.bShiftDown == TRUE )
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    // ���� �е� Ű��� Num Lock Ű�� ������ ����
    // 0xE0�� �����ϸ� Ȯ�� Ű �ڵ�� ���� �е��� �ڵ尡 ��ġ�Ƿ�,
    // Ȯ�� Ű �ڵ尡 ���ŵ��� �ʾ��� ����ó�� ���յ� �ڵ� ���
    else if( ( kIsNumberPadScanCode( bDownScanCode ) == TRUE ) &&
             ( gs_stKeyboardManager.bExtendedCodeIn == FALSE ) )
    {
        // Num Lock Ű�� ������������, ���յ� Ű�� �ǵ��� ��
        if( gs_stKeyboardManager.bNumLockOn == TRUE )
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }

    return bUseCombinedKey;
}
/**
 *  ���� Ű�� ���¸� �����ϰ� LED ���µ� ����ȭ ��
 */

void UpdateCombinationKeyStatusAndLED( BYTE bScanCode )
{
    BOOL bDown;
    BYTE bDownScanCode;
    BOOL bLEDStatusChanged = FALSE;

    // ���� �Ǵ� ������ ����ó��, �ֻ��� ��Ʈ(��Ʈ 7)�� 1�̸� Ű�� ���������� �ǹ��ϰ�
    // 0�̸� ���������� �ǹ���
    if( bScanCode & 0x80 )
    {
        bDown = FALSE;
        bDownScanCode = bScanCode & 0x7F;
    }
    else
    {
        bDown = TRUE;
        bDownScanCode = bScanCode;
    }

    // ���� Ű �˻�
    // Shift Ű�� ��ĵ �ڵ�(42 or 54)�̸� Shift Ű�� ���� ����
    if( ( bDownScanCode == 42 ) || ( bDownScanCode == 54 ) )
    {
        gs_stKeyboardManager.bShiftDown = bDown;
    }
    // Caps Lock Ű�� ��ĵ �ڵ�(58)�̸� Caps Lock�� ���� �����ϰ� LED ���� ����
    else if( ( bDownScanCode == 58 ) && ( bDown == TRUE ) )
    {
        gs_stKeyboardManager.bCapsLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    // Num Lock Ű�� ��ĵ �ڵ�(69)�̸� Num Lock�� ���¸� �����ϰ� LED ���� ����
    else if( ( bDownScanCode == 69 ) && ( bDown == TRUE ) )
    {
        gs_stKeyboardManager.bNumLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    // Scroll Lock Ű�� ��ĵ �ڵ�(70)�̸� Scroll Lock�� ���¸� �����ϰ� LED ���� ����
    else if( ( bDownScanCode == 70 ) && ( bDown == TRUE ) )
    {
        gs_stKeyboardManager.bScrollLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }

    // LED ���°� �������� Ű����� Ŀ�ǵ带 �����Ͽ� LED�� ����
    if( bLEDStatusChanged == TRUE )
    {
        kChangeKeyboardLED( gs_stKeyboardManager.bCapsLockOn,
            gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn );
    }
}

/**
 *  ��ĵ �ڵ带 ASCII �ڵ�� ��ȯ
 */
BOOL kConvertScanCodeToASCIICode( BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags )
{
    BOOL bUseCombinedKey;

    // ������ Pause Ű�� ���ŵǾ��ٸ�, Pause�� ���� ��ĵ �ڵ带 ����
    if( gs_stKeyboardManager.iSkipCountForPause > 0 )
    {
        gs_stKeyboardManager.iSkipCountForPause--;
        return FALSE;
    }

    // Pause Ű�� Ư���� ó��
    if( bScanCode == 0xE1 )
    {
        *pbASCIICode = KEY_PAUSE;
        *pbFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return TRUE;
    }
    // Ȯ�� Ű �ڵ尡 ������ ��, ���� Ű ���� ������ �����Ƿ� �÷��� ������ �ϰ� ����
    else if( bScanCode == 0xE0 )
    {
        gs_stKeyboardManager.bExtendedCodeIn = TRUE;
        return FALSE;
    }

    // ���յ� Ű�� ��ȯ�ؾ� �ϴ°�?
    bUseCombinedKey = kIsUseCombinedCode( bScanCode );

    // Ű �� ����
    if( bUseCombinedKey == TRUE )
    {
        *pbASCIICode = gs_vstKeyMappingTable[ bScanCode & 0x7F ].bCombinedCode;
    }
    else
    {
        *pbASCIICode = gs_vstKeyMappingTable[ bScanCode & 0x7F ].bNormalCode;
    }

    // Ȯ�� Ű ���� ����
    if( gs_stKeyboardManager.bExtendedCodeIn == TRUE )
    {
        *pbFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = FALSE;
    }
    else
    {
        *pbFlags = 0;
    }

    // ������ �Ǵ� ������ ���� ����
    if( ( bScanCode & 0x80 ) == 0 )
    {
        *pbFlags |= KEY_FLAGS_DOWN;
    }

    // ���� Ű ���� �Ǵ� ������ ���¸� ����
    UpdateCombinationKeyStatusAndLED( bScanCode );
    return TRUE;
}
BOOL kInitializeKeyboard(void){
	kInitializeQueue(&gs_stKeyQueue, (void*)gs_vstKeyQueueBuffer, KEY_MAXQUEUECOUNT, sizeof(KEYDATA));
	return kActivateKeyboard();
}
BOOL kConvertScanCodeAndPutQueue(BYTE bScanCode){
	KEYDATA stData;
	BOOL bResult=FALSE;
	BOOL bPreviousInterrupt;
	stData.bScanCode = bScanCode;
	if(kConvertScanCodeToASCIICode(bScanCode, &(stData.bASCIICode), &(stData.bFlags))==TRUE){
		bPreviousInterrupt= kSetInterruptFlag(FALSE);// interrupt ó��
		bResult = kPutQueue(&gs_stKeyQueue, &stData);
		kSetInterruptFlag(bPreviousInterrupt);	// interrupt ó��
	}
	return bResult;
}
BOOL kGetKeyFromKeyQueue(KEYDATA* pstData){
	BOOL bResult;
	BOOL bPreviousInterrupt;
	if(kIsQueueEmpty(&gs_stKeyQueue)==TRUE)
		return FALSE;
	bPreviousInterrupt= kSetInterruptFlag(FALSE);// interrupt ó��
	bResult = kGetQueue(&gs_stKeyQueue, pstData);
	kSetInterruptFlag(bPreviousInterrupt);// interrupt ó��
	return bResult;
}
