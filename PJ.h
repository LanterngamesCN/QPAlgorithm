//
// Created by andy_ro@qq.com
// 			5/26/2019
//
#ifndef GAME_LOGIC_PJ_H
#define GAME_LOGIC_PJ_H

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <memory.h>
#include <list>
#include <vector>
#include <boost/get_pointer.hpp>
#include <boost/shared_ptr.hpp>

#ifndef isZero
#define isZero(a)        ((a>-0.000001) && (a<0.000001))
#endif//isZero

#define MAX_CARD_TOTAL	PJ::MaxCardTotal	//牌总个数
#define GAME_PLAYER		PJ::MaxPlayer		//最多4人局
#define MIN_GAME_PLAYER	PJ::MinPlayer		//至少2人局
#define MAX_COUNT		PJ::MaxCount		//每人2张牌
#define MAX_ROUND       PJ::MaxRound		//最大局数

//抢庄牌九
namespace PJ {

	const int MaxCardTotal	= 32;	//牌总个数
	const int MaxPlayer		= 4;	//最多4人局
	const int MinPlayer		= 2;	//至少2人局
	const int MaxCount		= 2;	//每人2张牌
	const int MaxRound		= 2;	//最大局数

	//单牌类型：从大到小
	//天牌 > 地牌 > 人牌 > 鹅牌 > 梅牌 > 长三 >
	//板凳 > 斧头 > 红头十 > 高脚七 > 零霖六 > 杂九 =
	//杂九 > 杂八 = 杂八 > 杂七 = 杂七 > 杂五 =
	//杂五 > 二四 = 丁三
	enum CardTy {
		CNIL = 0,		    //无
		D12  = 1,			//丁三
		D24  = 2,			//二四
		Z32  = 3,			//杂五(32型)
		Z14  = 4,			//杂五(14型)
		Z25  = 5,			//杂七(25型)
		Z34  = 6,			//杂七(34型)
		Z26  = 7,			//杂八(26型)
		Z35  = 8,			//杂八(35型)
		Z36  = 9,			//杂九(36型)
		Z45  = 10,			//杂九(45型)
		L15  = 11,			//零霖六
		G16  = 12,			//高脚七
		H46  = 13,			//红头十
		F56  = 14,			//斧头
		B22  = 15,			//板凳
		C33  = 16,			//长三
		M55  = 17,			//梅牌
		E13  = 18,			//鹅牌
		R44  = 19,			//人牌
		D11  = 20,			//地牌
		T66  = 21,   		//天牌
		CARDMAX,
	};

	//对牌类型：从大到小
	//至尊 > 双天 > 双地 > 双人 > 双鹅 >
	//双梅 > 双长三 > 双板凳 > 双斧头 > 双红头 >
	//双高脚 > 双零霖 > 杂九 > 杂八 > 杂七 >
	//杂五 > 天王 > 地王 > 天杠 > 地杠 >
	//天高九 > 地高九
	enum HandTy {
		PNIL = 0,		//无

		/////////// 普通牌点
		PP0  = 1,			//零点
		PP1  = 2,			//一点
		PP2  = 3,			//二点
		PP3  = 4,			//三点
		PP4  = 5,			//四点
		PP5  = 6,			//五点
		PP6  = 7,			//六点
		PP7  = 8,			//七点
		PP8  = 9,			//八点
		PP9  = 10,			//九点

		/////////// 特殊牌型
		DG9  = 11,			//地高九
		TG9  = 12,			//天高九
		PDG  = 13,			//地杠
		PTG  = 14,			//天杠
		PDW  = 15,			//地王
		PTW  = 16,			//天王
		PZ5  = 17,			//杂五
		PZ7  = 18,			//杂七
		PZ8  = 19,			//杂八
		PZ9  = 20,			//杂九
		PLL  = 21,			//双零霖
		PGJ  = 22,			//双高脚
		PHT  = 23,			//双红头
		PFT  = 24,			//双斧头
		PBB  = 25,			//双板凳
		PC3  = 26,			//双长三
		PMM  = 27,			//双梅
		PEE  = 28,			//双鹅
		PRR  = 29,			//双人
		PDD  = 30,			//双地
		PTT  = 31,			//双天
		PZZ  = 32,			//至尊
	};

	//游戏逻辑类
	class CGameLogic
	{
	public:
		//构造函数
		CGameLogic();
		//析构函数
		virtual ~CGameLogic();
	public:
		//初始化扑克牌数据
		void InitCards();
		//debug打印
		void DebugListCards();
		//剩余牌数
		int8_t Remaining();
		//余牌
		uint8_t const* LeftCards();
		//洗牌
		void ShuffleCards();
		//发牌，生成n张玩家手牌
		void DealCards(int8_t n, uint8_t *cards);
	public:
		//0x66->PAITIAN
		static CardTy CardTyByCard(uint8_t card);
		//卡牌类型字符串 0x66->"T66"
		static std::string StringCardTyByCard(uint8_t card);
	public:
		//对牌类型字符串
		static std::string StringHandTy(HandTy handTy);
		//T66->"T66"
		static std::string StringCardTy(CardTy cardTy);
		//T66->0x66
		static uint8_t CardByCardTy(CardTy cardTy);
		//单牌比较 0-相等 >0-card1大 <0-cards1小
		static int CompareCard(uint8_t card1, uint8_t card2);
		//手牌由大到小排序
		static void SortCards(uint8_t *cards, int n);
		//玩家手牌类型，对牌类型
		static HandTy JudgeHandTy(uint8_t *cards);
		//玩家手牌点数
		static int CalcCardsPoints(uint8_t *cards);
		//9->PP9
		static HandTy HandTyByPoint(int point);
		//单牌点数(牌值)
		static int GetCardValue(uint8_t card);
		//手牌字符串
		static std::string StringCards(uint8_t const* cards, int n);
		//打印n张牌
		static void PrintCardList(uint8_t const* cards, int n);
		//手牌字符串
		static std::string hexString(uint8_t* cards, int n);
	private:
		//拆分字符串"32 14"
		static void CardsBy(std::string const& strcards, std::vector<std::string>& vec);
		//生成n张牌<-"32 14"
		static void MakeCardList(std::vector<std::string> const& vec, uint8_t* cards, int size);
	public:
		//生成n张牌<-"32 14"
		static int MakeCardList(std::string const& strcards, uint8_t* cards, int size);
	public:
		//确定牌型
		static HandTy GetHandCardsType(uint8_t *cards);
		//玩家比牌(闲家与庄家比牌) 0-和局 >0-cards1赢 <0-cards2赢
		static int CompareHandCards(uint8_t *cards1, uint8_t *cards2);
	private:
		int8_t index_;
		uint8_t cardsData_[MaxCardTotal];
	};
};

#endif		 


