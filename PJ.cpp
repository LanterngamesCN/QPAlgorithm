//
// Created by andy_ro@qq.com
// 			5/26/2019
//
#include <time.h>
#include <algorithm>
#include "math.h"
#include <iostream>
#include <memory>
#include <utility>
#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#include "cfg.h"
#include "PJ.h"
#include "StdRandom.h"

namespace PJ {
	
	//一副牌九(32张)
	uint8_t s_CardListData[MaxCardTotal] =
	{
		0x66,0x66,0x11,0x11,0x44,0x44,0x13,0x13,
		0x55,0x55,0x33,0x33,0x22,0x22,0x56,0x56,
		0x46,0x46,0x16,0x16,0x15,0x15,0x45,0x36,
		0x35,0x26,0x34,0x25,0x14,0x32,0x24,0x12,
	};

	//牌九信息  [T66] = {0x66, "T66"}
	struct CardInfo_t {
		uint8_t iType;
		std::string name;
	}s_cardInfo_tbl[CARDMAX] = {
		{ 0x00, "CNIL" },
		{ 0x12, "D12"  },//丁三
		{ 0x24, "D24"  },//二四
		{ 0x32, "Z32"  },//杂五(32型)
		{ 0x14, "Z14"  },//杂五(14型)
		{ 0x25, "Z25"  },//杂七(25型)
		{ 0x34, "Z34"  },//杂七(34型)
		{ 0x26, "Z26"  },//杂八(26型)
		{ 0x35, "Z35"  },//杂八(35型)
		{ 0x36, "Z36"  },//杂九(36型)
		{ 0x45, "Z45"  },//杂九(45型)
		{ 0x15, "L15"  },//零霖六
		{ 0x16, "G16"  },//高脚七
		{ 0x46, "H46"  },//红头十
		{ 0x56, "F56"  },//斧头
		{ 0x22, "B22"  },//板凳
		{ 0x33, "C33"  },//长三
		{ 0x55, "M55"  },//梅牌
		{ 0x13, "E13"  },//鹅牌
		{ 0x44, "R44"  },//人牌
		{ 0x11, "D11"  },//地牌
		{ 0x66, "T66"  },//天牌
	};

	//构造函数
	CGameLogic::CGameLogic()
	{
		index_ = 0;
		memset(cardsData_, 0, sizeof(uint8_t)*MaxCardTotal);
	}

	//析构函数
	CGameLogic::~CGameLogic()
	{

	}

	//初始化扑克牌数据
	void CGameLogic::InitCards()
	{
		//printf("--- *** 初始化一副牌九...\n");
		memcpy(cardsData_, s_CardListData, sizeof(uint8_t)*MaxCardTotal);
	}

	//debug打印
	void CGameLogic::DebugListCards() {
		for (int i = 0; i < MaxCardTotal; ++i) {
			printf("%02X:%d:%s\n",
				cardsData_[i],
				GetCardValue(cardsData_[i]),
				StringCardTyByCard(cardsData_[i]).c_str());
		}
	}

	//剩余牌数
	int8_t CGameLogic::Remaining() {
		return int8_t(MaxCardTotal - index_);
	}

	//洗牌
	void CGameLogic::ShuffleCards()
	{
		//printf("-- *** 洗牌...\n");
#if 0
		for (int i = MaxCardTotal - 1; i > 0; --i) {
			std::uniform_int_distribution<decltype(i)> d(0, i);
			int j = d(STD::Generator::instance().get_mt());
			std::swap(cardsData_[i], cardsData_[j]);
		}
#else
		std::shuffle(&cardsData_[0], &cardsData_[MAX_CARD_TOTAL], STD::Generator::instance().get_mt());
#endif
		index_ = 0;
	}

	//发牌，生成n张玩家手牌
	void CGameLogic::DealCards(int8_t n, uint8_t *cards)
	{
		//printf("-- *** %d张余牌，发牌 ...\n", Remaining());
		if (cards == NULL) {
			return;
		}
		if (n > Remaining()) {
			return;
		}
		std::shuffle(&cardsData_[index_], &cardsData_[MAX_CARD_TOTAL], STD::Generator::instance().get_mt());
		int k = 0;
		for (int i = index_; i < index_ + n; ++i) {
			cards[k++] = cardsData_[i];
		}
		index_ += n;
	}

