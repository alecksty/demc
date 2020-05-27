//=====================================================================================
//
//	模块:	反汇编EM78P153S代码!
//
//	版本:	1.00
//
//	日期:	2007-07-10
//
//	作者:	施探宇
//
//	说明:	demc -n rom.bin src.asm
//
//=====================================================================================
#include "stdafx.h"

//	保存标号个数
#define MAX_LABELS			256

//	代码地址上限
#define MAX_ADDRESS			0x400

//	最长代码字节数
#define CODE_MAX_BYTES		4

//	普通代码长度
#define CODE_BYTES			2

//	代码位数
#define CODE_BITS			(CODE_BYTES * 8)

//	长整型,所有指令都要改长,这样改成别语言就方便;
typedef unsigned long	UINT64;
typedef unsigned int	UINT32;
typedef unsigned short	UINT16;
typedef unsigned char	UINT8;
typedef unsigned char	BYTE;

//-----------------------------------------------------------------------------------
//	列表
//-----------------------------------------------------------------------------------
typedef struct tagDASM
{
	//	代码值
	UINT64	CodeData;
	//	代码位
	UINT64	CodeMask;
	//	寄存器位
	UINT64	RegsMask;
	//	比特位
	UINT64	BitsMask;
	//	数据位
	UINT64	DataMask;
	//	输出格式
	_TCHAR *pCodeFmt;
}DASM_TBL;

//-----------------------------------------------------------------------------------
//	代码转换有优先顺序:格式为 printf("CODE %S,%S,%S",REG,BIT,DAT)
//-----------------------------------------------------------------------------------
DASM_TBL tblCode[]=
{
	//  CODE	CO-MASK RG-MASK BI-MASK DA-MASK FORMAT
	{	0x0000,	0xffff,	0x0000,	0x0000,	0x0000,	_T("NOP")},
	{	0x0001,	0xffff,	0x0000,	0x0000,	0x0000,	_T("DAA")},
	{	0x0002,	0xffff,	0x0000,	0x0000,	0x0000,	_T("CONTW")},
	{	0x0003,	0xffff,	0x0000,	0x0000,	0x0000,	_T("SLEP")},
	{	0x0004,	0xffff,	0x0000,	0x0000,	0x0000,	_T("WDTC")},
	{	0x0000,	0xfff0,	0x000f,	0x0000,	0x0000,	_T("IOW   %s")},

	{	0x0010,	0xffff,	0x0000,	0x0000,	0x0000,	_T("ENI")},
	{	0x0011,	0xffff,	0x0000,	0x0000,	0x0000,	_T("DISI")},
	{	0x0012,	0xffff,	0x0000,	0x0000,	0x0000,	_T("RET")},
	{	0x0013,	0xffff,	0x0000,	0x0000,	0x0000,	_T("RETI")},
	{	0x0014,	0xffff,	0x0000,	0x0000,	0x0000,	_T("CONTR")},
	{	0x0010,	0xfff0,	0x000f,	0x0000,	0x0000,	_T("IOR   %s")},
	{	0x0040,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("MOV   %s,A")},
	{	0x0080,	0xffff,	0x0000,	0x0000,	0x0000,	_T("CLRA")},
	{	0x00C0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("CLR   %s")},

	{	0x0100,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("SUB   A,%s")},
	{	0x0140,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("SUB   %s,A")},
	{	0x0180,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("DECA  %s")},
	{	0x01C0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("DEC   %s")},

	{	0x0200,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("OR    A,%s")},
	{	0x0240,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("OR    %s,A")},
	{	0x0280,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("AND   A,%s")},
	{	0x02c0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("AND   %s,A")},

	{	0x0300,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("XOR   A,%s")},
	{	0x0340,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("XOR   %s,A")},
	{	0x0380,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("ADD   A,%s")},
	{	0x03c0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("ADD   %s,A")},

	{	0x0400,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("MOV   A,%s")},
	{	0x0440,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("MOV   %s,%s")},
	{	0x0480,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("COMA  %s")},
	{	0x04c0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("COM   %s")},

	{	0x0500,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("INCA")},
	{	0x0540,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("INC   %s")},
	{	0x0580,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("DJZA  %s")},
	{	0x05C0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("DJZ   %s")},

	{	0x0600,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("RRCA  %s")},
	{	0x0640,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("RRC   %s")},
	{	0x0680,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("RLCA  %s")},
	{	0x06c0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("RLC   %s")},

	{	0x0700,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("SWAPA %s")},
	{	0x0740,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("SWAP  %s")},
	{	0x0780,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("JZA   %s")},
	{	0x07c0,	0xffc0,	0x003f,	0x0000,	0x0000,	_T("JZ    %s")},

	{	0x0800,	0xfe00,	0x003f,	0x01C0,	0x0000,	_T("BC    %s,%s")},
	{	0x0A00,	0xfe00,	0x003f,	0x01C0,	0x0000,	_T("BS    %s,%s")},
	{	0x0C00,	0xfe00,	0x003f,	0x01C0,	0x0000,	_T("JBC   %s,%s")},
	{	0x0E00,	0xfe00,	0x003f,	0x01C0,	0x0000,	_T("JBS   %s,%s")},

	{	0x1000,	0xfc00,	0x0000,	0x0000,	0x03ff,	_T("CALL  %s")},
	{	0x1400,	0xfc00,	0x0000,	0x0000,	0x03ff,	_T("JMP   %s")},

	{	0x1800,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("MOV   A,%s")},
	{	0x1900,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("OR    A,%s")},
	{	0x1A00,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("AND   A,%s")},
	{	0x1B00,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("XOR   A,%s")},
	{	0x1C00,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("RETL  %s")},
	{	0x1D00,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("SUB   A,%s")},
	{	0x1E00,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("INT")},
	{	0x1F00,	0xff00,	0x0000,	0x0000,	0x00ff,	_T("ADD   A,%s")},
};

