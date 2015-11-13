#ifndef __Module_Mem_H_
#define __Module_Mem_H_
#include "stdio.h"
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
//__________________________________________________________
typedef struct str_ModMem{
	unsigned char*	Addr;
	unsigned char*	LoadBase;
	unsigned int	Length;
	unsigned int	Offset;
	unsigned int	Base;
	struct str_ModMem* next;
}str_ModMem;
//__________________________________________________________
class Mod_Mem{
public:
str_ModMem* ModMem = NULL;
/*
*	��ȡģ������ڴ��С
*/
	 unsigned int getMemLength()
	 {
		unsigned int mMemLength = 0;
		str_ModMem* mMod = ModMem;
		while(mMod != NULL){
			mMemLength += mMod->Length;
			mMod = mMod->next;
		}
		return mMemLength;
	}
/*
*��ȡ���һ���ڴ�ģ��
*/
	str_ModMem* Last()
	{
		str_ModMem* mMod = ModMem;
		if(mMod == NULL)return NULL;

		while(mMod->next != NULL){
			mMod = mMod->next;
		}
		return mMod;
	}
/*
*	���һ�����ڴ浽����ģ����
*/	
	str_ModMem* newMem(size_t in_Length)
	{	
		str_ModMem* mMem = Last();
		if(mMem == NULL){//û�ڴ�ģ��
			ModMem = (str_ModMem*)malloc(sizeof(str_ModMem)+1);
			memset(ModMem,0,sizeof(str_ModMem)+1);
			ModMem->Addr = (unsigned char*)malloc(in_Length);
			memset(ModMem->Addr,0,in_Length);
			ModMem->Length = in_Length;
			return ModMem;
		} 
		mMem->next = (str_ModMem*)malloc(sizeof(str_ModMem)+1);
		mMem = mMem->next;
		memset(mMem,0,sizeof(str_ModMem)+1);
		mMem->Addr = (unsigned char*)malloc(in_Length);
		memset(mMem->Addr,0,in_Length);
		mMem->Length = in_Length;
		return mMem;
	}
/*
*����ƫ�ƻ�ȡ�����ַ
*/
	unsigned char* getOffsetAddr(size_t in_Off){
		str_ModMem* mMod = ModMem;
		unsigned int mMemLength = 0;
		while(mMod != NULL){
			if(in_Off < (mMemLength + mMod->Length)){
				return mMod->Addr+in_Off-mMemLength;				
			}
			mMemLength += mMod->Length;
			mMod = mMod->next;
		}
		return NULL;
	}
/*
*����ƫ�ƻ�ȡ�����ַ
*/
	unsigned int* getOffsetAddr_INT(size_t in_Off){
		str_ModMem* mMod = ModMem;
		unsigned int mMemLength = 0;
		while(mMod != NULL){
			if(in_Off < (mMemLength + mMod->Length)){
				return (unsigned int*)(mMod->Addr+in_Off-mMemLength);				
			}
			mMemLength += mMod->Length;
			mMod = mMod->next;
		}
		return NULL;
	}
/*
*��ȡ����
*/
	unsigned int getOffsetData_INT(size_t in_Off){
		str_ModMem* mMod = ModMem;
		unsigned int mMemLength = 0;
		while(mMod != NULL){
			if(in_Off < (mMemLength + mMod->Length)){
				unsigned int* m_data;
				m_data = (unsigned int*)((unsigned int)mMod->Addr+in_Off-mMemLength);
				return *m_data;				
			}
			mMemLength += mMod->Length;
			mMod = mMod->next;
		}
		return NULL;
	}
/*
*������ļ�
*/
	void SaveFile(char* inFile){
		FILE* mfd=fopen(inFile,"wb");
		//�ж��ļ��Ƿ�򿪳ɹ�
		if(!mfd){
			return;
		}
		str_ModMem* mMod = ModMem;
		while(mMod != NULL){
			fwrite(mMod->Addr,1,mMod->Length,mfd);
			mMod = mMod->next;
		}
		//дԭʼ����
		fclose(mfd);
	}
};
//
#endif