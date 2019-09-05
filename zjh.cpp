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
#include "zjh.h"

namespace ZJH {
	
	//一副扑克(52张)
	uint8_t s_CardListData[MaxCardTotal] =
	{
		0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D, // 方块 A - K
		0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D, // 梅花 A - K
		0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D, // 红心 A - K
		0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D, // 黑桃 A - K
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
		//printf("--- *** 初始化一副扑克...\n");
		memcpy(cardsData_, s_CardListData, sizeof(uint8_t)*MaxCardTotal);
	}

	//debug打印
	void CGameLogic::DebugListCards() {
		//手牌按花色升序(方块到黑桃)，同花色按牌值从小到大排序
		SortCardsColor(cardsData_, MaxCardTotal, true, true, true);
		for (int i = 0; i < MaxCardTotal; ++i) {
			printf("%02X %s\n", cardsData_[i], StringCard(cardsData_[i]).c_str());
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
		static uint32_t seed = (uint32_t)time(NULL);
		//int c = rand() % 20 + 5;
		int c = rand_r(&seed) % 20 + 5;
		for (int k = 0; k < c; ++k) {
			for (int i = 0; i < MaxCardTotal; ++i) {
				//int j = rand() % MaxCardTotal;
				int j = rand_r(&seed) % MaxCardTotal;
				if (i != j) {
					std::swap(cardsData_[i], cardsData_[j]);
				}
			}
		}
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
		int k = 0;
		for (int i = index_; i < index_ + n; ++i) {
			assert(i < MaxCardTotal);
			cards[k++] = cardsData_[i];
		}
		index_ += n;
	}

	//花色：黑>红>梅>方
	uint8_t CGameLogic::GetCardColor(uint8_t card) {
		return (card & 0xF0);
	}

	//牌值：A<2<3<4<5<6<7<8<9<10<J<Q<K
	uint8_t CGameLogic::GetCardValue(uint8_t card) {
		return (card & 0x0F);
	}

	//点数：2<3<4<5<6<7<8<9<10<J<Q<K<A
	uint8_t CGameLogic::GetCardPoint(uint8_t card) {
		uint8_t value = GetCardValue(card);
		return value == 0x01 ? 0x0E : value;
	}

	//花色和牌值构造单牌
	uint8_t CGameLogic::MakeCardWith(uint8_t color, uint8_t value) {
		return (GetCardColor(color) | GetCardValue(value));
	}

	//牌值大小：A<2<3<4<5<6<7<8<9<10<J<Q<K
	static bool byCardValueColorGG(uint8_t card1, uint8_t card2) {
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		if (v0 != v1) {
			//牌值不同比大小
			return v0 > v1;
		}
		//牌值相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 > c1;
	}

	static bool byCardValueColorGL(uint8_t card1, uint8_t card2) {
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		if (v0 != v1) {
			//牌值不同比大小
			return v0 > v1;
		}
		//牌值相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 < c1;
	}

	static bool byCardValueColorLG(uint8_t card1, uint8_t card2) {
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		if (v0 != v1) {
			//牌值不同比大小
			return v0 < v1;
		}
		//牌值相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 > c1;
	}

	static bool byCardValueColorLL(uint8_t card1, uint8_t card2) {
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		if (v0 != v1) {
			//牌值不同比大小
			return v0 < v1;
		}
		//牌值相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 < c1;
	}

	//牌点大小：2<3<4<5<6<7<8<9<10<J<Q<K<A
	static bool byCardPointColorGG(uint8_t card1, uint8_t card2) {
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		if (p0 != p1) {
			//牌点不同比大小
			return p0 > p1;
		}
		//牌点相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 > c1;
	}

	static bool byCardPointColorGL(uint8_t card1, uint8_t card2) {
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		if (p0 != p1) {
			//牌点不同比大小
			return p0 > p1;
		}
		//牌点相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 < c1;
	}

	static bool byCardPointColorLG(uint8_t card1, uint8_t card2) {
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		if (p0 != p1) {
			//牌点不同比大小
			return p0 < p1;
		}
		//牌点相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 > c1;
	}

	static bool byCardPointColorLL(uint8_t card1, uint8_t card2) {
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		if (p0 != p1) {
			//牌点不同比大小
			return p0 < p1;
		}
		//牌点相同比花色
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		return c0 < c1;
	}

	//手牌排序(默认按牌点降序排列)，先比牌值/点数，再比花色
	//byValue bool false->按牌点 true->按牌值
	//ascend bool false->降序排列(即从大到小排列) true->升序排列(即从小到大排列)
	//clrAscend bool false->花色降序(即黑桃到方块) true->花色升序(即从方块到黑桃)
	void CGameLogic::SortCards(uint8_t *cards, int n, bool byValue, bool ascend, bool clrAscend)
	{
		if (byValue) {
			if (ascend) {
				if (clrAscend) {
					//LL
					std::sort(cards, cards + n, byCardValueColorLL);
				}
				else {
					//LG
					std::sort(cards, cards + n, byCardValueColorLG);
				}
			}
			else {
				if (clrAscend) {
					//GL
					std::sort(cards, cards + n, byCardValueColorGL);
				}
				else {
					//GG
					std::sort(cards, cards + n, byCardValueColorGG);
				}
			}
		}
		else {
			if (ascend) {
				if (clrAscend) {
					//LL
					std::sort(cards, cards + n, byCardPointColorLL);
				}
				else {
					//LG
					std::sort(cards, cards + n, byCardPointColorLG);
				}
			}
			else {
				if (clrAscend) {
					//GL
					std::sort(cards, cards + n, byCardPointColorGL);
				}
				else {
					//GG
					std::sort(cards, cards + n, byCardPointColorGG);
				}
			}
		}
	}
	
	//牌值大小：A<2<3<4<5<6<7<8<9<10<J<Q<K
	static bool byCardColorValueGG(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 > c1;
		}
		//花色相同比牌值
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		return v0 > v1;
	}
	