//-----------------------------------------------------------------------------------
//	寄存器名称
//-----------------------------------------------------------------------------------
_TCHAR *arrRegs[]=
{
/*0x00*/	_T("R0"),
/*0x01*/	_T("TCC"),
/*0x02*/	_T("PC"),
/*0x03*/	_T("STATUS"),
/*0x04*/	_T("RSR"),
/*0x05*/	_T("P5"),
/*0x06*/	_T("P6"),
/*0x07*/	_T("R_7"),
/*0x08*/	_T("R_8"),
/*0x09*/	_T("R_9"),
/*0x0A*/	_T("R_A"),
/*0x0B*/	_T("IOCB"),
/*0x0C*/	_T("IOCC"),
/*0x0D*/	_T("IOCD"),
/*0x0E*/	_T("WTD"),
/*0x0F*/	_T("RF"),
};

//-----------------------------------------------------------------------------------
//	状态位
//-----------------------------------------------------------------------------------
_TCHAR *arrStatus[]=
{
/*0x00*/	_T("C"),
/*0x01*/	_T("DC"),
/*0x02*/	_T("Z"),
/*0x03*/	_T("P"),
/*0x04*/	_T("T"),
/*0x05*/	_T("GP0"),
/*0x06*/	_T("GP1"),
/*0x07*/	_T("RST"),
};

//-----------------------------------------------------------------------------------
//	默认文件名称
//-----------------------------------------------------------------------------------
_TCHAR *pStart = _T("");
_TCHAR *pBinFile = _T("rom.bin");
_TCHAR *pSrcFile = _T("src.asm");
_TCHAR buff[]=_T("0x0000");

//	文件指针
FILE *fpBin;
FILE *fpSrc;

//	默认起始地址:
int iCodeStart = 0;

//	参数序号
int idArgv = 1;

//	指令缓冲区
BYTE pCodeBuff[CODE_MAX_BYTES];

//	存储标号和函数地址
UINT64 useLabels[MAX_LABELS];
UINT64 posLabels = 0;

//	显示帮助
void ShowHelp();

//	状态名
_TCHAR *GetStatuName(UINT64 nBits);

//	寄存器名
_TCHAR *GetRegname(UINT64 nReg);

//	登记标号
int Preparse(UINT64 lCode);

//	代码分析
int ParseCode(UINT64 lLine,UINT64 lCode,FILE *fpOut);

