#ifndef WEIGHTS_H
#define WEIGHTS_H

#define MAX_WEIGHT 10

//�����[a,b]
extern int RandomBetween(int a, int b);

//��Ȩֵ�����
extern int GetResultByWeight(int weight[], int len);

//Ȩ�س�
class CWeight {
public:
	CWeight();
	~CWeight();
public:
	//��ʼ��Ȩ�ؼ���
	void init(int weight[], int len);
	//���Ȩ������
	void shuffleSeq();
	//��Ȩֵ�����
	int getResultByWeight();
public:
	int count_;				 //ͳ�Ƹ���
	int weights_[MAX_WEIGHT];//Ȩ�ؼ���
	int indexID_[MAX_WEIGHT];//��Ӧ����
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//��������
enum ExchangeTy {
	Exchange = 0,	//����
	NoExchange,		//����
	MaxExchange,
};

//���ݻ��Ƹ���Ȩ�ؼ��㵱ǰ�Ƿ���
//ratioExchange ���Ƹ���Ȩ�� 
extern ExchangeTy CalcExchangeOrNot(int ratioExchange);
extern ExchangeTy CalcExchangeOrNot2(CWeight& pool);

//���԰�Ȩ��������ʽ��
//д���ļ��ٵ���Excel������ͼ��鿴������̬�ֲ����
//filename char const* Ҫд����ļ� ��/home/testweight.txt
extern void TestWeightsRatio(char const* filename);
extern void TestWeightsRatio2(char const* filename);

#endif
