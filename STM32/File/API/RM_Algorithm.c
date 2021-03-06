/*******************************************************************
��Ȩ������ RoboMasters(HITCRT_iHiter ս��)
�ļ����� RM_Algorithm.h
����޸����ڣ� 2016/1/25
�汾�� 1.0
--------------------------------------------------------------------
ģ����������ģ�鶨���˳��õ��㷨��
--------------------------------------------------------------------
�޸ļ�¼��
����                              ʱ��         �汾   ˵��
��������ع�ˡ��ű���������    2016/1/25     1.0     �������ļ�
********************************************************************/
#include "main.h"
#include <math.h>
#include <float.h>

/**************************************************************************************************/
/*********************************��ѧ��������㷨****************************************************/
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
�������ƣ�Clip()
�������ܣ�����������ȥ���������ֵ����Сֵ֮���ֵ����֮��������Сֵ
--------------------------------------------------------------------------------------------------*/
FP32 Clip(FP32 fpValue, SINT32 siMin, SINT32 siMax)
{
	if(fpValue < siMin)
	{
		return (FP32)(siMin);
	}
	else if(fpValue > siMax)
	{
		return (FP32)(siMax);
	}
	else 
	{
		return fpValue;
	}
}
/*--------------------------------------------------------------------------------------------------
�������ƣ�Round()
�������ܣ����������������룬����32λ������
--------------------------------------------------------------------------------------------------*/
SINT32 Round(FP32 fpValue)
{   
    if (fpValue >= 0)
    {
    	return (SINT32)(fpValue + 0.5f);
    }
    else 
    {
    	return (SINT32)(fpValue - 0.5f);
    }
}

/*--------------------------------------------------------------------------------------------------
�������ƣ�IsNum()
�������ܣ��ж�һ���������Ƿ�������
--------------------------------------------------------------------------------------------------*/
bool IsNum(DB64 x)
{
	if (x == x)
		return TRUE;
    else
		return FALSE;
}

/*--------------------------------------------------------------------------------------------------
�������ƣ�IsFiniteNum()
�������ܣ��ж�һ���������Ƿ������޵ļ��ж�һ��float�Ȳ���NANҲ����infinite
--------------------------------------------------------------------------------------------------*/
bool IsFiniteNum(DB64 x)
{
	if (x <= DBL_MAX && x >= -DBL_MAX)
		return TRUE;
	else
    	return FALSE;
}


/**************************************************************************************************/
/*********************************PID����㷨****************************************************/
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
�������ƣ�CalPID()  
�������ܣ�����PID��������������ͨ��PID
          ������PID�㷨�ļ��㹫ʽ��detU(k)=U(k)-U(k-1)=Kp(detE(k)+IE(k)+fpD*det(E(k))+det(E(k)))
          ����������ʽPID:U=U+KP*��E+KI*E+KD*[E+E(n-2)-2*E(n-1)]
--------------------------------------------------------------------------------------------------*/
void CalPID(volatile ST_PID *pstPid)    
{   
    UCHAR8 ucK;
   
	ucK=1;
	pstPid->U += pstPid->KP * (pstPid->E - pstPid->PreE) + ucK * pstPid->KI * pstPid->E + pstPid->KD * (pstPid->E - 2 * pstPid->PreE + pstPid->PrePreE);
	pstPid->PrePreE = pstPid->PreE;
	pstPid->PreE = pstPid->E;
    
    //�ж��Ƿ������֣�����������֣���ֵΪ��,�޸���ң�����ȿ�����ȶ���bug
    if (IsFiniteNum(pstPid->U) != TRUE)
        pstPid->U = 0;

    pstPid->U = Clip(pstPid->U,pstPid->UMin,pstPid->UMax);
}