//	查找已登记标号
int FindLabel(UINT64 lLine);

//	文件分析
int ParseFile(int iStart, FILE *pIn,FILE *pOut);

//=====================================================================================
//	开始函数
//=====================================================================================
int main(int argc, _TCHAR* argv[])
{
	//	判断参数
	if(argc < 2 || argc > 4)
	{
		ShowHelp();
#ifdef _DEBUG
		getchar();
#endif
		return -1;
	}

	//	默认指向第一个参数
	idArgv = 1;
	pStart = argv[idArgv];

	//	是否有地址参数?
	if(pStart[0] == '-')
	{
		iCodeStart = atoi(&pStart[1]);
		idArgv ++;
	}

	if(argc > idArgv )
	{
		pBinFile = argv[idArgv ];
	}

	if(argc > idArgv + 1)
	{
		pSrcFile = argv[idArgv +1];
	}
	
	fpBin = fopen(pBinFile,"rb");
	if(!fpBin)
	{
		printf("Can't open bin file: %s!\n",pBinFile);
#ifdef _DEBUG
		getchar();
#endif
		return -1;
	}

	fpSrc = fopen(pSrcFile,"w+a");
	if(!fpSrc)
	{
		fclose(fpBin);
		printf("Can open src file: %s!\n",pSrcFile);
#ifdef _DEBUG
		getchar();
#endif
		return -1;
	}

	//	显示抬头
	printf(_T(" EM78P153S DISASM TOOLS(C) Version 1.00\n"));
	printf(_T("---------------------------------------------------------------\n"));
	printf(_T(" Code start:  %x.\n"),iCodeStart);
	printf(_T(" Input file:  %s.\n"),pBinFile);
	printf(_T(" Output file: %s.\n"),pSrcFile);
	printf(_T("---------------------------------------------------------------\n"));
	printf(_T(" QQ:190376601,TEL:13751152175              --Aleck.Shi\n"));

	//	分析文件
	ParseFile(iCodeStart,fpBin,fpSrc);

	fclose(fpBin);
	fclose(fpSrc);

	//	处理完毕!
	printf(_T(" Processed done!\n"));

#ifdef _DEBUG
	getchar();
#endif
	return 0;
}

//=====================================================================================
//	显示帮助
//=====================================================================================
void ShowHelp()
{
	printf(_T(" EM78P153 DISASM TOOLS(C) Version:1.00\n"));
	printf(_T("---------------------------------------------------------------\n"));
	printf(_T(" usage:dasm153 [-code address] <bin file> [src file]\n"));
	printf(_T("   exp:dasm153 rom.bin\n"));
	printf(_T("       dasm153 -2 rom.bin\n"));
	printf(_T("       dasm153 -10 rom.bin src.asm\n"));
	printf(_T("---------------------------------------------------------------\n"));
	printf(_T(" QQ:190376601,TEL:13751152175                     ----Aleck.Shi\n"));
}

