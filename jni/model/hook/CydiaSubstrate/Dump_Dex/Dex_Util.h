#ifndef __DEX_UTIL_H_
#define __DEX_UTIL_H_
//__________________________________________________________
//��ȡһ��Leb128����
extern u4 readLeb128(u1* inAddr);
//��ȡ��һ��Leb128���ݵ�ַ
extern u1* readNEXTLeb128(u1* inAddr);
//��ȡLeb128���ݿ��
extern u4 readLimitLeb128(u1* inAddr);
//
extern int writeAddLeb128(u1* inAddr,u4 Add_Off);
extern int writeLeb128(u1* inAddr,u4 inData);
//������4K��������4K
extern u4 getDumpLen(u4 inLen);
//
extern u4 getFileLength(RepairMem* inRep_Start);
#endif