void CalPID_Su(volatile ST_PID *pstPid)    
{   
    UCHAR8 ucK;
   
	ucK=1;
	pstPid->U = pstPid->KP * pstPid->E + ucK * pstPid->KI * (pstPid->E - pstPid->PreE);
	pstPid->PrePreE = pstPid->PreE;
	pstPid->PreE = pstPid->E;

	if (IsFiniteNum(pstPid->U) != TRUE)
    pstPid->U = 0;

    pstPid->U = Clip(pstPid->U,pstPid->UMin,pstPid->UMax);
}
/*--------------------------------------------------------------------------------------------------
�������ƣ�CalPID_IS() :Calculate PID Integral Separation
�������ܣ����ַ���ʽPID
           ������PID�㷨�ļ��㹫ʽ��detU(k)=U(k)-U(k-1)=Kp(detE(k)+IE(k)+D*det(E(k))+det(E(k)))
           ���ַ���ʽPID:U=U+KP*��E+ucK*KI*E+KD*[E+E(n-2)-2*E(n-1)]
           ���ַ���ʽPID���ɷ�ֹ���󳬵���
--------------------------------------------------------------------------------------------------*/
void CalPID_IS(volatile ST_PID *pstPid)	
{   
    UCHAR8  ucK;
    
	if (fabs(pstPid->E) > pstPid->ELimit)
	{
		ucK=0; 
	}
	else 
	{
		ucK=1;
	}
	pstPid->U += pstPid->KP * (pstPid->E - pstPid->PreE) 
                 + ucK * pstPid->KI * pstPid->E 
                 + pstPid->KD * (pstPid->E - 2 * pstPid->PreE + pstPid->PrePreE);

	pstPid->PrePreE = pstPid->PreE;
	pstPid->PreE=pstPid->E;
}

/*--------------------------------------------------------------------------------------------------
�������ƣ�CalPID_WTCOL()  :Weaken The Case Of Limited
�������ܣ�����PID������������ʽPID
          ������PID�㷨�ļ��㹫ʽ��detU(k)=U(k)-U(k-1)=Kp(detE(k)+IE(k)+fpD*det(E(k))+det(E(k)))
          ��������ʽPID:U=U+KP*��E+ucK*KI*E+KD*[E+E(n-2)-2*E(n-1)]
          ��������ʽPID���ɷ�ֹPID���������ڱ���,������
--------------------------------------------------------------------------------------------------*/
void CalPID_WTCOL(volatile ST_PID *pstPid)  
{   
    UCHAR8 ucK;
	 
	if (((pstPid->U > pstPid->ULimit) && (pstPid->E > 0)) || ((pstPid->U < -pstPid->ULimit) && (pstPid->E < 0)))//��ͣ����ܼ�С�������ֵ��ǰ���²Ž��л���
	{
	    ucK = 0;
	}
	else
	{ 
		ucK = 1;
	}

	if(abs(pstPid->E) <= pstPid->EDead)
	{
		pstPid->E = 0;
	}
	pstPid->U += pstPid->KP * (pstPid->E - pstPid->PreE) 
                 + ucK * pstPid->KI * pstPid->E 
                 + pstPid->KD * (pstPid->E - 2 * pstPid->PreE + pstPid->PrePreE);
	
	pstPid->PrePreE = pstPid->PreE;
	pstPid->PreE = pstPid->E;
}

/*--------------------------------------------------------------------------------------------------
�������ƣ�CalPID_WTCOLIS() 
�������ܣ�����PID��
          ������PID�㷨�ļ��㹫ʽ��detU(k)=U(k)-U(k-1)=Kp(detE(k)+IE(k)+fpD*det(E(k))+det(E(k)))
          ���ַ���+��������ʽPID:U=U+KP*��E+ucK1*ucK2*KI*E+KD*[E+E(n-2)-2*E(n-1)]
          ���ַ���+��������ʽPID��϶����ŵ�
--------------------------------------------------------------------------------------------------*/
void CalPID_WTCOLIS(volatile ST_PID *pstPid)  
{   
    UCHAR8 ucK1, ucK2;
    
    //��������
	if (((pstPid->U > pstPid->ULimit) && (pstPid->E > 0)) 
		|| ((pstPid->U < -pstPid->ULimit) && (pstPid->E < 0)))//������һ�ε�pstPid���������ж��Ƿ�Ҫ�л��ֻ���
	{
	    ucK1=0;
	}
	else
	{ 
		ucK1=1;
	}
	
	//���ַ���
	if (fabs(pstPid->E) > pstPid->ELimit)
	{
		ucK2=0;
	}
	else
	{ 
		ucK2=1;
	}
	
	pstPid->U += pstPid->KP * (pstPid->E - pstPid->PreE) 
                 + ucK1 * ucK2 * pstPid->KI * pstPid->E 
                 + pstPid->KD * (pstPid->E - 2 * pstPid->PreE + pstPid->PrePreE);
	
	pstPid->PrePreE = pstPid->PreE;
	pstPid->PreE = pstPid->E;
}