//=====================================================================================
//	文件分析
//=====================================================================================
int ParseFile(int iStart, FILE *fpIn,FILE *fpOut)
{
	UINT64 lCodePos = 0;
	UINT64 lCode = 0;

	fprintf(fpOut,_T(";//=====================================================================================\n"));
	fprintf(fpOut,_T(";//\n"));
	fprintf(fpOut,_T(";//	模块:	<模块名称>\n"));
	fprintf(fpOut,_T(";//\n"));
	fprintf(fpOut,_T(";//	版本:	<版本号>\n"));
	fprintf(fpOut,_T(";//\n"));
	fprintf(fpOut,_T(";//	日期:	<日期>\n"));
	fprintf(fpOut,_T(";//\n"));
	fprintf(fpOut,_T(";//	作者:	<作者>\n"));
	fprintf(fpOut,_T(";//\n"));
	fprintf(fpOut,_T(";//	说明:	<模块说明>\n"));
	fprintf(fpOut,_T(";//\n"));
	fprintf(fpOut,_T(";//=====================================================================================\n"));

	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));
	fprintf(fpOut,_T(";// 通用寄存器:\n"));
	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));
	fprintf(fpOut,_T("R0     == 0X00\n"));
	fprintf(fpOut,_T("TCC    == 0x01\n"));
	fprintf(fpOut,_T("PC     == 0x02\n"));
	fprintf(fpOut,_T("STATUS == 0x03\n"));
	fprintf(fpOut,_T("RSR    == 0x04\n"));
	fprintf(fpOut,_T("P5     == 0x05\n"));
	fprintf(fpOut,_T("P6     == 0x06\n"));
	fprintf(fpOut,_T("IOCB   == 0x0B\n"));
	fprintf(fpOut,_T("IOCC   == 0x0C\n"));
	fprintf(fpOut,_T("IOCD   == 0x0D\n"));
	fprintf(fpOut,_T("WTD    == 0x0E\n"));
	fprintf(fpOut,_T("RF     == 0x0F\n"));

	fprintf(fpOut,_T("\n"));
	fprintf(fpOut,_T("C      == 0x00\n"));
	fprintf(fpOut,_T("DC     == 0x01\n"));
	fprintf(fpOut,_T("Z      == 0x02\n"));
	fprintf(fpOut,_T("P      == 0x03\n"));
	fprintf(fpOut,_T("T      == 0x04\n"));
	fprintf(fpOut,_T("GP0    == 0x05\n"));
	fprintf(fpOut,_T("GP1    == 0x06\n"));
	fprintf(fpOut,_T("RST    == 0x07\n"));
	fprintf(fpOut,_T("\n"));

	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));
	fprintf(fpOut,_T(";// 常量定义:\n"));
	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));

	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));
	fprintf(fpOut,_T(";// 变量定义:\n"));
	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));

	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));
	fprintf(fpOut,_T(";// 代码开始:\n"));
	fprintf(fpOut,_T(";//-------------------------------------------------------------------------------------\n"));
	fprintf(fpOut,_T("\t\tORG 0x00\n"));

	//	查找起始地址
	fseek(fpIn,iStart,SEEK_SET);

	//	登记标号!
	posLabels = 0;
	lCodePos = 0;
	while(!feof(fpIn))
	{
		fread(pCodeBuff,1,CODE_BYTES,fpIn);

		//	转换代码
		lCode = pCodeBuff[0];
		lCode |= pCodeBuff[1] << 8;

		//	登记标号
		Preparse(lCode);

		lCodePos ++;
		//	地址上限检查
		if(lCodePos >= MAX_ADDRESS)
			break;
	}

	//	查找起始地址
	fseek(fpIn,iStart,SEEK_SET);
	lCodePos = 0;
	while(!feof(fpIn))
	{
		//	读取数据
		fread(pCodeBuff,1,CODE_BYTES,fpIn);

		//	转换代码
		lCode = pCodeBuff[0];
		lCode |= pCodeBuff[1] << 8;

		//	代码生成
		ParseCode(lCodePos,lCode,fpOut);
		lCodePos ++;

		//	地址上限检查
		if(lCodePos >= MAX_ADDRESS)
			break;
	}

	fprintf(fpOut,_T(";//=====================================================================================\n"));
	fprintf(fpOut,_T(";// 文件结束. <END OF FILE> \n"));
	fprintf(fpOut,_T(";//=====================================================================================\n"));
	return 0;
}

//=====================================================================================
//	行号,标号分析
//=====================================================================================
int Preparse(UINT64 lCode)
{
	UINT64 i = 0;	
	UINT64 lCount =  sizeof(tblCode) / sizeof(DASM_TBL);
	UINT64 lLine = 0;
	
	//	初始化
	for( i = 0; i < lCount; i++ )
	{
		//	寻找跳转: JMP k,CALL k
		if((tblCode[i].DataMask == 0x03ff) && (posLabels < MAX_LABELS))
		{
			//	匹配指令集
			if((tblCode[i].CodeMask & lCode) == tblCode[i].CodeData)
			{
				//	行号
				lLine = lCode & tblCode[i].DataMask;
				//	登记标号
				if(!FindLabel(lLine))
				{
					useLabels[posLabels] = lLine;
#ifdef _DEBUG
					printf("useLabels[%d] = %04x\n",posLabels,lLine);
#endif
					posLabels++;
					return 1;
				}
			}
		}
	}
	return 0;
}

