#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "HFile/NativeLog.h"
#include "Dump_Dex.H"
#include "Dex_Util.H" 
#include "Dex_ClassDef.H"
/*
************************************************************
*			˵���������ݸ�ʽ
*1.DexClassDef	-> ����һ�������Ľṹ��
*	typedef struct DexClassDef {
		u4  classIdx;           // index into typeIds for this class 
		u4  accessFlags;
		u4  superclassIdx;      // index into typeIds for superclass 
		u4  interfacesOff;      // file offset to DexTypeList 
		u4  sourceFileIdx;      // index into stringIds for source file name 
		u4  annotationsOff;     // file offset to annotations_directory_item 
		u4  classDataOff;       // file offset to class_data_item 
		u4  staticValuesOff;    // file offset to DexEncodedArray 
	} DexClassDef;
*2.classDataOffָ���class_data_item����
*	i��class_data_item����ȫ������Leb128����
 		.byte 1			# Static field count: 0xc
		.byte 1			# Instance field count: 0x0
		.byte 1             # Direct method count: 0x1
		.byte 1             # Virtual method count: 0x0
	#List
		.byte 0                 # [0x0] static_field: (0x0) 
		.byte 0x19              # Access flags: 0x19
	#List
		.short 0x9E7            # [0x0] instance_field: (0x4e7) 
		.byte 0x12              # Access flags: 0x12
	#List
		.short 0x1D8E           # [0x0] direct_method: (0xe8e) '<init>'
		.byte 0x81, 0x80, 4     # Access flags: (0x10001) 'public constructor'
		.byte 0xE8, 0xB4, 0xBD, 0xF5, 0xF # Offset to the code: 0xfeaf5a68
	#List
		.short 0x1D8F           # [0x0] virtual_method: (0xe8f) 'a'
		.short 0x881            # Access flags: (0x401) 'public abstract'
		.byte 0x80, 0x80, 0x80, 0x80, 0 # Offset to the code: 0x0
	#
*3.����������ʱ������
************************************************************
*/
/*
************************************************************
*			�޸�Class Def ����
*
************************************************************
*/
void LOG_ClassDef(Link_ClassDef* inLink){
	if(inLink == NULL){
		DumpD("LOG_ClassDef@inLink is NULL");
		return ;
	}
	Link_ClassDef* mLink = inLink;
	while(mLink != NULL){
		DumpD("ClassDef off:0x%08x",mLink->info->classDataOff);
		if(mLink->item != NULL){
			DexMethod* mMethod= mLink->item->directMethods;
			while(mMethod != NULL){
				DumpD("directMethods Code off:0x%08x",mMethod->codeOff);
				mMethod = mMethod->next;
			}
			mMethod= mLink->item->virtualMethods;
			while(mMethod != NULL){
				DumpD("virtualMethods Code off:0x%08x",mMethod->codeOff);
				mMethod = mMethod->next;
			}
		}
		mLink = mLink->next;
	}	
}
/*
************************************************************
*				ClassDefItem_Add
*	ֱ���޸��ڴ��е�ƫ��
*
************************************************************
*/
void writeDexClassData(u1* itemAddr,DexClassData* item){
	//ֱ�ӹ���DexClassDataHeader
	itemAddr = readNEXTLeb128(itemAddr);
	itemAddr = readNEXTLeb128(itemAddr);
	itemAddr = readNEXTLeb128(itemAddr);
	itemAddr = readNEXTLeb128(itemAddr);
	//ֱ�ӹ���staticFieldsSize
	for(int m_i = 0;m_i < item->header.staticFieldsSize;m_i++){
		itemAddr = readNEXTLeb128(itemAddr);
		itemAddr = readNEXTLeb128(itemAddr);
	}
	//ֱ�ӹ���instanceFieldsSize
	for(int m_i = 0;m_i < item->header.instanceFieldsSize;m_i++){
		itemAddr = readNEXTLeb128(itemAddr);
		itemAddr = readNEXTLeb128(itemAddr);
	}
	//��ʼ����directMethodsSize
	DexMethod* mMethod= item->directMethods;
	for(int m_i = 0;m_i < item->header.directMethodsSize;m_i++){
		itemAddr = readNEXTLeb128(itemAddr);
		itemAddr = readNEXTLeb128(itemAddr);
		writeLeb128(itemAddr,mMethod->codeOff);
		DumpD("writeDexClassData itemAddr:0x%08x,off:0x%08x",itemAddr,readLeb128(itemAddr));

		itemAddr = readNEXTLeb128(itemAddr);
		mMethod = mMethod->next;
	}
	//��ʼ����virtualMethodsSize
	 mMethod= item->virtualMethods;
	for(int m_i = 0;m_i < item->header.virtualMethodsSize;m_i++){
		itemAddr = readNEXTLeb128(itemAddr);
		itemAddr = readNEXTLeb128(itemAddr);
		writeLeb128(itemAddr,mMethod->codeOff);
		DumpD("writeDexClassData itemAddr:0x%08x,off:0x%08x",itemAddr,readLeb128(itemAddr));
		itemAddr = readNEXTLeb128(itemAddr);
		mMethod = mMethod->next;
	}
}
/*
************************************************************
*
*
************************************************************
*/
void ClassDefItem_Add(Mod_Mem* inMem,Link_ClassDef* mLink,u4 inOff){
	//DumpD("Dest Addr:0x%08x,LoadMemAddr:0x%08x",Dest->Addr,Dest->LoadMemAddr);
	while(mLink != NULL){
		if(mLink->item != NULL){
			u1* ItemMem = inMem->getOffsetAddr(mLink->info->classDataOff);
			//getRepairMem(Dest,mLink->info->classDataOff);
			DexMethod* mMethod= mLink->item->directMethods;
			while(mMethod != NULL){
				if(mMethod->codeOff != 0){
					mMethod->codeOff += inOff;
				}
				mMethod = mMethod->next;
			}
			mMethod= mLink->item->virtualMethods;
			while(mMethod != NULL){
				if(mMethod->codeOff != 0){
					mMethod->codeOff += inOff;
				}
				mMethod = mMethod->next;
			}
			writeDexClassData(ItemMem,mLink->item);
		}
		mLink = mLink->next;
	}
}
/*
************************************************************
*����ClassDef Item����(��Ϊ����ǰ���Leb128,Ҫ�����ݰ���ʽ��ȡ)
*
*
************************************************************
*/
DexClassData* ClassDefItem_Parse(u1* item){
	DexClassData* mItem = (DexClassData*)malloc(sizeof(DexClassData)+1);
	memset(mItem,0,sizeof(DexClassData)+1);
	mItem->header.staticFieldsSize = readLeb128(item);
	item =  readNEXTLeb128(item);
	mItem->header.instanceFieldsSize = readLeb128(item);
		item =  readNEXTLeb128(item);
	mItem->header.directMethodsSize = readLeb128(item);
	item =  readNEXTLeb128(item);
	mItem->header.virtualMethodsSize = readLeb128(item); 
	item =  readNEXTLeb128(item);
	//staticFields
	DexField* mField = NULL;
	for(int m_i = 0;m_i < mItem->header.staticFieldsSize;m_i++){
		if(mItem->staticFields == NULL){
			mItem->staticFields = (DexField*)malloc(sizeof(DexField)+1);
			memset(mItem->staticFields,0,sizeof(DexField)+1);
			mField = mItem->staticFields;
		}else{
			mField->next =  (DexField*)malloc(sizeof(DexField)+1);
			memset(mField->next,0,sizeof(DexField)+1);
			mField = mField->next;
		}	 
		mField->fieldIdx = readLeb128(item);	
		item =  readNEXTLeb128(item);
		mField->accessFlags = readLeb128(item);
		item =  readNEXTLeb128(item);
	}
	//instanceFields
	for(int m_i = 0;m_i < mItem->header.instanceFieldsSize;m_i++){
		if(mItem->instanceFields == NULL){
			mItem->instanceFields = (DexField*)malloc(sizeof(DexField)+1);
			memset(mItem->instanceFields,0,sizeof(DexField)+1);
			mField = mItem->instanceFields;
		}else{
			mField->next =  (DexField*)malloc(sizeof(DexField)+1);
			memset(mField->next,0,sizeof(DexField)+1);
			mField = mField->next;
		}	 
		mField->fieldIdx = readLeb128(item);	
		item =  readNEXTLeb128(item);
		mField->accessFlags = readLeb128(item);
		item =  readNEXTLeb128(item);
	}
	//directMethods
	DexMethod* mMethod= NULL;
	for(int m_i = 0;m_i < mItem->header.directMethodsSize;m_i++){
		if(mItem->directMethods == NULL){
			mItem->directMethods = (DexMethod*)malloc(sizeof(DexMethod)+1);
			memset(mItem->directMethods,0,sizeof(DexMethod)+1);
			mMethod = mItem->directMethods;
		}else{
			mMethod->next =  (DexMethod*)malloc(sizeof(DexMethod)+1);
			memset(mMethod->next,0,sizeof(DexMethod)+1);
			mMethod = mMethod->next;
		}	 
		mMethod->methodIdx = readLeb128(item);	
		item =  readNEXTLeb128(item);
		mMethod->accessFlags = readLeb128(item);
		item =  readNEXTLeb128(item);
		mMethod->codeOff = readLeb128(item);
		item =  readNEXTLeb128(item);
	}
	//directMethods
	for(int m_i = 0;m_i < mItem->header.virtualMethodsSize;m_i++){
		if(mItem->virtualMethods == NULL){
			mItem->virtualMethods = (DexMethod*)malloc(sizeof(DexMethod)+1);
			memset(mItem->virtualMethods,0,sizeof(DexMethod)+1);
			mMethod = mItem->virtualMethods;
		}else{
			mMethod->next =  (DexMethod*)malloc(sizeof(DexMethod)+1);
			memset(mMethod->next,0,sizeof(DexMethod)+1);
			mMethod = mMethod->next;
		}	 
		mMethod->methodIdx = readLeb128(item);	
		item =  readNEXTLeb128(item);
		mMethod->accessFlags = readLeb128(item);
		item =  readNEXTLeb128(item);
		mMethod->codeOff = readLeb128(item);
		item =  readNEXTLeb128(item);
	}/**/
	return mItem;
}
/*
************************************************************
*����ClassDef����(��Ϊ��Ų��ǰ���Leb128,ֻҪ����ָ���OK)
************************************************************
*/
Link_ClassDef* ClassDef_Parse(DexFile* inDex){
 	if(inDex->pHeader->classDefsSize == 0){
		DumpD("ClassDef_Parse@����Dex classDefsSize Ϊ��!");
		return NULL;
 	}
	DexClassDef* ClassDef_Off = (DexClassDef*)((u8)inDex->pHeader->classDefsOff+(u8)inDex->pHeader);
	u1* ClassDef;
	Link_ClassDef* first = NULL;
	Link_ClassDef* ml_ClassDef=NULL;	
	for(int m_i = 0;m_i < inDex->pHeader->classDefsSize;m_i++){
		if(first == NULL){
			first = (Link_ClassDef*)malloc(sizeof(Link_ClassDef)+1);
			memset(first,0,sizeof(Link_ClassDef)+1);
			ml_ClassDef = first;
		}else{
			ml_ClassDef->next =  (Link_ClassDef*)malloc(sizeof(Link_ClassDef)+1);
			memset(ml_ClassDef->next,0,sizeof(Link_ClassDef)+1);
			ml_ClassDef = ml_ClassDef->next; 
		}	
		ml_ClassDef->info = ClassDef_Off;
		//����������item
		if(ml_ClassDef->info->classDataOff != 0){
			ml_ClassDef->item = ClassDefItem_Parse((u1*)((u8)ml_ClassDef->info->classDataOff+(u8)inDex->pHeader));
		}else{
			ml_ClassDef->item  = NULL;
		}
		//
		ClassDef_Off = (DexClassDef*)((u8)ClassDef_Off +  sizeof(DexClassDef));		
	}
	return first;
}
/*
************************************************************
*				Repair_ClassDef
*	�޸�ClassDef����������Լ�ClassDef����ָ������
*	1.�����ж�ClassDef�����Ƿ��ڷ�Χ��
*	2.�ж�ClassDef Item �����Ƿ����ڴ���
*
************************************************************
*/
unsigned int Repair_ClassDef(Mod_Mem* inMem,DexFile* inDex){
	//�ж�ClassDef�����Ƿ��Dump�ڴ���
	DumpD("Repair_ClassDef@�� �� У �� ClassDef!");
	if(inDex->pHeader->classDefsOff > inDex->pHeader->fileSize){
		DumpD("Repair_ClassDef@ClassDef �� �� �� �� ԭ ʼ �� Χ ֮ ��!");
		return 0;
	}/**/
	DumpD("Repair_ClassDef@�� �� �� ��ClassDef!");
	Link_ClassDef* mclassdef = ClassDef_Parse(inDex);
	//LOG_ClassDef(mclassdef);
	//�ж�ClassDef Item �����Ƿ����ڴ���,
	//����ȫ�� Off Code Source,�ж������Ƿ��ڷ�Χ��,��ȡ��С��ַ������ַ(0x0����)
	DumpD("Repair_ClassDef@�� �� У �� ClassDef Item!");	
	//������Сƫ�ƺ����ƫ��(0x0����)
	u4 ItemOffMin = 0xFFFFFFFF,ItemOffMax = 0;
	Link_ClassDef* mFind = mclassdef;
	while(mFind != NULL){
		if(mFind->item != NULL){
			DexMethod* mMethod= mFind->item->directMethods;
			while(mMethod != NULL){
				if(mMethod->codeOff != 0){
					if(mMethod->codeOff > ItemOffMax){
						ItemOffMax = mMethod->codeOff;
					}
					if(mMethod->codeOff < ItemOffMin){
						ItemOffMin = mMethod->codeOff;
					}
				}
					
				mMethod = mMethod->next;
			}
			mMethod= mFind->item->virtualMethods;
			while(mMethod != NULL){
				if(mMethod->codeOff != 0){
					if(mMethod->codeOff > ItemOffMax){
						ItemOffMax = mMethod->codeOff;
					}
					if(mMethod->codeOff < ItemOffMin){
						ItemOffMin = mMethod->codeOff;
					}
				}
				mMethod = mMethod->next;
			}
		}
		mFind = mFind->next;
	}
	DumpD("ItemOffMin:0x%08x,ItemOffMax:0x%08x",ItemOffMin,ItemOffMax);		
	//��������	
	u4 FileOff = inMem->getMemLength();
	str_ModMem* mMem = inMem->newMem(getDumpLen(ItemOffMax-ItemOffMin)+0x1000);
	mMem->LoadBase = (u1*)((ItemOffMin+(u8)(inDex->pHeader))&0xFFFFE000);
	memcpy(mMem->Addr,mMem->LoadBase,mMem->Length);
	//�޸�Def �ļ�ƫ��	
	u4 DataOff = (u8)inDex->pHeader- (u8)mMem->LoadBase + FileOff;
	DumpD("FileOff:0x%08x,DataOff:0x%08x",FileOff,DataOff);	
	ClassDefItem_Add(inMem,mclassdef,DataOff);
	//
	return ItemOffMin+DataOff;
	
}