	static bool byCardColorValueGL(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 > c1;
		}
		//花色相同比牌值
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		return v0 < v1;
	}
	
	static bool byCardColorValueLG(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 < c1;
		}
		//花色相同比牌值
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		return v0 > v1;
	}

	static bool byCardColorValueLL(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 < c1;
		}
		//花色相同比牌值
		uint8_t v0 = CGameLogic::GetCardValue(card1);
		uint8_t v1 = CGameLogic::GetCardValue(card2);
		return v0 < v1;
	}

	//牌点大小：2<3<4<5<6<7<8<9<10<J<Q<K<A
	static bool byCardColorPointGG(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 > c1;
		}
		//花色相同比牌点
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		return p0 > p1;
	}

	static bool byCardColorPointGL(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 > c1;
		}
		//花色相同比牌点
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		return p0 < p1;
	}

	static bool byCardColorPointLG(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 < c1;
		}
		//花色相同比牌点
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		return p0 > p1;
	}

	static bool byCardColorPointLL(uint8_t card1, uint8_t card2) {
		uint8_t c0 = CGameLogic::GetCardColor(card1);
		uint8_t c1 = CGameLogic::GetCardColor(card2);
		if (c0 != c1) {
			//花色不同比花色
			return c0 < c1;
		}
		//花色相同比牌点
		uint8_t p0 = CGameLogic::GetCardPoint(card1);
		uint8_t p1 = CGameLogic::GetCardPoint(card2);
		return p0 < p1;
	}

	//手牌排序(默认按牌点降序排列)，先比花色，再比牌值/点数
	//clrAscend bool false->花色降序(即黑桃到方块) true->花色升序(即从方块到黑桃)
	//byValue bool false->按牌点 true->按牌值
	//ascend bool false->降序排列(即从大到小排列) true->升序排列(即从小到大排列)
	void CGameLogic::SortCardsColor(uint8_t *cards, int n, bool clrAscend, bool byValue, bool ascend)
	{
		if (byValue) {
			if (ascend) {
				if (clrAscend) {
					//LL
					std::sort(cards, cards + n, byCardColorValueLL);
				}
				else {
					//GL
					std::sort(cards, cards + n, byCardColorValueGL);
				}
			}
			else {
				if (clrAscend) {
					//LG
					std::sort(cards, cards + n, byCardColorValueLG);
				}
				else {
					//GG
					std::sort(cards, cards + n, byCardColorValueGG);
				}
			}
		}
		else {
			if (ascend) {
				if (clrAscend) {
					//LL
					std::sort(cards, cards + n, byCardColorPointLL);
				}
				else {
					//GL
					std::sort(cards, cards + n, byCardColorPointGL);
				}
			}
			else {
				if (clrAscend) {
					//LG
					std::sort(cards, cards + n, byCardColorPointLG);
				}
				else {
					//GG
					std::sort(cards, cards + n, byCardColorPointGG);
				}
			}
		}
	}
	
	//牌值字符串 
	std::string CGameLogic::StringCardValue(uint8_t value)
	{
		if (0 == value) {
			return "?";
		}
		switch (value)
		{
		case A: return "A";
		case J: return "J";
		case Q: return "Q";
		case K: return "K";
		}
		char ch[3] = { 0 };
		sprintf(ch, "%d", value);
		return ch;
	}

	//花色字符串 
	std::string CGameLogic::StringCardColor(uint8_t color)
	{
		switch (color)
		{
		case Spade:   return "♠";
		case Heart:   return "♥";
		case Club:	  return "♣";
		case Diamond: return "♦";
		}
		return "?";
	}

	//单牌字符串
	std::string CGameLogic::StringCard(uint8_t card) {
		std::string s(StringCardColor(GetCardColor(card)));
		s += StringCardValue(GetCardValue(card));
		return s;
	}

	//牌型字符串
	std::string CGameLogic::StringHandTy(HandTy ty) {
		switch (ty)
		{
		case Tysp:   return "Tysp";
		case Ty20:    return "Ty20";
		case Ty123:   return "Ty123";
		case Tysc:   return "Tysc";
		case Ty123sc:  return "Ty123sc";
		case Ty30:    return "Ty30";
		case Tysp235: return "Tysp235";
		}
		return "";
	}

	//打印n张牌
	void CGameLogic::PrintCardList(uint8_t const* cards, int n, bool hide) {
		for (int i = 0; i < n; ++i) {
			if (cards[i] == 0 && hide) {
				continue;
			}
			printf("%s ", StringCard(cards[i]).c_str());
		}
		printf("\n");
	}
	
	//手牌字符串
	std::string CGameLogic::StringCards(uint8_t const* cards, int n) {
		std::string strcards;
		for (int i = 0; i < n; ++i) {
			if (i == 0) {
				strcards += StringCard(cards[i]);
			}
			else {
				strcards += " " + StringCard(cards[i]);
			}
		}
		return strcards;
	}
	
	//拆分字符串"♦A ♦3 ♥3 ♥4 ♦5 ♣5 ♥5 ♥6 ♣7 ♥7 ♣9 ♣10 ♣J"
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

	//字串构造牌"♦A"->0x11
	uint8_t CGameLogic::MakeCardBy(std::string const& name) {
		uint8_t color = 0, value = 0;
		if (0 == strncmp(name.c_str(), "♠", 3)) {
			color = Spade;
			std::string str(name.substr(3, -1));
			switch (str.front())
			{
			case 'J': value = J; break;
			case 'Q': value = Q; break;
			case 'K': value = K; break;
			case 'A': value = A; break;
			case 'T': value = T; break;
			default: {
				value = atoi(str.c_str());
				break;
			}
			}
		}
		else if (0 == strncmp(name.c_str(), "♥", 3)) {
			color = Heart;
			std::string str(name.substr(3, -1));
			switch (str.front())
			{
			case 'J': value = J; break;
			case 'Q': value = Q; break;
			case 'K': value = K; break;
			case 'A': value = A; break;
			case 'T': value = T; break;
			default: {
				value = atoi(str.c_str());
				break;
			}
			}
		}
		else if (0 == strncmp(name.c_str(), "♣", 3)) {
			color = Club;
			std::string str(name.substr(3, -1));
			switch (str.front())
			{
			case 'J': value = J; break;
			case 'Q': value = Q; break;
			case 'K': value = K; break;
			case 'A': value = A; break;
			case 'T': value = T; break;
			default: {
				value = atoi(str.c_str());
				break;
			}
			}
		}
		else if (0 == strncmp(name.c_str(), "♦", 3)) {
			color = Diamond;
			std::string str(name.substr(3, -1));
			switch (str.front())
			{
			case 'J': value = J; break;
			case 'Q': value = Q; break;
			case 'K': value = K; break;
			case 'A': value = A; break;
			case 'T': value = T; break;
			default: {
				value = atoi(str.c_str());
				break;
			}
			}
		}
		assert(value != 0);
		return value ? MakeCardWith(color, value) : 0;
	}

	//生成n张牌<-"♦A ♦3 ♥3 ♥4 ♦5 ♣5 ♥5 ♥6 ♣7 ♥7 ♣9 ♣10 ♣J"
	void CGameLogic::MakeCardList(std::vector<std::string> const& vec, uint8_t *cards, int size) {
		int c = 0;
		for (std::vector<std::string>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			cards[c++] = MakeCardBy(it->c_str());
		}
	}

	//生成n张牌<-"♦A ♦3 ♥3 ♥4 ♦5 ♣5 ♥5 ♥6 ♣7 ♥7 ♣9 ♣10 ♣J"
	int CGameLogic::MakeCardList(std::string const& strcards, uint8_t *cards, int size) {
		std::vector<std::string> vec;
		CardsBy(strcards, vec);
		MakeCardList(vec, cards, size);
		return (int)vec.size();
	}
	
	//比较散牌大小
	int CGameLogic::CompareSanPai(uint8_t *cards1, uint8_t *cards2)
	{
		//牌型相同按顺序比点
		for (int i = 0; i < 3; ++i) {
			uint8_t p0 = CGameLogic::GetCardPoint(cards1[i]);
			uint8_t p1 = CGameLogic::GetCardPoint(cards2[i]);
			if (p0 != p1) {
				return p0 - p1;
			}
		}
		//点数相同按顺序比花色
		for (int i = 0; i < 3; ++i) {
			uint8_t c0 = GetCardColor(cards1[i]);
			uint8_t c1 = GetCardColor(cards2[i]);
			if (c0 != c1) {
				return c0 - c1;
			}
		}
		return 0;
	}

	//比较对子大小
	int CGameLogic::CompareDuiZi(uint8_t *cards1, uint8_t *cards2)
	{
		//先比对牌点数
		uint8_t p0 = CGameLogic::GetCardPoint(cards1[1]);
		uint8_t p1 = CGameLogic::GetCardPoint(cards2[1]);
		if (p0 != p1) {
			return p0 - p1;
		}
		//对牌点数相同，比单牌点数
		uint8_t sp0 = CGameLogic::GetCardPoint(cards1[0]);
		if (sp0 == p0) {
			sp0 = CGameLogic::GetCardPoint(cards1[2]);
		}
		uint8_t sp1 = CGameLogic::GetCardPoint(cards2[0]);
		if (sp1 == p1) {
			sp1 = CGameLogic::GetCardPoint(cards2[2]);
		}
		if (sp0 != sp1) {
			return sp0 - sp1;
		}
		//单牌点数相同，比对牌里面最大的花色
		uint8_t c0, c1;
		if (sp0 == p0) {
			c0 = GetCardColor(cards1[0]);
		}
		else {
			c0 = GetCardColor(cards1[1]);
		}
		if (sp1 == p1) {
			c1 = GetCardColor(cards2[0]);
		}
		else {
			c1 = GetCardColor(cards2[1]);
		}
		return c0 - c1;
	}

	//比较顺子大小
	int CGameLogic::CompareShunZi(uint8_t *cards1, uint8_t *cards2)
	{
		//牌型相同按顺序比点
		for (int i = 0; i < 3; ++i) {
			uint8_t p0 = CGameLogic::GetCardPoint(cards1[i]);
			uint8_t p1 = CGameLogic::GetCardPoint(cards2[i]);
			if (p0 != p1) {
				return p0 - p1;
			}
		}
		//点数相同按顺序比花色
		for (int i = 0; i < 3; ++i) {
			uint8_t c0 = GetCardColor(cards1[i]);
			uint8_t c1 = GetCardColor(cards2[i]);
			if (c0 != c1) {
				return c0 - c1;
			}
		}
		return 0;
	}

	//比较金花大小
	int CGameLogic::CompareJinHua(uint8_t *cards1, uint8_t *cards2)
	{
		//牌型相同按顺序比点
		for (int i = 0; i < 3; ++i) {
			uint8_t p0 = CGameLogic::GetCardPoint(cards1[i]);
			uint8_t p1 = CGameLogic::GetCardPoint(cards2[i]);
			if (p0 != p1) {
				return p0 - p1;
			}
		}
		//点数相同按顺序比花色
		uint8_t c0 = GetCardColor(cards1[0]);
		uint8_t c1 = GetCardColor(cards2[0]);
		return c0 - c1;
	}

	//比较顺金大小
	int CGameLogic::CompareShunJin(uint8_t *cards1, uint8_t *cards2)
	{
		//牌型相同按顺序比点
		for (int i = 0; i < 3; ++i) {
			uint8_t p0 = CGameLogic::GetCardPoint(cards1[i]);
			uint8_t p1 = CGameLogic::GetCardPoint(cards2[i]);
			if (p0 != p1) {
				return p0 - p1;
			}
		}
		//点数相同按顺序比花色
		uint8_t c0 = GetCardColor(cards1[0]);
		uint8_t c1 = GetCardColor(cards2[0]);
		return c0 - c1;
	}

	//比较豹子大小
	int CGameLogic::CompareBaoZi(uint8_t *cards1, uint8_t *cards2)
	{
		//炸弹比较点数
		uint8_t p0 = CGameLogic::GetCardPoint(cards1[0]);
		uint8_t p1 = CGameLogic::GetCardPoint(cards2[0]);
		return p0 - p1;
	}
	
	//玩家手牌类型
	HandTy CGameLogic::GetHandCardsType_private(uint8_t *cards) {
		//手牌按牌点从大到小排序(AK...32)
		SortCards(cards, MaxCount, false, false, false);
		uint8_t p0 = GetCardPoint(cards[0]);
		uint8_t p1 = GetCardPoint(cards[1]);
		uint8_t p2 = GetCardPoint(cards[2]);
		if (p0 == p1) {
			if (p1 == p2) {
				//豹子(炸弹)：三张值相同的牌
				return Ty30;
			}
			//对子：二张值相同的牌
			return Ty20;
		}
		if (p1 == p2) {
			//对子：二张值相同的牌
			return Ty20;
		}
		//三张连续牌
		if ((p1 == p2 + 1) && (p0 == p1 + 1 || p0 == p2 + 12)) {
			uint8_t c0 = GetCardColor(cards[0]);
			uint8_t c1 = GetCardColor(cards[1]);
			uint8_t c2 = GetCardColor(cards[2]);
			if (p0 == p2 + 12) {
				//手牌按牌值从大到小排序 A32->32A
				SortCards(cards, MaxCount, true, false, false);
			}
			if (c0 == c1 && c1 == c2) {
				//顺金(同花顺)：花色相同的顺子
				return Ty123sc;
			}
			//顺子：花色不同的顺子
			return Ty123;
		}
		uint8_t c0 = GetCardColor(cards[0]);
		uint8_t c1 = GetCardColor(cards[1]);
		uint8_t c2 = GetCardColor(cards[2]);
		if (c0 == c1 && c1 == c2) {
			//金花(同花)：花色相同，非顺子
			return Tysc;
		}
		if (p0 == 0x05 && p1 == 0x03 && p2 == 0x02) {
			//特殊：散牌中的235
			return Tysp235;
		}
		//散牌(高牌/单张)：三张牌不组成任何类型的牌
		return Tysp;
	}
	
	//玩家手牌类型
	HandTy CGameLogic::GetHandCardsType(uint8_t *cards, bool sp235) {
		HandTy ty = GetHandCardsType_private(cards);
		//不考虑特殊牌型
		if(!sp235) {
			if (ty == Tysp235) {
				ty = Tysp;
			}
		}
		return ty;
	}

	//比较手牌大小 >0-cards1大 <0-cards2大
	int CGameLogic::CompareHandCards(uint8_t *cards1, uint8_t *cards2, bool sp235)
	{
		HandTy t0 = GetHandCardsType(cards1, sp235);
		HandTy t1 = GetHandCardsType(cards2, sp235);
		if (t0 == t1) {
			//牌型相同情况
			if (t0 == Tysp235) {
				t0 = t1 = Tysp;
			}
		label:
			switch (t0)
			{
			case Tysp:
			{
				return CompareSanPai(cards1, cards2);
			}
			case Ty20:
			{
				return CompareDuiZi(cards1, cards2);
			}
			case Ty123:
			{
				return CompareShunZi(cards1, cards2);
			}
			case Tysc:
			{
				return CompareJinHua(cards1, cards2);
			}
			case Ty123sc:
			{
				return CompareShunJin(cards1, cards2);
			}
			case Ty30:
			{
				return CompareBaoZi(cards1, cards2);
			}
			}
		}
		else /*if (t0 != t1)*/ {
			//牌型不同情况
			if (t0 == Tysp235) {
				if (t1 == Ty30) {
					return t0 - t1;
				}
				t0 = Tysp;
			} else if (t1 == Tysp235) {
				if (t0 == Ty30) {
					return t0 - t1;
				}
				t1 = Tysp;
			}
			if (t0 == t1) {
				goto label;
			}
			//牌型不同比较
			return t0 - t1;
		}
	}
	
	//是否含带A散牌
	bool CGameLogic::HasCardValue(uint8_t *cards, uint8_t cardValue, int count) {
		int c = 0;
		for (int i = 0; i < MAX_COUNT; ++i) {
			if (cardValue == GetCardValue(cards[i])) {
				if (++c == count) {
					return true;
				}
			}
		}
		return false;
	}
	
	//手牌点数最大牌
	uint8_t CGameLogic::MaxCard(uint8_t *cards) {
		//手牌按牌点从大到小排序(AK...32)
		SortCards(cards, MaxCount, false, false, false);
		return cards[0];
	}
	
	//手牌点数最小牌
	uint8_t CGameLogic::MinCard(uint8_t *cards) {
		//手牌按牌点从大到小排序(AK...32)
		SortCards(cards, MaxCount, false, false, false);
		return cards[2];
	}
	
	//比较手牌大小 >0-cards1大 <0-cards2大
	bool CGameLogic::GreaterHandCards(boost::shared_ptr<uint8_t>& cards1, boost::shared_ptr<uint8_t>& cards2)
	{
		return CompareHandCards(boost::get_pointer(cards1), boost::get_pointer(cards2)) > 0;
	}
	
	void CGameLogic::TestCards() {
		CGameLogic g;
		//初始化
		g.InitCards();
		//洗牌
		g.ShuffleCards();
		bool pause = false;
		while (1) {
			if (pause) {
				pause = false;
				if ('q' == getchar()) {
					break;
				}
			}
			//余牌不够则重新洗牌
			if (g.Remaining() < 2*MAX_COUNT) {
				g.ShuffleCards();
			}
			
			uint8_t cards[2][MAX_COUNT] = { 0 };
			HandTy ty[2] = { ZJH::Tysp };
			
			for (int i = 0; i < 2; ++i) {
				//发牌
				g.DealCards(MAX_COUNT, &(cards[i])[0]);
				//手牌排序
				CGameLogic::SortCards(&(cards[i])[0], MAX_COUNT, true, true, true);
				//手牌牌型
				ty[i] = CGameLogic::GetHandCardsType(&(cards[i])[0]);
			}
			
			if (ty[0] != ZJH::Tysp && ty[1] != ZJH::Tysp) {
			//if ((ty[0] == ZJH::Tysp235 && ty[1] == ZJH::Ty30) ||
			//	(ty[1] == ZJH::Tysp235 && ty[0] == ZJH::Ty30)) {
				std::string s1 = CGameLogic::StringCards(&(cards[0])[0], MAX_COUNT);
				std::string s2 = CGameLogic::StringCards(&(cards[1])[0], MAX_COUNT);
				std::string cc = CGameLogic::CompareHandCards(&(cards[0])[0], &(cards[1])[0]) > 0 ? ">" : "<";
				printf("[%s]%s %s [%s]%s\n",
					CGameLogic::StringHandTy(ty[0]).c_str(), s1.c_str(),
					cc.c_str(),
					CGameLogic::StringHandTy(ty[1]).c_str(), s2.c_str());
				pause = true;
			}
		}
	}
	
	//filename char const* 文件读取手牌 cardsList.ini
	void CGameLogic::TestCards(char const* filename) {
		std::vector<std::string> lines;
		readFile(filename, lines, ";;");
		//1->文件读取手牌 0->随机生成13张牌
		int flag = atoi(lines[0].c_str());
		//默认最多枚举多少组墩
		int size = atoi(lines[1].c_str());
		//1->文件读取手牌 0->随机生成13张牌
		if (flag > 0) {
			CGameLogic g;
			uint8_t cards[2][MAX_COUNT] = { 0 };
			HandTy ty[2] = { ZJH::Tysp };
			//line[2]构造一副手牌3张
			CGameLogic::MakeCardList(lines[2], &(cards[0])[0], MAX_COUNT);
			CGameLogic::MakeCardList(lines[3], &(cards[1])[0], MAX_COUNT);
			
			for (int i = 0; i < 2; ++i) {
				//手牌排序
				CGameLogic::SortCards(&(cards[i])[0], MAX_COUNT, true, true, true);
				//手牌牌型
				ty[i] = CGameLogic::GetHandCardsType(&(cards[i])[0]);
			}
			std::string s1 = CGameLogic::StringCards(&(cards[0])[0], MAX_COUNT);
			std::string s2 = CGameLogic::StringCards(&(cards[1])[0], MAX_COUNT);
			std::string cc = CGameLogic::CompareHandCards(&(cards[0])[0], &(cards[1])[0]) > 0 ? ">" : "<";
			printf("[%s]%s %s [%s]%s\n",
				CGameLogic::StringHandTy(ty[0]).c_str(), s1.c_str(),
				cc.c_str(),
				CGameLogic::StringHandTy(ty[1]).c_str(), s2.c_str());
		}
		else {
			TestCards();
		}
	}
};