//=====================================================================================
//	是否登记行号
//=====================================================================================
int FindLabel(UINT64 lLine)
{
	UINT64 i = 0;

	for( i = 0; i < posLabels; i++ )
	{
		//	登记标号
		if(useLabels[i] == lLine)
			return 1;
	}
	return 0;
}

//=====================================================================================
//	取得寄存器名称!
//=====================================================================================
_TCHAR *GetRegname(UINT64 nReg)
{
	if(nReg < 16)
	{
		return arrRegs[nReg];
	}
	else
	{
		sprintf(buff,_T("0x%02x"),nReg);
	}
	return buff;
}

//=====================================================================================
//	取得状态位名称
//=====================================================================================
_TCHAR *GetStatuName(UINT64 nBits)
{
	if(nBits < 8)
	{
		return arrStatus[nBits];
	}
	return NULL;
}

//=====================================================================================
//	代码分析
//=====================================================================================
int ParseCode(UINT64 lLine,UINT64 lCode,FILE *fpOut)
{
	UINT64 i = 0;	
	UINT64 lCount =  sizeof(tblCode) / sizeof(DASM_TBL);

	_TCHAR buff1[64];
	_TCHAR buff2[64];
	
	for( i = 0; i < lCount; i++ )
	{
		//	匹配指令集
		if((tblCode[i].CodeMask & lCode) == tblCode[i].CodeData)
		{
			//	有需要的才标号,需要查表
			if(FindLabel(lLine))
			{
				fprintf(fpOut,_T("lab_%04x:\n"),lLine);
			}

			//	开始空点位置!
			fprintf(fpOut,_T("\t\t"));

			// 类似:BC r,b
			if(tblCode[i].RegsMask && tblCode[i].BitsMask)
			{
				//sprintf(buff1,_T("0x%02x"),tblCode[i].RegsMask & lCode);
				strcpy(buff1,GetRegname(tblCode[i].RegsMask & lCode));

				//sprintf(buff2,_T("%d"),(tblCode[i].BitsMask & lCode) >> 6);
				//	状态寄存器
				if((tblCode[i].RegsMask & lCode) == 3)
				{
					strcpy(buff2,GetStatuName((tblCode[i].BitsMask & lCode) >> 6));
				}
				else
				{
					sprintf(buff2,_T("%d"),(tblCode[i].BitsMask & lCode) >> 6);
				}
				fprintf(fpOut,(const char*)tblCode[i].pCodeFmt,buff1,buff2);
			}
			// 类似:INC r
			else if(tblCode[i].RegsMask)
			{
				//sprintf(buff1,_T("0x%02x"),tblCode[i].RegsMask & lCode);
				strcpy(buff1,GetRegname(tblCode[i].RegsMask & lCode));
				fprintf(fpOut,(const char*)tblCode[i].pCodeFmt,buff1);
			}
			//	类似 JMP k,CALL k
			else if(tblCode[i].DataMask)
			{
				//	函数位置
				if(tblCode[i].DataMask == 0x03ff)
				{
					sprintf(buff1,_T("lab_%04x"),tblCode[i].DataMask & lCode);
				}
				else
				{
					//	立即数处理
					sprintf(buff1,_T("@0x%02x"),tblCode[i].DataMask & lCode);
				}
				fprintf(fpOut,(const char*)tblCode[i].pCodeFmt,buff1);
			}
			//	类似 WDTC,CLRA
			else if(!tblCode[i].DataMask && !tblCode[i].BitsMask && !tblCode[i].RegsMask)
			{
				fprintf(fpOut,(const char*)tblCode[i].pCodeFmt);
			}
			//	错误的匹配!
			else
			{
				fprintf(fpOut,_T(";Error Code:%04x\n"), lCode);
				printf(_T("Error Code:%04x\n"),lCode);
			}

			//	换行
			fprintf(fpOut,_T("\n"));
			break;
		}
	}
	
	//	错误的指令
	if(i >= lCount)
	{
		fprintf(fpOut,_T(";Unknow Code:%04x\n"),lCode);
		printf(_T("Unknow Code:%04x\n"),lCode);
	}

	return 0;
}