	//0x66->PAITIAN
	CardTy CGameLogic::CardTyByCard(uint8_t card)
	{
		CardTy cardTy = CNIL;
		switch (card) {
		case 0x66: cardTy = T66; break;
		case 0x11: cardTy = D11; break;
		case 0x44: cardTy = R44; break;
		case 0x13: cardTy = E13; break;
		case 0x55: cardTy = M55; break;
		case 0x33: cardTy = C33; break;
		case 0x22: cardTy = B22; break;
		case 0x56: cardTy = F56; break;
		case 0x46: cardTy = H46; break;
		case 0x16: cardTy = G16; break;
		case 0x15: cardTy = L15; break;
		case 0x45: cardTy = Z45; break;
		case 0x36: cardTy = Z36; break;
		case 0x35: cardTy = Z35; break;
		case 0x26: cardTy = Z26; break;
		case 0x34: cardTy = Z34; break;
		case 0x25: cardTy = Z25; break;
		case 0x14: cardTy = Z14; break;
		case 0x32: cardTy = Z32; break;
		case 0x24: cardTy = D24; break;
		case 0x12: cardTy = D12; break;
		}
		return cardTy;
	}

	//卡牌类型字符串 0x66->"T66"
	std::string CGameLogic::StringCardTyByCard(uint8_t card) {
		std::string cardTy = "CNIL";
		switch (card)
		{
		case 0x66: cardTy = "T66"; break;
		case 0x11: cardTy = "D11"; break;
		case 0x44: cardTy = "R44"; break;
		case 0x13: cardTy = "E13"; break;
		case 0x55: cardTy = "M55"; break;
		case 0x33: cardTy = "C33"; break;
		case 0x22: cardTy = "B22"; break;
		case 0x56: cardTy = "F56"; break;
		case 0x46: cardTy = "H46"; break;
		case 0x16: cardTy = "G16"; break;
		case 0x15: cardTy = "L15"; break;
		case 0x45: cardTy = "Z45"; break;
		case 0x36: cardTy = "Z36"; break;
		case 0x35: cardTy = "Z35"; break;
		case 0x26: cardTy = "Z26"; break;
		case 0x34: cardTy = "Z34"; break;
		case 0x25: cardTy = "Z25"; break;
		case 0x14: cardTy = "Z14"; break;
		case 0x32: cardTy = "Z32"; break;
		case 0x24: cardTy = "D24"; break;
		case 0x12: cardTy = "D12"; break;
		}
		return cardTy;
	}

	//对牌类型字符串
	std::string CGameLogic::StringHandTy(HandTy handTy)
	{
		std::string cardTy = "PNIL";
		switch (handTy)
		{
		/////////// 特殊牌型
		case PNIL:  cardTy = "PNIL"; break;	//无
		case PZZ: cardTy = "PZZ"; break;	//至尊
		case PTT: cardTy = "PTT"; break;	//双天
		case PDD: cardTy = "PDD"; break;	//双地
		case PRR: cardTy = "PRR"; break;	//双人
		case PEE: cardTy = "PEE"; break;	//双鹅
		case PMM: cardTy = "PMM"; break;	//双梅
		case PC3: cardTy = "PC3"; break;	//双长三
		case PBB: cardTy = "PBB"; break;	//双板凳
		case PFT: cardTy = "PFT"; break;	//双斧头
		case PHT: cardTy = "PHT"; break;	//双红头
		case PGJ: cardTy = "PGJ"; break;	//双高脚
		case PLL: cardTy = "PLL"; break;	//双零霖
		case PZ9: cardTy = "PZ9"; break;	//杂九
		case PZ8: cardTy = "PZ8"; break;	//杂八
		case PZ7: cardTy = "PZ7"; break;	//杂七
		case PZ5: cardTy = "PZ5"; break;	//杂五
		case PTW: cardTy = "PTW"; break;	//天王
		case PDW: cardTy = "PDW"; break;	//地王
		case PTG: cardTy = "PTG"; break;	//天杠
		case PDG: cardTy = "PDG"; break;	//地杠
		case TG9: cardTy = "TG9"; break;	//天高九
		case DG9: cardTy = "DG9"; break;	//地高九
		/////////// 普通牌点 
		case PP9: cardTy = "PP9"; break;	//九点
		case PP8: cardTy = "PP8"; break;	//八点
		case PP7: cardTy = "PP7"; break;	//七点
		case PP6: cardTy = "PP6"; break;	//六点
		case PP5: cardTy = "PP5"; break;	//五点
		case PP4: cardTy = "PP4"; break;	//四点
		case PP3: cardTy = "PP3"; break;	//三点
		case PP2: cardTy = "PP2"; break;	//二点
		case PP1: cardTy = "PP1"; break;	//一点
		case PP0: cardTy = "PP0"; break;	//零点
		}
		return cardTy;
	}

	//T66->"T66"
	std::string CGameLogic::StringCardTy(CardTy cardTy)
	{
		return s_cardInfo_tbl[cardTy].name;
	}

	//T66->0x66
	uint8_t CGameLogic::CardByCardTy(CardTy cardTy)
	{
		return s_cardInfo_tbl[cardTy].iType;
	}

	//单牌比较 0-相等 >0-card1大 <0-cards1小
	int CGameLogic::CompareCard(uint8_t card1, uint8_t card2)
	{
		//单牌大小
		//天牌 > 地牌 > 人牌 > 鹅牌 > 梅牌 > 长三 >
		//板凳 > 斧头 > 红头十 > 高脚七 > 零霖六 > 杂九 =
		//杂九 > 杂八 = 杂八 > 杂七 = 杂七 > 杂五 =
		//杂五 > 二四 = 丁三
		CardTy t1 = CardTyByCard(card1);
		CardTy t2 = CardTyByCard(card2);
		if (t1 == t2) {
			return 0; //card1==card2
		}
		int d = std::abs(t1 - t2);
		if (d == 1) {//相邻
			int m = std::max(t1, t2);
			if (m == Z36 || m == Z26 || m == Z25 || m == Z32 || m == D12)
				return 0; //card1==card2
		}
		return t1 - t2;
	}

	//单牌大小
	static bool compareByCardType(uint8_t card1, uint8_t card2) {
		int t1 = CGameLogic::CardTyByCard(card1);
		int t2 = CGameLogic::CardTyByCard(card2);
		return t1 > t2;
	}

	//手牌由大到小排序
	void CGameLogic::SortCards(uint8_t *cards, int n)
	{
		std::sort(cards, cards + n, compareByCardType);
	}

	//玩家手牌类型，对牌类型
	HandTy CGameLogic::JudgeHandTy(uint8_t *cards) {
		HandTy t = PNIL;
		//手牌由大到小排序
		//SortCards(cards, MAX_COUNT);
		uint8_t t1 = CardTyByCard(cards[0]);
		uint8_t t2 = CardTyByCard(cards[1]);
		if (t1 == D24 && t2 == D12)
			t = PZZ;	//至尊
		else if (t1 == T66 && t2 == T66)
			t = PTT;	//双天
		else if (t1 == D11 && t2 == D11)
			t = PDD;	//双地
		else if (t1 == R44 && t2 == R44)
			t = PRR;	//双人
		else if (t1 == E13 && t2 == E13)
			t = PEE;	//双鹅
		else if (t1 == M55 && t2 == M55)
			t = PMM;	//双梅
		else if (t1 == C33 && t2 == C33)
			t = PC3;	//双长三
		else if (t1 == B22 && t2 == B22)
			t = PBB;	//双板凳
		else if (t1 == F56 && t2 == F56)
			t = PFT;	//双斧头
		else if (t1 == H46 && t2 == H46)
			t = PHT;	//双红头
		else if (t1 == G16 && t2 == G16)
			t = PGJ;	//双高脚
		else if (t1 == L15 && t2 == L15)
			t = PLL;	//双零霖
		else if (t1 == Z45 && t2 == Z36)
			t = PZ9;	//杂九
		else if (t1 == Z35 && t2 == Z26)
			t = PZ8;	//杂八
		else if (t1 == Z34 && t2 == Z25)
			t = PZ7;	//杂七
		else if (t1 == Z14 && t2 == Z32)
			t = PZ5;	//杂五
		else if (t1 == T66 && (t2 == Z45 || t2 == Z36))
			t = PTW;	//天王
		else if (t1 == D11 && (t2 == Z45 || t2 == Z36))
			t = PDW;	//地王
		else if (t1 == T66 && (t2 == R44 || t2 == Z35 || t2 == Z26))
			t = PTG;	//天杠
		else if (t1 == D11 && (t2 == R44 || t2 == Z35 || t2 == Z26))
			t = PDG;	//地杠
		else if (t1 == T66 && t2 == Z25)
			t = TG9;	//天高九
		else if (t1 == D11 && t2 == G16)
			t = DG9;	//地高九
		return t;
	}

	//玩家手牌点数
	int CGameLogic::CalcCardsPoints(uint8_t *cards)
	{
		//手牌由大到小排序
		//SortCards(cards, MAX_COUNT);
		return (GetCardValue(cards[0]) + GetCardValue(cards[1])) % 10;
	}

	//9->PP9
	HandTy CGameLogic::HandTyByPoint(int point)
	{
		HandTy ty = PNIL;
		switch (point)
		{
		case 9: ty = PP9; break;
		case 8: ty = PP8; break;
		case 7: ty = PP7; break;
		case 6: ty = PP6; break;
		case 5: ty = PP5; break;
		case 4: ty = PP4; break;
		case 3: ty = PP3; break;
		case 2: ty = PP2; break;
		case 1: ty = PP1; break;
		case 0: ty = PP0; break;
		}
		return ty;
	}

	//单牌点数(牌值)
	int CGameLogic::GetCardValue(uint8_t card) {
		return (card & 0x0F) + ((card >> 4) & 0x0F);
	}

	//玩家比牌(闲家与庄家比牌) 0-和局 >0-cards1赢 <0-cards2赢
	int CGameLogic::CompareHandCards(uint8_t *cards1, uint8_t *cards2)
	{
		//手牌由大到小排序
		//SortCards(cards1, MAX_COUNT);
		//SortCards(cards2, MAX_COUNT);
		//判断玩家手牌类型
		HandTy pairType1 = JudgeHandTy(cards1);
		HandTy pairType2 = JudgeHandTy(cards2);
		//有对牌且牌型不同
		if (pairType1 != pairType2) {
			if (pairType1 == PNIL) {
				return -1;
			}
			if (pairType2 == PNIL) {
				return 1;
			}
			return pairType1 - pairType2;
		}
		//pairType1 == pairType2 != nil 有对牌且牌型相同
		if (pairType1 != PNIL) {
			return 0;//和局
		}
		//pairType1 == pairType2 == nil 无对牌，两牌点数和取个位数来进行比牌 
		int pt1 = CalcCardsPoints(cards1);
		int pt2 = CalcCardsPoints(cards2);
		//点数不同
		if (pt1 != pt2) {
			return pt1 - pt2;
		}
		//点数相同，比较单牌最大牌大小
		int result = CompareCard(cards1[0], cards2[0]);
		if (result != 0) {
			return result;
		}
		//result == 0 单牌最大牌大小相同，比较单牌最小牌大小
		result = CompareCard(cards1[1], cards2[1]);
		if (result != 0) {
			return result;
		}
		//result == 0 单牌最小牌大小相同
		return 0;//和局
	}
	
	//确定牌型
	HandTy CGameLogic::GetHandCardsType(uint8_t *cards)
	{
		HandTy handTy = JudgeHandTy(cards);
		if (handTy != PNIL) {
			return handTy;//特殊牌型
		}
		handTy = HandTyByPoint(CalcCardsPoints(cards));
		return handTy;//普通牌点
	}

	//手牌字符串
	std::string CGameLogic::StringCards(uint8_t const* cards, int n) {
		std::string strcards;
		for (int i = 0; i < n; ++i) {
			if (i == 0) {
				char c[10] = { 0 };
				snprintf(c, sizeof(c), "%02X:%d:", cards[i], GetCardValue(cards[i]));
				strcards += c + StringCardTyByCard(cards[i]);//StringCardTy(CardTyByCard(cards[i]));
			}
			else {
				char c[10] = { 0 };
				snprintf(c, sizeof(c), "%02X:%d:", cards[i], GetCardValue(cards[i]));
				strcards += " " + std::string(c) + StringCardTyByCard(cards[i]);//StringCardTy(CardTyByCard(cards[i]));
			}
		}
		return strcards;
	}
	
	//打印n张牌
	void  CGameLogic::PrintCardList(uint8_t const* cards, int n) {
		for (int i = 0; i < n; ++i) {
			printf("%02X:%d:%s\n",
				cards[i],
				GetCardValue(cards[i]),
				StringCardTyByCard(cards[i]).c_str());
		}
		printf("\n");
	}

	//拆分字符串"32 14"
	void CGameLogic::CardsBy(std::string const& strcards, std::vector<std::string>& vec) {
		std::string str(strcards);
		while (true) {
			std::string::size_type s = str.find_first_of(' ');
			if (-1 == s) {
				break;
			}
			vec.push_back(str.substr(0, s));
			str = str.substr(s + 1);
		}
		if (!str.empty()) {
			vec.push_back(str.substr(0, -1));
		}
	}

	static int cHex(char c) {
		if (c >= '0' && c <= '9') {
			return c - '0';
		}
		if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}
		if (c >= 'A' && c <= 'F') {
			return c - 'A' + 10;
		}
		return -1;
	}
	
	static int sHex(char const *data, int len) {
		int x = 0;
		for (int i = 0; i < len; ++i) {
			x |= cHex(data[i]) << ((len - i - 1) * 4);
		}
		return x;
	}

	//生成n张牌<-"32 14"
	void CGameLogic::MakeCardList(std::vector<std::string> const& vec, uint8_t* cards, int size) {
		int c = 0;
		for (std::vector<std::string>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			cards[c++] = (uint8_t)sHex(it->c_str(), it->length());
		}
	}

	//生成n张牌<-"32 14"
	int CGameLogic::MakeCardList(std::string const& strcards, uint8_t* cards, int size) {
		std::vector<std::string> vec;
		CardsBy(strcards, vec);
		MakeCardList(vec, cards, size);
		return (int)vec.size();
	}
};