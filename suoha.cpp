//
// Created by andy_ro@qq.com
// 			11/19/2021
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
#include <map>

#include "funcC.h"
#include "cfg.h"
#include "suoha.h"
#include "StdRandom.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>

namespace SUOHA {

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
		//SortCardsColor(cardsData_, MaxCardTotal, true, true, true);
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
		case Tysp:		return "Tysp";
		case Ty20:		return "Ty20";
		case Ty22:		return "Ty22";
		case Ty30:		return "Ty30";
		case Ty123:		return "Ty123";
		case Tysc:		return "Tysc";
		case Ty32:		return "Ty32";
		case Ty40:		return "Ty40";
		case Ty123sc:	return "Ty123sc";
		case Ty123scRoyal: return "Ty123scRoyal";
		}
		assert(false);
		return "";
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

	//获取牌有效列数
	//cards uint8_t const* 相同牌值n张牌(n<=4)
	//n int 黑/红/梅/方4张牌
	uint8_t CGameLogic::get_card_c(uint8_t const* cards, int n) {
		assert(n <= 4);
		for (int i = 0; i < n; ++i) {
			if (cards[i] == 0) {
				return i;
			}
		}
		return n;
	}

	//返回指定花色牌列号
	//cards uint8_t const* 相同牌值n张牌(n<=4)
	//n int 黑/红/梅/方4张牌
	//clr CardColor 指定花色
	uint8_t CGameLogic::get_card_colorcol(uint8_t const* cards, int n, CardColor clr) {
		assert(n <= 4);
		//int c = get_card_c(cards, n);
		for (int i = 0; i < n/*c*/; ++i) {
			if (clr == GetCardColor(cards[i])) {
				return i;
			}
		}
		return 0xFF;
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

	//手牌点数最大牌
	uint8_t CGameLogic::MaxCard(uint8_t const* cards, size_t size) {
		uint8_t card = 0;
		for (size_t i = 0; i < size; ++i) {
			if (i == 0) {
				card = cards[i];
			}
			else if (GetCardPoint(card) < GetCardPoint(cards[i])) {
				card = cards[i];
			}
		}
		return card;
	}
	
	uint8_t CGameLogic::MaxCard(std::vector<uint8_t> const& cards) {
		assert(cards.size() > 0);
		return CGameLogic::MaxCard(&cards[0], cards.size());
	}
	
	//手牌点数最小牌
	uint8_t CGameLogic::MinCard(uint8_t const* cards, size_t size) {
		uint8_t card = 0;
		for (size_t i = 0; i < size; ++i) {
			if (i == 0) {
				card = cards[i];
			}
			else if (GetCardPoint(card) > GetCardPoint(cards[i])) {
				card = cards[i];
			}
		}
		return card;
	}
	
	uint8_t CGameLogic::MinCard(std::vector<uint8_t> const& cards) {
		assert(cards.size() > 0);
		return CGameLogic::MinCard(&cards[0], cards.size());
	}

	//返回去重后的余牌/散牌
	//src uint8_t const* 牌源
	//pdst uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//dst4 uint8_t(*)[4] 存放所有四张牌型，c4 四张牌型数
	//dst3 uint8_t(*)[4] 存放所有三张牌型，c3 三张牌型数
	//dst2 uint8_t(*)[4] 存放所有对子牌型，c2 对子牌型数
	//cpy uint8_t* 去重后的余牌/散牌(除去四张/三张/对子)///////
	int CGameLogic::RemoveRepeatCards(uint8_t const* src, int len,
		uint8_t(**const pdst)[4],
		uint8_t(*dst4)[4], int& c4, uint8_t(*dst3)[4], int& c3,
		uint8_t(*dst2)[4], int& c2, uint8_t *cpy, int& cpylen) {
		uint8_t const* psrc = src;
		int lensrc = len;
		uint8_t cpysrc[MaxSZ] = { 0 };
		//枚举所有重复四张牌型
		int const size4 = 3;
		//uint8_t dst4[size4][4] = { 0 };
		c4 = EnumRepeatCards(psrc, lensrc, 4, dst4, size4, cpy, cpylen);
		if (c4 > 0) {
			memcpy(cpysrc, cpy, cpylen);//cpysrc
			lensrc = cpylen;//lensrc
			psrc = cpysrc;//psrc
		}
		//枚举所有重复三张牌型
		int const size3 = 4;
		//uint8_t dst3[size3][4] = { 0 };
		c3 = EnumRepeatCards(psrc, lensrc, 3, dst3, size3, cpy, cpylen);
		if (c3 > 0) {
			memcpy(cpysrc, cpy, cpylen);//cpysrc
			lensrc = cpylen;//lensrc
			psrc = cpysrc;//psrc
		}
		//枚举所有重复二张牌型
		int const size2 = 6;
		//uint8_t dst2[size2][4] = { 0 };
		c2 = EnumRepeatCards(psrc, lensrc, 2, dst2, size2, cpy, cpylen);
		if (c2 == 0) {
			memcpy(cpy, psrc, lensrc);//cpy
			cpylen = lensrc;//cpylen
		}
		//用psrc衔接dst4/dst3/dst2 //////
		//uint8_t(*pdst[6])[4] = { 0 };
		//typedef uint8_t(*Ptr)[4];
		//Ptr pdst[6] = { 0 };
		int c = 0;
		//所有重复四张牌型
		for (int i = 0; i < c4; ++i) {
			pdst[c++] = &dst4[i];
		}
		//所有重复三张牌型
		for (int i = 0; i < c3; ++i) {
			pdst[c++] = &dst3[i];
		}
		//所有重复二张牌型
		for (int i = 0; i < c2; ++i) {
			pdst[c++] = &dst2[i];
		}
		return c;
	}

	static bool byValueG(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t v0 = CGameLogic::GetCardValue(a[0][0]);
		uint8_t v1 = CGameLogic::GetCardValue(b[0][0]);
		return v0 > v1;
	}
	static bool byValueL(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t v0 = CGameLogic::GetCardValue(a[0][0]);
		uint8_t v1 = CGameLogic::GetCardValue(b[0][0]);
		return v0 < v1;
	}
	static bool byPointG(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 > p1;
	}
	static bool byPointL(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 < p1;
	}
	static void SortCards_src(uint8_t(**const psrc)[4], int n, bool byValue, bool ascend) {
		if (byValue) {
			if (ascend) {
				std::sort(psrc, psrc + n, byValueL);
			}
			else {
				std::sort(psrc, psrc + n, byValueG);
			}
		}
		else {
			if (ascend) {
				std::sort(psrc, psrc + n, byPointL);
			}
			else {
				std::sort(psrc, psrc + n, byPointG);
			}
		}
	}
	
	//枚举所有指定重复牌型(四张/三张/二张)
	//src uint8_t const* 牌源
	//n int 抽取n张(4/3/2)
	//dst uint8_t(*)[4] 存放指定重复牌型
	//cpy uint8_t* 抽取后不符合要求的余牌
	int CGameLogic::EnumRepeatCards(uint8_t const* src, int len, int n,
		uint8_t(*dst)[4], int size, uint8_t *cpy, int& cpylen) {
		//手牌按牌值从小到大排序(A23...QK)
		//SortCards(cards, len, true, true, true);
		assert(n <= 4);
		//assert(len > 0);
		int i = 0, j = 0, k = 0, dstLen = 0;
		cpylen = 0;
	next:
		int s = i++;
		//while (i < len) {
		//	s = i++;
		//	v_src_pre = GetCardValue(src[s]);
		//	if (v_src_pre > 0) {
		//		break;
		//	}
		//}
		if (s + n <= len) {
			uint8_t v_src_pre = GetCardValue(src[s]);
			for (; i < len; ++i) {
				//src中当前的牌值
				uint8_t v_src_cur = GetCardValue(src[i]);
				//两张牌值相同的牌
				if (v_src_pre == v_src_cur) {
					//收集到n张牌后返回
					if (i - s + 1 == n) {
						//缓存符合要求的牌型
						memcpy(dst[dstLen++], &src[s], n);
						//printf("--- *** dst s:%d i:%d\n", s, i);
						//PrintCardList(dst[dstLen - 1], n);
						++i;
						goto next;
					}
				}
				//空位
				//else if (v_src_cur == 0) {
				//}
				//两张牌值不同的牌
				else {
					//缓存不合要求的副本
					memcpy(&cpy[k], &src[s], i - s);
					k += (i - s);
					//printf("--- *** cpy s:%d i:%d\n", s, i);
					//PrintCardList(cpy, k);
					//赋值新的
					s = i;
					v_src_pre = GetCardValue(src[s]);
				}
			}
		}
		if (dstLen) {
			//缓存不合要求的副本
			if (s < len) {
				memcpy(&cpy[k], &src[s], len - s);
				cpylen = k;
				cpylen += len - s;
			}
			else {
				cpylen = k;
			}
			//if (cpylen > 0) {
			//	printf("--- *** cpy s:%d i:%d\n", s, i);
			//	PrintCardList(cpy, cpylen);
			//}
		}
		return dstLen;
	}

	//填充卡牌条码标记位
	//src uint8_t* 用A2345678910JQK来占位 size = MaxSZ
	//dst uint8_t const* 由单张组成的余牌或枚举组合牌
	void CGameLogic::FillCardsMarkLoc(uint8_t *src, int size, uint8_t const* dst, int dstlen, bool reset) {
		assert(size == MaxSZ);
		if (reset) {
			memset(src, 0, MaxSZ * sizeof(uint8_t));
		}
		for (int i = 0; i < dstlen; ++i) {
			uint8_t v = GetCardValue(dst[i]);
			assert(v >= A && v <= K);
			if (src[v - 1] != 0) {
				//assert(false);
			}
			src[v - 1] = dst[i];
		}
	}
	
	//填充卡牌条码标记位
	//src uint8_t* 用2345678910JQKA来占位 size = MaxSZ
	//dst uint8_t const* 由单张组成的余牌或枚举组合牌
	void CGameLogic::FillCardsMarkLocByPoint(uint8_t *src, int size, uint8_t const* dst, int dstlen, bool reset) {
		assert(size == MaxSZ);
		if (reset) {
			memset(src, 0, MaxSZ * sizeof(uint8_t));
		}
		for (int i = 0; i < dstlen; ++i) {
			uint8_t p = GetCardPoint(dst[i]);
			assert(p >= 2 && p <= GetCardPoint(A));
			if (src[p - 1] != 0) {
				//assert(false);
			}
			src[p - 1] = dst[i];
		}
	}
	
	//从补位合并后的单张组合牌中枚举所有不区分花色n张连续牌型///////
	//src uint8_t const* 单张组合牌源，去重后的余牌/散牌与从psrc每组中各抽一张牌补位合并
	//n int 抽取n张(3/5/13)
	//start int const 检索src/mark的起始下标
	//psrcctx short const* 标记psrc的ctx信息
	//dst std::vector<std::vector<uint8_t>>& 存放所有连续牌型(不区分花色)
	//clr std::vector<bool>& 对应dst是否同花
	int CGameLogic::EnumConsecCardsMarkLoc(uint8_t const* src, int len, int n,
		int const start, short const* psrcctx,
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst,
		std::vector<bool>& clr) {
		assert(len > 0);
		int i = start, j = 0, k = 0, s;
		uint8_t v_src_pre = 0, c_src_pre = 0;
		bool sameclr = true;
	next:
		while (i < len) {
			s = i++;
			v_src_pre = GetCardValue(src[s]);
			c_src_pre = GetCardColor(src[s]);
			if (v_src_pre > 0) {
				break;
			}
		}
		sameclr = true;
		if (s + n <= len) {
			for (; i < len; ++i) {
				//src中当前牌值
				uint8_t v_src_cur = GetCardValue(src[i]);
				//src中当前花色
				uint8_t c_src_cur = GetCardColor(src[i]);
				//牌位有值则连续
				if (v_src_cur > 0) {
					if (sameclr && c_src_cur != c_src_pre) {
						//printf("c_src_pre[%d] = %s c_src_cur[%d] = %s\n",
						//	s, StringCardColor(c_src_pre).c_str(),
						//	i, StringCardColor(c_src_cur).c_str());
						sameclr = false;
					}
					//收集到n张牌后返回
					if (i - s + 1 == n) {
						//缓存符合要求的牌型
						std::vector<uint8_t> v(&src[s], &src[s] + n);
						//连续n张牌
						dst.push_back(v);
						//printf("--- *** dst s:%d i:%d\n", s, i);
						//PrintCardList(&v.front(), n);
						//是否同花色
						clr.push_back(sameclr);
						//printf("--- *** sameclr = %s\n", sameclr ? "true" : "false");
						//查找ctx信息
						std::vector<short> x;
						//检查s到i中的牌是否存在psrc中
						for (k = s; k <= i; ++k) {
							if (psrcctx[k] > 0) {
								//当前连续牌映射psrc的ctx信息
								x.push_back(psrcctx[k]);
								//psrc第r组/牌数c
								//uint8_t r = (0xFF & (psrcctx[k] >> 8));
								//uint8_t	c = (0xFF & psrcctx[k]);
								//printf("[%d][%d]%s\n", r, c, StringCard(src[k]).c_str());
							}
						}
						//printf("--- *** x.size = %d\n", x.size());
						if (x.size() > 0) {
							ctx.push_back(x);
						}
						else {
							x.push_back(0xFF);
							ctx.push_back(x);
						}
#if 0
						++s;
						v_src_pre = GetCardValue(src[s]);
						c_src_pre = GetCardColor(src[s]);
						sameclr = true;
#else
						i = ++s;
						goto next;
#endif

					}
				}
				//牌位无值不连续
				else {
					++i;
					goto next;
				}
			}
		}
		return dst.size();
	}
	
	//从补位合并后的单张组合牌中枚举所有n张指定花色牌型///////
	//src uint8_t const* 单张组合牌源，去重后的余牌/散牌与从psrc每组中各抽一张牌补位合并
	//n int 抽取n张(3/5/13)
	//clr CardColor 指定花色
	//psrcctx short const* 标记psrc的ctx信息
	//dst std::vector<std::vector<uint8_t>>& 存放所有同花牌型(非顺子)
	//consec bool false->不保留同花顺 true->保留同花顺 
	//dst2 std::vector<std::vector<uint8_t>>& 存放所有同花顺牌型
	int CGameLogic::EnumSameColorCardsMarkLoc(uint8_t const* src, int len, int n, CardColor clr,
		short const* psrcctx,
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst,
		bool consec,
		std::vector<std::vector<uint8_t>>& dst2) {
		//assert(len > 0);
		int i = 0, j = 0, k = 0, s, c = 0;
		uint8_t cards[MaxSZ] = { 0 };
	next:
		while (k < len) {
			s = k++;
			uint8_t c_src_pre = GetCardColor(src[s]);
			if (c_src_pre > 0 && c_src_pre == clr) {
				break;
			}
		}
		c = 0;
		if (s + n <= len) {
			for (i = s; i < len; ++i) {
				//src中当前牌花色
				uint8_t c_src_cur = GetCardColor(src[i]);
				//当前牌是指定花色
				if (clr == c_src_cur) {
					//收集到n张牌后返回
					cards[c++] = src[i];
					if (c == n) {
						//缓存符合要求的牌型
						if (i - s + 1 == n) {
							if(consec) {
								//s到i为花色相同的n张连续牌型(同花顺)，不缓存
								std::vector<uint8_t> v(&src[s], &src[s] + n);
								dst2.push_back(v);
								//printf("--- *** dst s:%d i:%d n:%d\n", s, i, n);
								//PrintCardList(&v.front(), n);
							}
						}
						else {
							//s到i中间有无效牌或不同于当前花色的牌，但已收集到n张与当前花色相同的牌
							std::vector<uint8_t> v(&cards[0], &cards[0] + c);
							dst.push_back(v);
							//printf("--- *** dst s:%d i:%d n:%d\n", s, i, n);
							//PrintCardList(&v.front(), n);
						}
						c = 0;
						goto next;
					}
				}
				//牌位无值
				else if (c_src_cur == 0) {
				}
				//当前牌非指定花色
				else {
				}
			}
		}
		return dst.size();
	}

	//枚举所有不区分花色n张连续牌型(同花顺/顺子)，先去重再补位，最后遍历查找
	//去重后的余牌/散牌与单张组合牌补位合并//////
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//psrc uint8_t(**const)[4] 从所有四张/三张/对子中各取一张合成单张组合牌
	//cpy uint8_t const* 去重后的余牌/散牌(除去四张/三张/对子)牌源///////
	//n int 抽取n张(3/5/13)
	//ctx std::vector<std::vector<short>>& 标记psrc的ctx信息
	//dst std::vector<std::vector<uint8_t>>& 存放所有连续牌型
	//clr std::vector<bool>& 对应dst是否同花
	void CGameLogic::EnumConsecCards(
		uint8_t(**const psrc)[4], int const psrclen,
		uint8_t const* cpy, int const cpylen, int n,
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst,
		std::vector<bool>& clr) {
		//用A2345678910JQK来占位 size = MaxSZ
		uint8_t mark[MaxSZ] = { 0 };
		//标记psrc的ctx信息mark[j]=>psrcctx[j]=>psrc[i],c
		short psrcctx[MaxSZ] = { 0 };
		//用去重后的余牌/散牌进行补位
		FillCardsMarkLoc(mark, MaxSZ, cpy, cpylen, true);
		//printf("\n\n");
		//PrintCardList(mark, MaxSZ, false);
		//psrc[ai],ac,av
		uint8_t ai = 0, ac = 0, av = 0;
		for (int i = 0; i < psrclen; ++i) {
			//psrc第i组/牌数c ///
			uint8_t c = get_card_c(psrc[i][0], 4);
			//mark[j]=>psrcctx[j]=>psrc[i],c
			uint8_t v = GetCardValue((*psrc[i])[0]);
			//当前组合位于psrc的ctx信息
			psrcctx[v - 1] = (short)(((0xFF & i) << 8) | (0xFF & c));
			//从psrc每组中各抽一张牌补位 //////
			FillCardsMarkLoc(mark, MaxSZ, psrc[i][0], 1, false);
			if (v == A) {
				ai = i, ac = c, av = v;
			}
		}
		//补位完成后判断能否构成10JQKA牌型A...K
		if (GetCardValue(mark[0]) == A && GetCardValue(mark[K - 1]) == K) {
			int i;
			for (i = K - 2; i > K - n; --i) {
				if (mark[i] == 0) {
					//有断层不连续
					break;
				}
			}
			//满足要求的连续n张
			if (i == K - n) {
				//psrc中有单牌A
				if (av > 0) {
					//当前组合位于psrc的ctx信息
					psrcctx[K] = (short)(((0xFF & ai) << 8) | (0xFF & ac));
				}
				//mark下标为K的位置用单牌A占位
				mark[K] = av > 0 ? (*psrc[ai])[0] : mark[0];
				//printf("r:%d c:%d ===>>> %s\n", ai, ac, StringCard(mark[K]).c_str());
			}
		}
		//printf("mark[%d] n = %d\n", MaxSZ, n);
		//PrintCardList(mark, MaxSZ, false);
		//printf("\n\n\n");
		//补位合并后枚举所有n张连续牌型
		EnumConsecCardsMarkLoc(mark, MaxSZ, n, 0, psrcctx, ctx, dst, clr);
	}

	//枚举所有n张相同花色不连续牌型(同花)，先去重再补位，最后遍历查找
	//去重后的余牌/散牌与单张组合牌补位合并//////
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//psrc uint8_t(**const)[4] 从所有四张/三张/对子中各取一张合成单张组合牌
	//cpy uint8_t const* 去重后的余牌/散牌(除去四张/三张/对子)牌源///////
	//n int 抽取n张(3/5/13)
	//ctx std::vector<std::vector<short>>& 标记psrc的ctx信息
	//dst std::vector<std::vector<uint8_t>>& 存放所有同花牌型
	void CGameLogic::EnumSameColorCards(
		uint8_t(**const psrc)[4], int const psrclen,
		uint8_t const* cpy, int const cpylen, int n,
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst) {
		//printf("--- *** 枚举所有同花\n");
		CardColor color[4] = { Spade,Heart,Club,Diamond, };
		//按黑/红/梅/方顺序依次遍历color[k]
		for (int k = 0; k < 4; ++k) {
			//用A2345678910JQK来占位 size = MaxSZ
			uint8_t mark[MaxSZ] = { 0 };
			//标记psrc的ctx信息mark[j]=>psrcctx[j]=>psrc[i],c
			//short psrcctx[MaxSZ] = { 0 };
			//用去重后的余牌/散牌进行补位
			FillCardsMarkLoc(mark, MaxSZ, cpy, cpylen, false);
			//printf("\n\n");
			//printf("--- *** >>> color[%s]\n", StringCardColor(color[k]).c_str());
			//PrintCardList(mark, MaxSZ, false);
			for (int i = 0; i < psrclen; ++i) {
				//psrc第i组/牌数c ///
				uint8_t c = get_card_c(psrc[i][0], 4);
				uint8_t j = get_card_colorcol(psrc[i][0], (int)c, color[k]);
				if (j != 0xFF) {
					//PrintCardList(psrc[i][0], c, false);
					//printf("--- *** +++ color[%s] psrc[%d][%d]\n", StringCardColor(color[k]).c_str(), i, j);
					assert(GetCardValue((*psrc[i])[0]) == GetCardValue((*psrc[i])[j]));
					//mark[j]=>psrcctx[j]=>psrc[i],c
					//uint8_t v = GetCardValue((*psrc[i])[0]);
					//当前组合位于psrc的ctx信息
					//psrcctx[v - 1] = (short)(((0xFF & i) << 8) | (0xFF & c));
					//从psrc每组中调取指定花色牌补位 //////
					FillCardsMarkLoc(mark, MaxSZ, &(*psrc[i])[j], 1, false);
				}
			}
			//PrintCardList(mark, MaxSZ, false);
			//printf("\n\n\n");
			//std::vector<std::vector<uint8_t>> _;
			//补位合并后枚举所有n张当前花色牌型(同花非顺子)，不保留同花顺(在别的地方枚举出来了)//////
			//EnumSameColorCardsMarkLoc(mark, MaxSZ, n, color[k], NULL/*psrcctx*/, ctx, dst, false, _);
			int c = 0;
			//所有当前花色牌位于mark索引ctx
			short psrcctx[MaxSZ] = { 0 };
			for (int i = 0; i < MaxSZ; ++i) {				
				if (color[k] == GetCardColor(mark[i])) {
					assert(GetCardValue(mark[i]) > 0);
					psrcctx[c++] = i;
				}
			}
			//满足要求的n张同花
 			if (c >= n) {
 				CFuncC fnC;
 				std::vector<std::vector<int>> vec;
 				//////从c个当前花色牌里面选取任意n个的组合数C(c,n) //////
 				int m = fnC.FuncC(c, n, vec);
				//printf("--- *** C(%d,%d) = %d\n", c, n, m);
				//CFuncC::Print(vec);
				for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
					it != vec.end(); ++it) {
					if (psrcctx[*it->begin()] - psrcctx[*it->rbegin()] + 1 == n) {
						//同花顺
						std::vector<uint8_t> v;
						for (std::vector<int>::const_reverse_iterator ir = it->rbegin();
							ir != it->rend(); ++ir) {
							assert(psrcctx[*ir] < MaxSZ &&
								mark[psrcctx[*ir]] > 0);
							v.push_back(mark[psrcctx[*ir]]);
						}
						//_.push_back(v);
						//PrintCardList(&v.front(), v.size());
					}
					//判断能否构成10JQKA牌型A...K
					else if (GetCardValue(mark[psrcctx[*it->rbegin()]]) == A &&
						     GetCardValue(mark[psrcctx[*it->begin()]]) == K &&
						psrcctx[*it->begin()] - psrcctx[*(it->rbegin() + 1)] + 1 == n - 1) {
						//同花顺10JQKA/QKA
						std::vector<uint8_t> v;
						for (std::vector<int>::const_reverse_iterator ir = it->rbegin();
							ir != it->rend(); ++ir) {
							assert(psrcctx[*ir] < MaxSZ &&
								mark[psrcctx[*ir]] > 0);
							v.push_back(mark[psrcctx[*ir]]);
						}
						//_.push_back(v);
						//PrintCardList(&v.front(), v.size());
					}
					else {
						//同花
						std::vector<uint8_t> v;
						for (std::vector<int>::const_reverse_iterator ir = it->rbegin();
							ir != it->rend(); ++ir) {
							assert(psrcctx[*ir] < MaxSZ &&
								mark[psrcctx[*ir]] > 0);
							v.push_back(mark[psrcctx[*ir]]);
						}
						dst.push_back(v);
						//PrintCardList(&v.front(), v.size());
					}
				}
				//printf("\n---\n");
 			}
		}
		//for (std::vector<std::vector<uint8_t>>::const_iterator it = dst.begin();
		//	it != dst.end(); ++it) {
		//	PrintCardList(&it->front(), it->size());
		//}
	}

	//枚举所有同花顺/顺子(区分花色五张连续牌)
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//psrc uint8_t(**const)[4] 从所有四张/三张/对子中各取一张合成单张组合牌
	//ctx std::vector<std::vector<short>>& 标记psrc的ctx信息
	//src std::vector<std::vector<uint8_t>> const& 所有单张组合牌源///////
	//clr std::vector<bool> const& 对应src是否同花
	//dst1 std::vector<std::vector<uint8_t>>& 存放所有同花顺
	//dst2 std::vector<std::vector<uint8_t>>& 存放所有顺子
	void CGameLogic::EnumConsecCardsByColor(
		uint8_t(**const pdst)[4], int const pdstlen,
		std::vector<std::vector<short>> const& ctx,
		std::vector<std::vector<uint8_t>> const& src,
		std::vector<bool> const& clr,
		std::vector<std::vector<uint8_t>>& dst1,
		std::vector<std::vector<uint8_t>>& dst2) {
		//printf("--- *** -------------------------------------------\n");
		//printf("--- *** pdstlen = %d ctx.size = %d\n", pdstlen, ctx.size());
		assert(src.size() == ctx.size());
		int i = 0;
		for (std::vector<std::vector<uint8_t>>::const_iterator it = src.begin();
			it != src.end(); ++it) {
			//PrintCardList(&it->front(), it->size());
			typedef uint8_t(*Ptr)[4];
			Ptr psrc[6] = { 0 };
			int colc[6] = { 0 };
			int k = 0;
			//printf("--->>> start ctx.size=%d\n", ctx[i].size());
			std::vector<short> const& pctx = ctx[i++];
			for (std::vector<short>::const_iterator ir = pctx.begin();
				ir != pctx.end(); ++ir) {
				if (*ir == 0xFF) {
					break;
				}
				//pdst第j组/牌数c
				uint8_t j = (0xFF & ((*ir) >> 8));
				uint8_t	c = (0xFF & (*ir));
				//牌值A,2,3,4,5,6,7,8,9,10,J,Q,K
				uint8_t vdst_c = GetCardValue((*pdst[j])[0]);
				uint8_t vsrc_s = GetCardValue((&it->front())[0]);
				uint8_t vsrc_p = GetCardValue((&it->front())[it->size() - 1]);
				//牌点2,3,4,5,6,7,8,9,10,J,Q,K,A
				uint8_t pdst_c = GetCardPoint((*pdst[j])[0]);
				uint8_t psrc_s = GetCardPoint((&it->front())[0]);
				uint8_t psrc_p = GetCardPoint((&it->front())[it->size() - 1]);
				assert(
					(vdst_c >= vsrc_s && vdst_c <= vsrc_p) ||
					(pdst_c >= psrc_s && pdst_c <= psrc_p));
				//PrintCardList(&(*pdst[j])[0], c);
				colc[k] = c;
				psrc[k++] = pdst[j];
			}
			if (k > 0) {
				std::vector<std::vector<int>> vec;
				//psrc组与组之间按牌值升序排列(从小到大)
				//SortCards_src(psrc, k, true, true);
				//printf("---\n");
				//PrintCardList(&it->front(), it->size());
				//f(k)=C(n,1)
				//Multi(k)=f(1)*f(2)...*f(k)
				//////从选取的k组中分别任取一张牌的组合数 //////
				int c = DepthVisit(4, k, psrc, colc, *it, dst1, dst2);
				//printf("--- c=%d dst1(sameclr)=%d dst2=%d\n", c, dst1.size(), dst2.size());
			}
			else {
				clr[i - 1] ? dst1.push_back(*it) : dst2.push_back(*it);
			}
			//printf("--->>> end\n\n");
		}
	}
	
	//求组合C(n,1)*C(n,1)...*C(n,1)
	//f(k)=C(n,1)
	//Multi(k)=f(1)*f(2)...*f(k)
	//n int 访问广度
	//k int 访问深度
	//深度优先遍历，由浅到深，广度遍历，由里向外
	int CGameLogic::DepthVisit(int n,
		int k,
		uint8_t(*const*const psrc)[4],
		int const* colc,
		std::vector<uint8_t> const& vec,
		std::vector<std::vector<uint8_t>>& dst0,
		std::vector<std::vector<uint8_t>>& dst1) {
		int c = 0;
		static int const KDEPTH = MaxSZ;
		int e[KDEPTH + 1] = { 0 };
		e[0] = k;
		//dst0.clear();
		//dst1.clear();
		return DepthC(n, k, e, c, psrc, colc, vec, dst0, dst1);
	}
	
	//递归求组合C(n,1)*C(n,1)...*C(n,1)
	//f(k)=C(n,1)
	//Multi(k)=f(1)*f(2)...*f(k)
	//n int 访问广度
	//k int 访问深度
	//深度优先遍历，由浅到深，广度遍历，由里向外
	int CGameLogic::DepthC(int n,
		int k, int *e, int& c,
		uint8_t(*const*const psrc)[4],
		int const* colc,
		std::vector<uint8_t> const& vec,
		std::vector<std::vector<uint8_t>>& dst0,
		std::vector<std::vector<uint8_t>>& dst1) {
		for (int i = colc[k - 1]/*n*/; i > 0; --i) {
			//if ((*psrc[k - 1])[i - 1] == 0) {
			//	continue;
			//}
			assert(k > 0);
			//psrc第[j - 1]组第e[j] - 1张(0/1/2/3)
			//psrc第[j - 1]组第e[j] - 1张(0/1/2)
			//psrc第[j - 1]组第e[j] - 1张(0/1)
			e[k] = i;
			if (k > 1) {
				DepthC(colc[k - 1]/*n*/, k - 1, e, c, psrc, colc, vec, dst0, dst1);
			}
			else {
				++c;
				std::vector<uint8_t> v;
				//深度1...e[0]
				for (int j = e[0]; j > 0; --j) {
					//printf("%d", e[j]);
					//printf("%s", StringCard((*psrc[j - 1])[e[j] - 1]).c_str());
					v.push_back((*psrc[j - 1])[e[j] - 1]);
				}
				//printf(",");
				//PrintCardList(&vec[0], vec.size());
				//用2345678910JQKA来占位 size = MaxSZ
				uint8_t mark[MaxSZ] = { 0 };
				if (GetCardValue(*vec.rbegin()) == A) {
					//按牌点来补位
					FillCardsMarkLocByPoint(mark, MaxSZ, &vec[0], vec.size(), false);
					FillCardsMarkLocByPoint(mark, MaxSZ, &v[0], v.size(), false);
				}
				//用A2345678910JQK来占位 size = MaxSZ
				else {
					//按牌值来补位
					FillCardsMarkLoc(mark, MaxSZ, &vec[0], vec.size(), false);
					FillCardsMarkLoc(mark, MaxSZ, &v[0], v.size(), false);
				}
				v.clear();
				bool sameclr = true;
				uint8_t c_pre = 0;
				for (int j = 0; j < MaxSZ; ++j) {
					if (mark[j] > 0) {
						if (c_pre == 0) {
							c_pre = GetCardColor(mark[j]);
						}
						else if(c_pre != GetCardColor(mark[j])) {
							sameclr = false;
						}
						v.push_back(mark[j]);
					}
				}
				//PrintCardList(&v[0], v.size());
				sameclr ?
					dst0.push_back(v) :
					dst1.push_back(v);
				//v.clear();
			}
		}
		return c;
	}
	
	//同花顺之间/同花之间/顺子之间比较大小
 	static bool As123scbyPointG(std::vector<uint8_t> const* a, std::vector<uint8_t> const* b) {
		return CGameLogic::CompareCards(&a->front(), a->size(), &b->front(), b->size()/*, true*/, Tysp) > 0;
 	}
	//同花顺之间/同花之间/顺子之间比较大小
 	static bool As123scbyPointL(std::vector<uint8_t> const* a, std::vector<uint8_t> const* b) {
		return CGameLogic::CompareCards(&a->front(), a->size(), &b->front(), b->size()/*, true*/, Tysp) < 0;
 	}
	//同花顺之间/同花之间/顺子之间比较大小
	static void SortCardsByPoint_src123sc(std::vector<uint8_t> const** psrc, int n, bool ascend) {
		if (ascend) {
			std::sort(psrc, psrc + n, As123scbyPointL);
		}
		else {
			std::sort(psrc, psrc + n, As123scbyPointG);
		}
	}

	//简单牌型分类/重复(四张/三张/二张)/同花/顺子/同花顺/散牌
	//src uint8_t const* 牌源
	//n int 抽取n张(5/5/3)
	//pdst uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//dst4 uint8_t(*)[4] 存放所有四张牌型，c4 四张牌型数
	//dst3 uint8_t(*)[4] 存放所有三张牌型，c3 三张牌型数
	//dst2 uint8_t(*)[4] 存放所有对子牌型，c2 对子牌型数
	//cpy uint8_t* 存放去重后的余牌/散牌(除去四张/三张/对子)///////
	//dst0 std::vector<std::vector<uint8_t>>& 存放所有同花色n张连续牌
	//dst1 std::vector<std::vector<uint8_t>>& 存放所有非同花n张连续牌
	//dstc std::vector<std::vector<uint8_t>>& 存放所有同花n张非连续牌
	void CGameLogic::ClassifyCards(uint8_t const* src, int len, int n,
		uint8_t(**const pdst)[4],
		uint8_t(*dst4)[4], int& c4,
		uint8_t(*dst3)[4], int& c3,
		uint8_t(*dst2)[4], int& c2,
		uint8_t *cpy, int& cpylen,
		std::vector<std::vector<uint8_t>>& dst0,
		std::vector<std::vector<uint8_t>>& dst1,
		std::vector<std::vector<uint8_t>>& dstc) {
		int c = 0/*c4 = 0, c3 = 0, c2 = 0, cpylen = 0*/;
		//uint8_t cpy[MaxSZ] = { 0 };
		//所有重复四张牌型
		int const size4 = 3;
		//uint8_t dst4[size4][4] = { 0 };
		//所有重复三张牌型
		int const size3 = 4;
		//uint8_t dst3[size3][4] = { 0 };
		//所有重复二张牌型
		int const size2 = 6;
		//uint8_t dst2[size2][4] = { 0 };
		//返回去重后的余牌/散牌cpy
		//printf("------------\n");
		c = RemoveRepeatCards(src, len, pdst, dst4, c4, dst3, c3, dst2, c2, cpy, cpylen);
		//{
		//	int z = 0;
		//	for (int i = 0; i < c4; ++i)
		//		printf("[%d][%d]%s\n", z++, 4, StringCards(&(dst4[i])[0], 4).c_str());
		//	for (int i = 0; i < c3; ++i)
		//		printf("[%d][%d]%s\n", z++, 3, StringCards(&(dst3[i])[0], 3).c_str());
		//	for (int i = 0; i < c2; ++i)
		//		printf("[%d][%d]%s\n", z++, 2, StringCards(&(dst2[i])[0], 2).c_str());
		//}
		//printf("--- c = %d\n", c);
		//PrintCardList(cpy, cpylen);
		//printf("---\n\n");

		//pdst组与组之间按牌值升序排列(从小到大)
		//SortCards_src(pdst, c, true, true);
		//枚举所有不区分花色n张连续牌型(同花顺/顺子)
		std::vector<std::vector<short>> ctx;
		std::vector<std::vector<uint8_t>> dst, dst0_, dst1_, dstc_;
		std::vector<bool> clr;
		EnumConsecCards(pdst, c, cpy, cpylen, n, ctx, dst, clr);
		//枚举所有区分花色n张连续牌(同花顺/顺子)
		EnumConsecCardsByColor(pdst, c, ctx, dst, clr, dst0_, dst1_);
		//枚举所有同花色n张不连续牌型(同花)
		EnumSameColorCards(pdst, c, cpy, cpylen, n, ctx, dstc_);
		{
			//同花色n张连续牌
			int c = 0;
			static int const MaxSZ = 1500;
			std::vector<uint8_t> const* psrc[MaxSZ] = { 0 };
			for (std::vector<std::vector<uint8_t>>::const_iterator it = dst0_.begin();
				it != dst0_.end(); ++it) {
				assert(c < MaxSZ);
				psrc[c++] = &*it;
			}
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src123sc(psrc, c, false);
			for (int i = 0; i < c; ++i) {
				assert(psrc[i]->size() == n);
				std::vector<uint8_t> v(&psrc[i]->front(), &psrc[i]->front() + psrc[i]->size());
				dst0.push_back(v);
			}
		}
		{
			//非同花n张连续牌
			int c = 0;
			static int const MaxSZ = 1500;
			std::vector<uint8_t> const* psrc[MaxSZ] = { 0 };
			for (std::vector<std::vector<uint8_t>>::const_iterator it = dst1_.begin();
				it != dst1_.end(); ++it) {
				assert(c < MaxSZ);
				psrc[c++] = &*it;
			}
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src123sc(psrc, c, false);
			for (int i = 0; i < c; ++i) {
				assert(psrc[i]->size() == n);
				std::vector<uint8_t> v(&psrc[i]->front(), &psrc[i]->front() + psrc[i]->size());
				dst1.push_back(v);
			}
		}
		{
			//同花n张非连续牌
			int c = 0;
			static int const MaxSZ = 1500;
			std::vector<uint8_t> const* psrc[MaxSZ] = { 0 };
			for (std::vector<std::vector<uint8_t>>::const_iterator it = dstc_.begin();
				it != dstc_.end(); ++it) {
				assert(c < MaxSZ);
				psrc[c++] = &*it;
			}
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src123sc(psrc, c, false);
			for (int i = 0; i < c; ++i) {
				assert(psrc[i]->size() == n);
				std::vector<uint8_t> v(&psrc[i]->front(), &psrc[i]->front() + psrc[i]->size());
				dstc.push_back(v);
			}
		}
	}

	//求组合C(n,k)
	//n int 访问广度
	//k int 访问深度
	//深度优先遍历，由浅到深，广度遍历，由里向外
	int CGameLogic::FuncC(int n, int k,
		uint8_t(*const*const psrc)[4], int const r,
		std::vector<std::vector<uint8_t>>& dst) {
		int c = 0;
		static int const KDEPTH = MaxSZ;
		int e[KDEPTH + 1] = { 0 };
		e[0] = k;
		return FuncC(n, k, e, c, psrc, r, dst);
	}

	//递归求组合C(n,k)
	//n int 访问广度
	//k int 访问深度
	//深度优先遍历，由浅到深，广度遍历，由里向外
	int CGameLogic::FuncC(int n, int k, int *e, int& c,
		uint8_t(*const*const psrc)[4], int const r,
		std::vector<std::vector<uint8_t>>& dst) {
		for (int i = n; i > 0; --i) {
			if ((*psrc[r])[i - 1] == 0) {
				continue;
			}
			assert(k > 0);
			//psrc第[r]组第e[j] - 1张(0/1/2/3)
			//psrc第[r]组第e[j] - 1张(0/1/2)
			//psrc第[r]组第e[j] - 1张(0/1)
			e[k] = i;
			if (k > 1) {
				//递归C(i-1,k-1)
				FuncC(i - 1, k - 1, e, c, psrc, r, dst);
			}
			else {
				++c;
				std::vector<uint8_t> v;
				//深度1...e[0]
				for (int j = e[0]; j > 0; --j) {
					//printf("%d", e[j]);
					//printf("%s", StringCard((*psrc[r])[e[j] - 1]).c_str());
					v.push_back((*psrc[r])[e[j] - 1]);
				}
				//printf(",");
				dst.push_back(v);
				//v.clear();
			}
		}
		return c;
	}
	
	//葫芦牌型之间比大小
	static bool As32byPointG(const uint8_t(*const a)[5], const uint8_t(*const b)[5]) {
		//比较三张的大小(中间的牌)
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][2]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][2]);
		if (p0 != p1) {
			return p0 > p1;
		}
		//三张大小相同，对子取最小
		uint8_t sp0 = CGameLogic::GetCardPoint(a[0][0]);
		if (sp0 == p0) {
			sp0 = CGameLogic::GetCardPoint(a[0][4]);
		}
		uint8_t sp1 = CGameLogic::GetCardPoint(b[0][0]);
		if (sp1 == p1) {
			sp1 = CGameLogic::GetCardPoint(b[0][4]);
		}
		return sp0 >/*<*/ sp1;
	}
	//葫芦牌型之间比大小
	static bool As32byPointL(const uint8_t(*const a)[5], const uint8_t(*const b)[5]) {
		//比较三张的大小(中间的牌)
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][2]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][2]);
		if (p0 != p1) {
			return p0 < p1;
		}
		//三张大小相同，对子取最小
		uint8_t sp0 = CGameLogic::GetCardPoint(a[0][0]);
		if (sp0 == p0) {
			sp0 = CGameLogic::GetCardPoint(a[0][4]);
		}
		uint8_t sp1 = CGameLogic::GetCardPoint(b[0][0]);
		if (sp1 == p1) {
			sp1 = CGameLogic::GetCardPoint(b[0][4]);
		}
		return sp0 </*>*/ sp1;
	}
	//葫芦牌型之间比大小
	static void SortCardsByPoint_src32(uint8_t(**const psrc)[5], int n, bool ascend) {
		if (ascend) {
			std::sort(psrc, psrc + n, As32byPointL);
		}
		else {
			std::sort(psrc, psrc + n, As32byPointG);
		}
	}

	//枚举所有葫芦(一组三条加上一组对子)
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
	//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
	//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
	//dst std::vector<std::vector<uint8_t>>& 存放所有葫芦牌型
	int CGameLogic::EnumCards32(
		uint8_t(**const psrc)[4],
		uint8_t(*const src4)[4], int const c4,
		uint8_t(*const src3)[4], int const c3,
		uint8_t(*const src2)[4], int const c2,
		std::vector<std::vector<uint8_t>>& dst) {
		//printf("--- *** 枚举所有葫芦\n");
		int c = 0;
		static int const MaxSZ = 500;
		uint8_t src[MaxSZ][5] = { 0 };
		int n = c4 + c3 + c2;
		CFuncC fnC;
		std::vector<std::vector<int>> vec;
		//psrc组与组之间按牌值升序排列(从小到大)
		//SortCards_src(psrc, n, true, true);
		//////从n组里面选取任意2组的组合数C(n,2) //////
		/*c = */fnC.FuncC(n, 2, vec);
		//for (int i = 0; i < n; ++i) {
		//	PrintCardList(psrc[i][0], get_card_c(psrc[i][0], 4));
		//}
		//printf("\n--- *** ------------------- C(%d,%d)=%d\n", n, 2, c);
		for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			assert(2 == it->size());
			//printf("\n--- *** start\n");
#if 0
			for (std::vector<int>::const_iterator ir = it->begin();
				ir != it->end(); ++ir) {
				printf("%d", *ir);
			}
			printf("\n---\n");
#endif
			//PrintCardList(psrc[(*it)[0]][0], get_card_c(psrc[(*it)[0]][0], 4));
			//PrintCardList(psrc[(*it)[1]][0], get_card_c(psrc[(*it)[1]][0], 4));
			//printf("---\n");
			//////从选取的2组中一组取三张，另一组取二张 //////
			int c0 = get_card_c(psrc[(*it)[0]][0], 4);
			int c1 = get_card_c(psrc[(*it)[1]][0], 4);
			if (c0 >= 3) {
				//第0组取三张C(c0,3)，第1组取二张C(c1,2)
				std::vector<std::vector<uint8_t>> v0, v1;
				//psrc[(*it)[0]]组的牌值小于psrc[(*it)[1]]组
				int c_0 = FuncC(c0, 3, psrc, (*it)[0], v0);
				int c_1 = FuncC(c1, 2, psrc, (*it)[1], v1);
				//printf("\n--- C(%d,3)=%d，C(%d,2)=%d\n", c0, c_0, c1, c_1);
				//////从v1,v0中分别任取一项组合葫芦牌型C(v0.size(),1)*C(v1.size(),1) //////
				for (std::vector<std::vector<uint8_t>>::const_iterator it2 = v1.begin();
					it2 != v1.end(); ++it2) {//对子
					for (std::vector<std::vector<uint8_t>>::const_iterator it3 = v0.begin();
						it3 != v0.end(); ++it3) {//三张
#if 0
						std::vector<uint8_t> v(&it2->front(), &it2->front() + it2->size());
						v.resize(v.size() + it3->size());
						memcpy(&v.front() + v.size(), &it3->front(), it3->size());
#else
						//std::vector<uint8_t> v;
						//for (std::vector<uint8_t>::const_iterator ir = it2->begin(); ir != it2->end(); ++ir) {
						//	v.push_back(*ir);
						//}
						//for (std::vector<uint8_t>::const_iterator ir = it3->begin(); ir != it3->end(); ++ir) {
						//	v.push_back(*ir);
						//}
						bool exist = false;
						uint8_t cc[5] = { 0 };
						memcpy(&cc, &it2->front(), it2->size());
						memcpy(&cc[it2->size()], &it3->front(), it3->size());
						for (int i = 0; i < c; ++i) {
							if (0 == CompareCardPointBy(&(src[i])[0], 5, cc, 5/*, false*/)) {
								exist = true;
								break;
							}
						}
						if (!exist) {
							assert(c < MaxSZ);
							assert(it2->size() + it3->size() == 5);
							memcpy(&(src[c])[0], &it2->front(), it2->size());
							memcpy(&(src[c++])[it2->size()], &it3->front(), it3->size());
						}
#endif
						//PrintCardList(&v.front(), v.size());
						//dst.push_back(v);
					}
				}
			}
			if (c1 >= 3) {
				//第0组取二张C(c0,2)，第1组取三张C(c1,3)
				std::vector<std::vector<uint8_t>> v0, v1;
				//psrc[(*it)[0]]组的牌值小于psrc[(*it)[1]]组
				int c_0 = FuncC(c0, 2, psrc, (*it)[0], v0);
				int c_1 = FuncC(c1, 3, psrc, (*it)[1], v1);
				//printf("\n--- C(%d,2)=%d，C(%d,3)=%d\n", c0, c_0, c1, c_1);
				//////从v1,v0中分别任取一项组合葫芦牌型C(v0.size(),1)*C(v1.size(),1) //////
				for (std::vector<std::vector<uint8_t>>::const_iterator it2 = v1.begin();
					it2 != v1.end(); ++it2) {//三张
					for (std::vector<std::vector<uint8_t>>::const_iterator it3 = v0.begin();
						it3 != v0.end(); ++it3) {//对子
#if 0
						std::vector<uint8_t> v(&it2->front(), &it2->front() + it2->size());
						v.resize(v.size() + it3->size());
						memcpy(&v.front() + v.size(), &it3->front(), it3->size());
#else
						//std::vector<uint8_t> v;
						//for (std::vector<uint8_t>::const_iterator ir = it2->begin(); ir != it2->end(); ++ir) {
						//	v.push_back(*ir);
						//}
						//for (std::vector<uint8_t>::const_iterator ir = it3->begin(); ir != it3->end(); ++ir) {
						//	v.push_back(*ir);
						//}
						bool exist = false;
						uint8_t cc[5] = { 0 };
						memcpy(&cc, &it2->front(), it2->size());
						memcpy(&cc[it2->size()], &it3->front(), it3->size());
						for (int i = 0; i < c; ++i) {
							if (0 == CompareCardPointBy(&(src[i])[0], 5, cc, 5/*, false*/)) {
								exist = true;
								break;
							}
						}
						if (!exist) {
							assert(c < MaxSZ);
							assert(it2->size() + it3->size() == 5);
							memcpy(&(src[c])[0], &it2->front(), it2->size());
							memcpy(&(src[c++])[it2->size()], &it3->front(), it3->size());
						}
#endif
						//PrintCardList(&v.front(), v.size());
						//dst.push_back(v);
					}
				}
			}
			//printf("\n--- *** end c=%d\n", dst.size());
		}
		{
			n = c;
			//uint8_t(*src[6])[4] = { 0 };
			typedef uint8_t(*Ptr)[5];
			Ptr psrc[MaxSZ] = { 0 };
			c = 0;
			for (int i = 0; i < n; ++i) {
				assert(c < MaxSZ);
				psrc[c++] = &src[i];
			}
			//葫芦牌型组牌规则，三张尽量取最大(不同的手牌，三张决定了葫芦之间的大小)，对子取值越小，对其余组牌干扰越小
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src32(psrc, n, false);
			for (int i = 0; i < n; ++i) {
				std::vector<uint8_t> v(&(*psrc[i])[0], &(*psrc[i])[0] + 5);
				dst.push_back(v);
			}
		}
		return dst.size();
	}
	
	//三条之间比大小
	static bool As30byPointG(const uint8_t(*const a)[3], const uint8_t(*const b)[3]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 > p1;
	}
	//三条之间比大小
	static bool As30byPointL(const uint8_t(*const a)[3], const uint8_t(*const b)[3]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 < p1;
	}
	//三条之间比大小
	static void SortCardsByPoint_src30(uint8_t(**const psrc)[3], int n, bool ascend) {
		if (ascend) {
			std::sort(psrc, psrc + n, As30byPointL);
		}
		else {
			std::sort(psrc, psrc + n, As30byPointG);
		}
	}

	//枚举所有三条(三张值相同的牌)
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
	//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
	//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
	//dst std::vector<std::vector<uint8_t>>& 存放所有三条牌型
	int CGameLogic::EnumCards30(
		uint8_t(**const psrc)[4],
		uint8_t(*const src4)[4], int const c4,
		uint8_t(*const src3)[4], int const c3,
		uint8_t(*const src2)[4], int const c2,
		std::vector<std::vector<uint8_t>>& dst) {
		//printf("--- *** 枚举所有三条\n");
		int c = 0;
		static int const MaxSZ = 500;
		uint8_t src[MaxSZ][3] = { 0 };
		int n = c4 + c3 + c2;
		CFuncC fnC;
		std::vector<std::vector<int>> vec;
		//psrc组与组之间按牌值升序排列(从小到大)
		//SortCards_src(psrc, n, true, true);
		//////从n组里面选取任意1组的组合数C(n,1) //////
		/*c = */fnC.FuncC(n, 1, vec);
		//for (int i = 0; i < n; ++i) {
		//	PrintCardList(psrc[i][0], get_card_c(psrc[i][0], 4));
		//}
		//printf("\n--- *** ------------------- C(%d,%d)=%d\n", n, 1, c);
		for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			assert(1 == it->size());
			//printf("\n--- *** start\n");
#if 0
			for (std::vector<int>::const_iterator ir = it->begin();
				ir != it->end(); ++ir) {
				printf("%d", *ir);
			}
			printf("\n---\n");
#endif
			//PrintCardList(psrc[(*it)[0]][0], get_card_c(psrc[(*it)[0]][0], 4));
			//PrintCardList(psrc[(*it)[1]][0], get_card_c(psrc[(*it)[1]][0], 4));
			//printf("---\n");
			//////从选取的组中取三张 //////
			int c0 = get_card_c(psrc[(*it)[0]][0], 4);
			if (c0 >= 3) {
				//取三张C(c0,3)
				std::vector<std::vector<uint8_t>> v0;
				int c_0 = FuncC(c0, 3, psrc, (*it)[0], v0);
				//printf("\n--- C(%d,3)=%d\n", c0, c_0);
				//////从v0中任取一项组合三条牌型C(v0.size(),1) //////
				for (std::vector<std::vector<uint8_t>>::const_iterator it3 = v0.begin();
					it3 != v0.end(); ++it3) {//三张
#if 0
					std::vector<uint8_t> v(&it3->front(), &it3->front() + it3->size());
#else
					//std::vector<uint8_t> v;
					//for (std::vector<uint8_t>::const_iterator ir = it3->begin(); ir != it3->end(); ++ir) {
					//	v.push_back(*ir);
					//}
					bool exist = false;
					for (int i = 0; i < c; ++i) {
						if (0 == CompareCardPointBy(&(src[i])[0], 3, &it3->front(), 3/*, false*/)) {
							exist = true;
							break;
						}
					}
					if (!exist) {
						assert(c < MaxSZ);
						assert(it3->size() == 3);
						memcpy(&(src[c++])[0], &it3->front(), it3->size());
					}
#endif
					//PrintCardList(&v.front(), v.size());
					//dst.push_back(v);
				}
			}
			//printf("\n--- *** end c=%d\n", dst.size());
		}
		{
			n = c;
			//uint8_t(*src[6])[4] = { 0 };
			typedef uint8_t(*Ptr)[3];
			Ptr psrc[MaxSZ] = { 0 };
			c = 0;
			for (int i = 0; i < n; ++i) {
				assert(c < MaxSZ);
				psrc[c++] = &src[i];
			}
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src30(psrc, n, false);
			for (int i = 0; i < n; ++i) {
				std::vector<uint8_t> v(&(*psrc[i])[0], &(*psrc[i])[0] + 3);
				dst.push_back(v);
			}
		}
		return dst.size();
	}
	
	//两对之间比大小
	static bool As22byPointG(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t s0, p0;
		if (CGameLogic::GetCardValue(a[0][0]) == A) {
			s0 = CGameLogic::GetCardPoint(a[0][0]);
			p0 = CGameLogic::GetCardPoint(a[0][3]);
		}
		else {
			s0 = CGameLogic::GetCardPoint(a[0][3]);
			p0 = CGameLogic::GetCardPoint(a[0][0]);
		}
		uint8_t s1, p1;
		if (CGameLogic::GetCardValue(b[0][0]) == A) {
			s1 = CGameLogic::GetCardPoint(b[0][0]);
			p1 = CGameLogic::GetCardPoint(b[0][3]);
		}
		else {
			s1 = CGameLogic::GetCardPoint(b[0][3]);
			p1 = CGameLogic::GetCardPoint(b[0][0]);
		}
		if (s0 != s1) {
			//比较最大的对子
			return s0 > s1;
		}
		//比较次大的对子，次大对子取值越小，对其余组牌干扰越小
		return p0 >/*<*/ p1;
	}
	//两对之间比大小
	static bool As22byPointL(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t s0, p0;
		if (CGameLogic::GetCardValue(a[0][0]) == A) {
			s0 = CGameLogic::GetCardPoint(a[0][0]);
			p0 = CGameLogic::GetCardPoint(a[0][3]);
		}
		else {
			s0 = CGameLogic::GetCardPoint(a[0][3]);
			p0 = CGameLogic::GetCardPoint(a[0][0]);
		}
		uint8_t s1, p1;
		if (CGameLogic::GetCardValue(b[0][0]) == A) {
			s1 = CGameLogic::GetCardPoint(b[0][0]);
			p1 = CGameLogic::GetCardPoint(b[0][3]);
		}
		else {
			s1 = CGameLogic::GetCardPoint(b[0][3]);
			p1 = CGameLogic::GetCardPoint(b[0][0]);
		}
		if (s0 != s1) {
			//比较最大的对子
			return s0 < s1;
		}
		//比较次大的对子，次大对子取值越小，对其余组牌干扰越小
		return p0 </*>*/ p1;
	}
	//两对之间比大小
	static void SortCardsByPoint_src22(uint8_t(**const psrc)[4], int n, bool ascend) {
		if (ascend) {
			std::sort(psrc, psrc + n, As22byPointL);
		}
		else {
			std::sort(psrc, psrc + n, As22byPointG);
		}
	}

	//枚举所有两对(两个对子加上一张单牌)
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
	//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
	//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
	//dst std::vector<std::vector<uint8_t>>& 存放所有两对牌型
	int CGameLogic::EnumCards22(
		uint8_t(**const psrc)[4],
		uint8_t(*const src4)[4], int const c4,
		uint8_t(*const src3)[4], int const c3,
		uint8_t(*const src2)[4], int const c2,
		std::vector<std::vector<uint8_t>>& dst) {
		//printf("--- *** 枚举所有两对\n");
		int c = 0;
		static int const MaxSZ = 500;
		uint8_t src[MaxSZ][4] = { 0 };
		int n = c4 + c3 + c2;
		CFuncC fnC;
		std::vector<std::vector<int>> vec;
		//psrc组与组之间按牌值升序排列(从小到大)
		//SortCards_src(psrc, n, true, true);
		//////从n组里面选取任意2组的组合数C(n,2) //////
		/*int c = */fnC.FuncC(n, 2, vec);
		//for (int i = 0; i < n; ++i) {
		//	PrintCardList(psrc[i][0], get_card_c(psrc[i][0], 4));
		//}
		//printf("\n--- *** ------------------- C(%d,%d)=%d\n", n, 2, c);
		for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			assert(2 == it->size());
			//printf("\n--- *** start\n");
#if 0
			for (std::vector<int>::const_iterator ir = it->begin();
				ir != it->end(); ++ir) {
				printf("%d", *ir);
			}
			printf("\n---\n");
#endif
			//PrintCardList(psrc[(*it)[0]][0], get_card_c(psrc[(*it)[0]][0], 4));
			//PrintCardList(psrc[(*it)[1]][0], get_card_c(psrc[(*it)[1]][0], 4));
			//printf("---\n");
			//////从选取的2组中一组取二张，另一组取二张 //////
			int c0 = get_card_c(psrc[(*it)[0]][0], 4);
			int c1 = get_card_c(psrc[(*it)[1]][0], 4);
			assert(c0 >= 2 && c1 >= 2);
			//第0组取二张C(c0,2)，第1组取二张C(c1,2)
			std::vector<std::vector<uint8_t>> v0, v1;
			int c_0 = FuncC(c0, 2, psrc, (*it)[0], v0);
			int c_1 = FuncC(c1, 2, psrc, (*it)[1], v1);
			//printf("\n--- C(%d,2)=%d，C(%d,2)=%d\n", c0, c_0, c1, c_1);
			//////从v1,v0中分别任取一项组合两对子牌型C(v0.size(),1)*C(v1.size(),1) //////
			for (std::vector<std::vector<uint8_t>>::const_iterator it2 = v1.begin();
				it2 != v1.end(); ++it2) {//对子
				for (std::vector<std::vector<uint8_t>>::const_iterator it3 = v0.begin();
					it3 != v0.end(); ++it3) {//对子
#if 0
					std::vector<uint8_t> v(&it2->front(), &it2->front() + it2->size());
					v.resize(v.size() + it3->size());
					memcpy(&v.front() + v.size(), &it3->front(), it3->size());
#else
					//std::vector<uint8_t> v;
					//for (std::vector<uint8_t>::const_iterator ir = it2->begin(); ir != it2->end(); ++ir) {
					//	v.push_back(*ir);
					//}
					//for (std::vector<uint8_t>::const_iterator ir = it3->begin(); ir != it3->end(); ++ir) {
					//	v.push_back(*ir);
					//}
					bool exist = false;
					uint8_t cc[4] = { 0 };
					memcpy(&cc, &it2->front(), it2->size());
					memcpy(&cc[it2->size()], &it3->front(), it3->size());
					for (int i = 0; i < c; ++i) {
						if (0 == CompareCardPointBy(&(src[i])[0], 4, cc, 4/*, false*/)) {
							exist = true;
							break;
						}
					}
					if (!exist) {
						assert(c < MaxSZ);
						assert(it2->size() == 2 && it3->size() == 2);
						memcpy(&(src[c])[0], &it2->front(), it2->size());
						memcpy(&(src[c++])[it2->size()], &it3->front(), it3->size());
					}
#endif
					//PrintCardList(&v.front(), v.size());
					//dst.push_back(v);
				}
			}
			//printf("\n--- *** end c=%d\n", dst.size());
		}
		{
			n = c;
			//uint8_t(*src[6])[4] = { 0 };
			typedef uint8_t(*Ptr)[4];
			Ptr psrc[MaxSZ] = { 0 };
			c = 0;
			for (int i = 0; i < n; ++i) {
				assert(c < MaxSZ);
				psrc[c++] = &src[i];
			}
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src22(psrc, n, false);
			for (int i = 0; i < n; ++i) {
				std::vector<uint8_t> v(&(*psrc[i])[0], &(*psrc[i])[0] + 4);
				dst.push_back(v);
			}
		}
		return dst.size();
	}
	
	//对子之间比大小
	static bool As20byPointG(const uint8_t(*const a)[2], const uint8_t(*const b)[2]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 > p1;
	}
	//对子之间比大小
	static bool As20byPointL(const uint8_t(*const a)[2], const uint8_t(*const b)[2]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 < p1;
	}
	//对子之间比大小
	static void SortCardsByPoint_src20(uint8_t(**const psrc)[2], int n, bool ascend) {
		if (ascend) {
			std::sort(psrc, psrc + n, As20byPointL);
		}
		else {
			std::sort(psrc, psrc + n, As20byPointG);
		}
	}

	//枚举所有对子(一对)
	//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
	//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
	//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
	//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
	//dst std::vector<std::vector<uint8_t>>& 存放所有对子牌型
	int CGameLogic::EnumCards20(
		uint8_t(**const psrc)[4],
		uint8_t(*const src4)[4], int const c4,
		uint8_t(*const src3)[4], int const c3,
		uint8_t(*const src2)[4], int const c2,
		std::vector<std::vector<uint8_t>>& dst) {
		//printf("--- *** 枚举所有对子\n");
		int c = 0;
		static int const MaxSZ = 50;
		uint8_t src[MaxSZ][2] = { 0 };
		int n = c4 + c3 + c2;
		CFuncC fnC;
		std::vector<std::vector<int>> vec;
		//psrc组与组之间按牌值升序排列(从小到大)
		//SortCards_src(psrc, n, true, true);
		//////从n组里面选取任意1组的组合数C(n,1) //////
		/*int c = */fnC.FuncC(n, 1, vec);
		//for (int i = 0; i < n; ++i) {
		//	PrintCardList(psrc[i][0], get_card_c(psrc[i][0], 4));
		//}
		//printf("\n--- *** ------------------- C(%d,%d)=%d\n", n, 1, c);
		for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			assert(1 == it->size());
			//printf("\n--- *** start\n");
#if 0
			for (std::vector<int>::const_iterator ir = it->begin();
				ir != it->end(); ++ir) {
				printf("%d", *ir);
			}
			printf("\n---\n");
#endif
			//PrintCardList(psrc[(*it)[0]][0], get_card_c(psrc[(*it)[0]][0], 4));
			//PrintCardList(psrc[(*it)[1]][0], get_card_c(psrc[(*it)[1]][0], 4));
			//printf("---\n");
			//////从选取的组中取二张 //////
			int c0 = get_card_c(psrc[(*it)[0]][0], 4);
			assert(c0 >= 2);
			//取二张C(c0,2)
			std::vector<std::vector<uint8_t>> v0;
			int c_0 = FuncC(c0, 2, psrc, (*it)[0], v0);
			//printf("\n--- C(%d,2)=%d\n", c0, c_0);
			//////从v0中任取一项组合对子牌型C(v0.size(),1) //////
			for (std::vector<std::vector<uint8_t>>::const_iterator it3 = v0.begin();
				it3 != v0.end(); ++it3) {//二张
#if 0
				std::vector<uint8_t> v(&it3->front(), &it3->front() + it3->size());
#else
// 				std::vector<uint8_t> v;
// 				for (std::vector<uint8_t>::const_iterator ir = it3->begin(); ir != it3->end(); ++ir) {
// 					v.push_back(*ir);
// 				}
				bool exist = false;
				for (int i = 0; i < c; ++i) {
					if (0 == CompareCardPointBy(&(src[i])[0], 2, &it3->front(), 2/*, false*/)) {
						exist = true;
						break;
					}
				}
				if (!exist) {
					assert(c < MaxSZ);
					assert(it3->size() == 2);
					memcpy(&(src[c++])[0], &it3->front(), it3->size());
				}
#endif
				//PrintCardList(&v.front(), v.size());
				//dst.push_back(v);
			}
			//printf("\n--- *** end c=%d\n", dst.size());
		}
		{
			n = c;
			//uint8_t(*src[6])[4] = { 0 };
			typedef uint8_t(*Ptr)[2];
			Ptr psrc[MaxSZ] = { 0 };
			c = 0;
			for (int i = 0; i < n; ++i) {
				assert(c < MaxSZ);
				psrc[c++] = &src[i];
			}
			//psrc组与组之间按牌值降序排列(从大到小)
			SortCardsByPoint_src20(psrc, n, false);
			for (int i = 0; i < n; ++i) {
				std::vector<uint8_t> v(&(*psrc[i])[0], &(*psrc[i])[0] + 2);
				dst.push_back(v);
			}
		}
		return dst.size();
	}
	
	//铁支之间比大小
	static bool As40byPointG(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 > p1;
	}
	//铁支之间比大小
	static bool As40byPointL(const uint8_t(*const a)[4], const uint8_t(*const b)[4]) {
		uint8_t p0 = CGameLogic::GetCardPoint(a[0][0]);
		uint8_t p1 = CGameLogic::GetCardPoint(b[0][0]);
		return p0 < p1;
	}
	//铁支之间比大小
	static void SortCardsByPoint_src40(uint8_t(**const psrc)[4], int n, bool ascend) {
		if (ascend) {
			std::sort(psrc, psrc + n, As40byPointL);
		}
		else {
			std::sort(psrc, psrc + n, As40byPointG);
		}
	}

	//按照尾墩5张/中墩5张/头墩3张依次抽取枚举普通牌型
	//src uint8_t const* 手牌余牌(13/8/3)，初始13张，按5/5/3依次抽，余牌依次为13/8/3
	//n int 抽取n张(5/5/3) 第一次抽5张余8张，第二次抽5张余3张，第三次取余下3张抽完
	//dst4 uint8_t(*)[4] 存放所有四张牌型，c4 四张牌型数
	//dst3 uint8_t(*)[4] 存放所有三张牌型，c3 三张牌型数
	//dst2 uint8_t(*)[4] 存放所有对子牌型，c2 对子牌型数
	//cpy uint8_t* 存放去重后的余牌/散牌(除去四张/三张/对子)
	//v123sc std::vector<std::vector<uint8_t>>& 存放所有同花色n张连续牌
	//v123 std::vector<std::vector<uint8_t>>& 存放所有非同花n张连续牌
	//vsc std::vector<std::vector<uint8_t>>& 存放所有同花n张非连续牌
	//v40 std::vector<std::vector<uint8_t>>& 存放所有铁支
	//v32 std::vector<std::vector<uint8_t>>& 存放所有葫芦
	//v30 std::vector<std::vector<uint8_t>>& 存放所有三条
	//v22 std::vector<std::vector<uint8_t>>& 存放所有两对
	//v20 std::vector<std::vector<uint8_t>>& 存放所有对子
	void CGameLogic::EnumCards(uint8_t const* src, int len, int n,
		uint8_t(*dst4)[4], int& c4,
		uint8_t(*dst3)[4], int& c3,
		uint8_t(*dst2)[4], int& c2,
		uint8_t *cpy, int& cpylen,
		std::vector<std::vector<uint8_t>>& v123sc,
		std::vector<std::vector<uint8_t>>& v123,
		std::vector<std::vector<uint8_t>>& vsc,
		std::vector<std::vector<uint8_t>>& v40,
		std::vector<std::vector<uint8_t>>& v32,
		std::vector<std::vector<uint8_t>>& v30,
		std::vector<std::vector<uint8_t>>& v22,
		std::vector<std::vector<uint8_t>>& v20) {
		//合并dst4/dst3/dst2到psrc然后排序
		//uint8_t(*ptr[6])[4] = { 0 };
		typedef uint8_t(*Ptr)[4];
		Ptr psrc[6] = { 0 };
		//所有同花色n张连续牌
		v123sc.clear();
		//所有非同花n张连续牌
		v123.clear();
		//所有同花n张非连续牌
		vsc.clear();
		//简单牌型分类/重复(四张/三张/二张)/同花/顺子/同花顺/散牌
		ClassifyCards(src, len, n, psrc, dst4, c4, dst3, c3, dst2, c2, cpy, cpylen, v123sc, v123, vsc);
		//psrc组与组之间按牌值升序排列(从小到大)
		SortCards_src(psrc, c4 + c3 + c2, true, true);
		if (n >= 5) {
			//所有葫芦(一组三条加上一组对子)
			v32.clear();
			EnumCards32(psrc, dst4, c4, dst3, c3, dst2, c2, v32);
		}
		if (n >= 3) {
			//所有三条(三张值相同的牌)
			v30.clear();
			EnumCards30(psrc, dst4, c4, dst3, c3, dst2, c2, v30);
		}
		if (n >= 4) {
			//所有两对(两个对子加上一张单牌)
			v22.clear();
			EnumCards22(psrc, dst4, c4, dst3, c3, dst2, c2, v22);
		}
		if (n >= 2) {
			//所有对子(一对)
			v20.clear();
			EnumCards20(psrc, dst4, c4, dst3, c3, dst2, c2, v20);
		}
		if (n >= 4) {
			//所有铁支(四张值相同的牌)
			v40.clear();
			{
				typedef uint8_t(*Ptr)[4];
				Ptr pdst[MaxSZ] = { 0 };
				int c = 0;
				for (int i = 0; i < c4; ++i) {
					assert(c < MaxSZ);
					pdst[c++] = &dst4[i];
				}
				//psrc组与组之间按牌值降序排列(从大到小)
				SortCardsByPoint_src40(pdst, c4, false);
				for (int i = 0; i < c4; ++i) {
					std::vector<uint8_t> v(&(*pdst[i])[0], &(*pdst[i])[0] + 4);
					v40.push_back(v);
				}
			}
			//for (int i = c4 - 1; i >= 0; --i) {
			//	//std::vector<uint8_t> v(&(*&dst4[i])[0], &(*&dst4[i])[0] + 4);
			//	std::vector<uint8_t> v(&(dst4[i])[0], &(dst4[i])[0] + 4);
			//	v40.push_back(v);
			//}
		}
	}

	void CGameLogic::classify_t::PrintCardList() {
		printf("\n\n\n");
		for (int i = 0; i < c4; ++i) {
			CGameLogic::PrintCardList(&(dst4[i])[0], 4);
		}
		for (int i = 0; i < c3; ++i) {
			CGameLogic::PrintCardList(&(dst3[i])[0], 3);
		}
		for (int i = 0; i < c2; ++i) {
			CGameLogic::PrintCardList(&(dst2[i])[0], 2);
		}
		printf("---\n");
		CGameLogic::PrintCardList(cpy, cpylen);
		printf("\n\n");
	}

	//按照尾墩5张/中墩5张/头墩3张依次抽取枚举普通牌型
	//src uint8_t const* 手牌余牌(13/8/3)，初始13张，按5/5/3依次抽，余牌依次为13/8/3
	//n int 抽取n张(5/5/3) 第一次抽5张余8张，第二次抽5张余3张，第三次取余下3张抽完
	//classify classify_t& 存放分类信息(所有重复四张/三张/二张/散牌/余牌)
	//enumList EnumTree& 存放枚举墩牌型列表数据
	void CGameLogic::EnumCards(uint8_t const* src, int len,
		int n, classify_t& classify, EnumTree& enumList) {
		//printf("\n\n--- *** EnumCards(%d, %d) from ", len, n);
		//PrintCardList(src, len);
		//int c4 = 0, c3 = 0, c2 = 0;
		//所有重复四张牌型
		//static int const size4 = 3;
		//uint8_t dst4[size4][4] = { 0 };
		//所有重复三张牌型
		//static int const size3 = 4;
		//uint8_t dst3[size3][4] = { 0 };
		//所有重复二张牌型
		//static int const size2 = 6;
		//uint8_t dst2[size2][4] = { 0 };
		//去重后的余牌/散牌
		//uint8_t cpy[MaxSZ] = { 0 };
		//int cpylen = 0;
		//memset(&classify, 0, sizeof(classify_t));
		enumList.Reset();
		//枚举墩牌型
		EnumCards(src, len, n, 
			classify.dst4, classify.c4, classify.dst3, classify.c3, classify.dst2, classify.c2, classify.cpy, classify.cpylen,
			enumList.v123sc, enumList.v123, enumList.vsc, enumList.v40, enumList.v32, enumList.v30, enumList.v22, enumList.v20);
	}

	//牌型判断，算法入口 /////////
	//src uint8_t const* 牌源
	//hand handinfo_t& 保存手牌信息
	HandTy CGameLogic::AnalyseCards(uint8_t const* src, int len, handinfo_t& hand, bool bv) {
		//printf("--- *** 待分析手牌如下\n");
		//PrintCardList(src, len);
		assert(len > 0);
		uint8_t psrc[len];// = { 0 };
		memcpy(psrc, src, len);
		//牌源按牌值从小到大排序(A23...QK)
		SortCards(psrc, len, true, true, true);
		hand.Reset();
		classify_t classify = { 0 };
		EnumTree enumList;
		EnumCards(psrc, len, std::min(len, 5), classify, enumList);
		if (enumList.v123sc.size() > 0 && (std::min(len, 5) == 5 || (bv && std::min(len, 5) == 4))) {
			if (len == 4) {
				hand.ty_ = Ty123sc;//同花顺
				EnumTree::CardData& card = enumList.v123sc[0];
				std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
			}
			else {
				EnumTree::CardData& card = enumList.v123sc[0];
				if (GetCardPoint(card[0]) == 10 && GetCardPoint(card[4]) == K + 1) {
					hand.ty_ = Ty123scRoyal;//皇家同花顺
				}
				else {
					hand.ty_ = Ty123sc;//同花顺
				}
				std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
			}
		}
		else if (enumList.v40.size() > 0) {
			hand.ty_ = Ty40;//铁支
			EnumTree::CardData& card = enumList.v40[0];
			std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
		}
		else if (enumList.v32.size() > 0) {
			//查看所有枚举牌型
			//enumList.PrintEnumCards(false, TyAll);
			hand.ty_ = Ty32;//葫芦
			EnumTree::CardData& card = enumList.v32[0];
			std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
		}
		else if (enumList.vsc.size() > 0 && (std::min(len, 5) == 5 || (bv && std::min(len, 5) == 4))) {
			if (len == 4) {
				hand.ty_ = Tysc;//同花
				EnumTree::CardData& card = enumList.vsc[0];
				std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
			}
			else {
				hand.ty_ = Tysc;//同花
				EnumTree::CardData& card = enumList.vsc[0];
				std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
			}
		}
		else if (enumList.v123.size() > 0 && (std::min(len, 5) == 5 || (bv && std::min(len, 5) == 4))) {
			if (len == 4) {
				hand.ty_ = Ty123;//顺子
				EnumTree::CardData& card = enumList.v123[0];
				std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
			}
			else {
				hand.ty_ = Ty123;//顺子
				EnumTree::CardData& card = enumList.v123[0];
				std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
			}
		}
		else if (enumList.v30.size() > 0) {
			hand.ty_ = Ty30;//三条
			EnumTree::CardData& card = enumList.v30[0];
			std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
		}
		else if (enumList.v22.size() > 0) {
			//查看所有枚举牌型
			//enumList.PrintEnumCards(false, TyAll);
			hand.ty_ = Ty22;//两对
			EnumTree::CardData& card = enumList.v22[0];
			std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
		}
		else if (enumList.v20.size() > 0) {
			hand.ty_ = Ty20;//对子
			EnumTree::CardData& card = enumList.v20[0];
			std::copy(card.begin(), card.end(), std::back_inserter(hand.cards_));
		}
		else {
			hand.ty_ = Tysp;//散牌
			//余牌按点数从小到大排序(23...QKA)
			SortCards(classify.cpy, classify.cpylen, false, true, true);
			assert(classify.cpylen == len);
			hand.cards_.resize(len);
			memcpy(&hand.cards_.front(), &classify.cpy[0], classify.cpylen);
		}
		return hand.ty_;
	}

	int CGameLogic::CompareCards(uint8_t* src, uint8_t* dst, int len) {
		handinfo_t h0, h1;
		AnalyseCards(src, len, h0);
		AnalyseCards(dst, len, h1);
		if (h0.ty_ != h1.ty_) {
			//牌型不同比牌型
			return h0.ty_ - h1.ty_;
		}
		assert(h0.cards_.size() > 0);
		assert(h1.cards_.size() > 0);
		//牌型相同比大小
		return CompareCards(
			&h0.cards_[0], h0.cards_.size(),
			&h1.cards_[0], h1.cards_.size(), h0.ty_);
	}

	int CGameLogic::CompareCards(handinfo_t const& src, handinfo_t const& dst) {
		if (src.ty_ != dst.ty_) {
			//牌型不同比牌型
			return src.ty_ - dst.ty_;
		}
		assert(src.cards_.size() > 0);
		assert(dst.cards_.size() > 0);
		//牌型相同比大小
		return CompareCards(
			&src.cards_[0], src.cards_.size(),
			&dst.cards_[0], dst.cards_.size(), src.ty_);
	}

	//牌型相同按牌点从大到小顺序逐次比点
	//src uint8_t const* srcLen张牌
	//dst uint8_t const* dstLen张牌
	//clr bool 是否比花色
	int CGameLogic::CompareCardPointBy(
		uint8_t const* src, int srcLen,
		uint8_t const* dst, int dstLen) {
		uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
		assert(srcLen > 0);
		assert(dstLen > 0);
		memcpy(psrc, src, srcLen);
		memcpy(pdst, dst, dstLen);
		SortCards(psrc, srcLen, false, true, true);
		SortCards(pdst, dstLen, false, true, true);
		//牌型相同按点数从大到小顺序比点数
		int i = srcLen - 1, j = dstLen - 1;
		while(i >= 0 && j >= 0) {
			uint8_t p0 = CGameLogic::GetCardPoint(psrc[i--]);
			uint8_t p1 = CGameLogic::GetCardPoint(pdst[j--]);
			if (p0 != p1) {
				return p0 - p1;
			}
		}
		{
			//按点数从大到小顺序比花色
			int i = srcLen - 1, j = dstLen - 1;
			while (i >= 0 && j >= 0) {
				uint8_t c0 = CGameLogic::GetCardColor(src[i--]);
				uint8_t c1 = CGameLogic::GetCardColor(dst[j--]);
				if (c0 != c1) {
					return c0 - c1;
				}
			}
		}
		return 0;
	}
	
	//牌型相同的src与dst两墩之间比大小
	//src uint8_t const* srcLen张牌
	//dst uint8_t const* dstLen张牌
	//clr bool 是否比花色
	//ty HandTy 比较的两单墩牌的普通牌型
	//sp bool 是否比较散牌/单张
	//sd bool 是否比较次大的对子(两对之间比较)
	//dz bool 是否比较葫芦的对子(葫芦之间比较)
	int CGameLogic::CompareCards(
		uint8_t const* src, int srcLen,
		uint8_t const* dst, int dstLen, HandTy ty) {
		switch (ty) {
		//同花顺之间比较
		case Ty123scRoyal:
		case Ty123sc: return CompareCardPointBy(src, srcLen, dst, dstLen);
		//同花之间比较
		case Tysc: return CompareCardPointBy(src, srcLen, dst, dstLen);
		//顺子之间比较
		case Ty123: return CompareCardPointBy(src, srcLen, dst, dstLen);
		//散牌之间比较
		case Tysp: return CompareCardPointBy(src, srcLen, dst, dstLen);
		//铁支之间比较
		case Ty40: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			CGameLogic::SortCards(psrc, srcLen, true, true, true);
			CGameLogic::SortCards(pdst, dstLen, true, true, true);
			//printf("铁支之间比较\n");
			//PrintCardList(psrc, srcLen);
			//PrintCardList(pdst, dstLen);
			int c4_0 = 0, cpylen_0 = 0;
			uint8_t dst4_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size4 = 3;
				c4_0 = CGameLogic::EnumRepeatCards(psrc, srcLen, 4, dst4_0, size4, cpy_0, cpylen_0);
				if (c4_0 != 1) {
					CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c4_0 == 1);
			}
			int c4_1 = 0, cpylen_1 = 0;
			uint8_t dst4_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size4 = 3;
				c4_1 = CGameLogic::EnumRepeatCards(pdst, dstLen, 4, dst4_1, size4, cpy_1, cpylen_1);
				if (c4_1 != 1) {
					CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c4_1 == 1);
			}
			uint8_t p0 = CGameLogic::GetCardPoint(dst4_0[0][0]);
			uint8_t p1 = CGameLogic::GetCardPoint(dst4_1[0][0]);
			//printf("铁支之间比较 比较四张点数\n");
			//PrintCardList(&dst4_0[0][0], 3);
			//PrintCardList(&dst4_1[0][0], 3);
			//比较四张点数
			//assert(p0 != p1);
			return p0 - p1;
		}
		//葫芦之间比较
		case Ty32: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			CGameLogic::SortCards(psrc, srcLen, true, true, true);
			CGameLogic::SortCards(pdst, dstLen, true, true, true);
			//printf("葫芦之间比较\n");
			//PrintCardList(psrc, srcLen);
			//PrintCardList(pdst, dstLen);
			int c3_0 = 0, cpylen_0 = 0;
			uint8_t dst3_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_0 = CGameLogic::EnumRepeatCards(psrc, srcLen, 3, dst3_0, size3, cpy_0, cpylen_0);
				if (c3_0 != 1) {
					CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c3_0 == 1);
			}
			int c3_1 = 0, cpylen_1 = 0;
			uint8_t dst3_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_1 = CGameLogic::EnumRepeatCards(pdst, dstLen, 3, dst3_1, size3, cpy_1, cpylen_1);
				if (c3_1 != 1) {
					CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c3_1 == 1);
			}
			uint8_t p0 = CGameLogic::GetCardPoint(dst3_0[0][0]);
			uint8_t p1 = CGameLogic::GetCardPoint(dst3_1[0][0]);
			//printf("葫芦之间比较 比较三张点数\n");
			//PrintCardList(&dst3_0[0][0], 3);
			//PrintCardList(&dst3_1[0][0], 3);
			//比较三张点数
			//assert(p0 != p1);
			return p0 - p1;
		}
		//三条之间比较
		case Ty30: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			CGameLogic::SortCards(psrc, srcLen, true, true, true);
			CGameLogic::SortCards(pdst, dstLen, true, true, true);
			//printf("三条之间比较\n");
			//PrintCardList(psrc, srcLen);
			//PrintCardList(pdst, dstLen);
			int c3_0 = 0, cpylen_0 = 0;
			uint8_t dst3_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_0 = CGameLogic::EnumRepeatCards(psrc, srcLen, 3, dst3_0, size3, cpy_0, cpylen_0);
				if (c3_0 != 1) {
					CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c3_0 == 1);
			}
			int c3_1 = 0, cpylen_1 = 0;
			uint8_t dst3_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_1 = CGameLogic::EnumRepeatCards(pdst, dstLen, 3, dst3_1, size3, cpy_1, cpylen_1);
				if (c3_1 != 1) {
					CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c3_1 == 1);
			}
			uint8_t p0 = CGameLogic::GetCardPoint(dst3_0[0][0]);
			uint8_t p1 = CGameLogic::GetCardPoint(dst3_1[0][0]);
			//printf("三条之间比较 比较三张点数\n");
			//PrintCardList(&dst3_0[0][0], 3);
			//PrintCardList(&dst3_1[0][0], 3);
			//比较三张点数
			//assert(p0 != p1);
			return p0 - p1;
		}
		//两对之间比较 ♦A ♣A ♣8 ♣K ♠8
		case Ty22: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			CGameLogic::SortCards(psrc, srcLen, true, true, true);
			CGameLogic::SortCards(pdst, dstLen, true, true, true);

			int c2_0 = 0, cpylen_0 = 0;
			uint8_t dst2_0[2][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_0 = CGameLogic::EnumRepeatCards(psrc, srcLen, 2, dst2_0, size2, cpy_0, cpylen_0);
				if (c2_0 != 2) {
					CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c2_0 == 2);
			}
			int c2_1 = 0, cpylen_1 = 0;
			uint8_t dst2_1[2][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_1 = CGameLogic::EnumRepeatCards(pdst, dstLen, 2, dst2_1, size2, cpy_1, cpylen_1);
				if (c2_1 != 2) {
					CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c2_1 == 2);
			}
			uint8_t s0, p0, s0_c, p0_c;
			if (CGameLogic::GetCardPoint(dst2_0[0][0]) > CGameLogic::GetCardPoint(dst2_0[1][0])) {
				s0 = CGameLogic::GetCardPoint(dst2_0[0][0]); s0_c = CGameLogic::GetCardColor(dst2_0[0][1]);
				p0 = CGameLogic::GetCardPoint(dst2_0[1][0]); p0_c = CGameLogic::GetCardColor(dst2_0[1][1]);
				
			}
			else {
				s0 = CGameLogic::GetCardPoint(dst2_0[1][0]); s0_c = CGameLogic::GetCardColor(dst2_0[1][1]);
				p0 = CGameLogic::GetCardPoint(dst2_0[0][0]); p0_c = CGameLogic::GetCardColor(dst2_0[0][1]);
			}
			uint8_t s1, p1, s1_c, p1_c;
			if (CGameLogic::GetCardPoint(dst2_1[0][0]) > CGameLogic::GetCardPoint(dst2_1[1][0])) {
				s1 = CGameLogic::GetCardPoint(dst2_1[0][0]); s1_c = CGameLogic::GetCardColor(dst2_1[0][1]);
				p1 = CGameLogic::GetCardPoint(dst2_1[1][0]); p1_c = CGameLogic::GetCardColor(dst2_1[1][1]);
			}
			else {
				s1 = CGameLogic::GetCardPoint(dst2_1[1][0]); s1_c = CGameLogic::GetCardColor(dst2_1[1][1]);
				p1 = CGameLogic::GetCardPoint(dst2_1[0][0]); p1_c = CGameLogic::GetCardColor(dst2_1[0][1]);
			}
			//比较最大对子点数
			if (s0 != s1) {
				return s0 - s1;
			}
			//比较次大对子点数
			if (p0 != p1) {
				return p0 - p1;
			}
			//比较最大对子花色
			return s0_c - s1_c;
		}
		//对子之间比较
		case Ty20: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			CGameLogic::SortCards(psrc, srcLen, true, true, true);
			CGameLogic::SortCards(pdst, dstLen, true, true, true);

			int c2_0 = 0, cpylen_0 = 0;
			uint8_t dst2_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_0 = CGameLogic::EnumRepeatCards(psrc, srcLen, 2, dst2_0, size2, cpy_0, cpylen_0);
				if (c2_0 != 1) {
					CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c2_0 == 1);
			}
			int c2_1 = 0, cpylen_1 = 0;
			uint8_t dst2_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_1 = CGameLogic::EnumRepeatCards(pdst, dstLen, 2, dst2_1, size2, cpy_1, cpylen_1);
				if (c2_1 != 1) {
					CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c2_1 == 1);
			}
			uint8_t p0 = CGameLogic::GetCardPoint(dst2_0[0][0]); uint8_t c0 = CGameLogic::GetCardColor(dst2_0[0][1]);
			uint8_t p1 = CGameLogic::GetCardPoint(dst2_1[0][0]); uint8_t c1 = CGameLogic::GetCardColor(dst2_1[0][1]);
			//比较对子点数
			if (p0 != p1) {
				return p0 - p1;
			}
			//比较对子花色
			return c0 - c1;
		}
		}
		assert(false);
		return 0;
	}

	//枚举牌型测试
	void CGameLogic::TestEnumCards(int size) {
		CGameLogic g;
		//handinfo_t hand;
		//初始化
		g.InitCards();
		//洗牌
		g.ShuffleCards();
		bool pause = false;
		while (1) {
			//if (pause) {
			//	getchar();
			//}
			//余牌不够则重新洗牌
			if (g.Remaining() < 13) {
				g.ShuffleCards();
			}
			uint8_t cards[MAX_COUNT] = { 0 };
			//发牌
			g.DealCards(MAX_COUNT, cards);
			//手牌排序
			CGameLogic::SortCards(cards, MAX_COUNT, true, true, true);
			//printf("=================================================\n\n");
			//一副手牌
			CGameLogic::PrintCardList(cards, MAX_COUNT);
 			//手牌牌型分析
			//int c = CGameLogic::AnalyseCards(cards, MAX_COUNT, size, hand);
			//有特殊牌型时
			//pause = (hand.specialTy_ != Tysp);
			//有三同花顺/三同花/三顺子时
			//pause = ((hand.specialTy_ == SSS::TyThree123) || (hand.specialTy_ == SSS::TyThree123sc));
			//没有重复四张，有2个重复三张和3个重复二张
			//pause = (hand.classify.c4 == 0 &&
			//	hand.classify.c3 >= 2 &&
			//	hand.classify.c2 >= 3);
			//if (pause) {
				//查看所有枚举牌型
				//hand.rootEnumList->PrintEnumCards(false, TyAll);
				//查看手牌特殊牌型
				//hand.PrintSpecCards();
				//查看手牌枚举三墩牌型
				//hand.PrintEnumCards();
				//查看重复牌型和散牌
				//hand.classify.PrintCardList();
				//printf("--- *** c = %d %s\n\n\n\n", c, hand.StringSpecialTy().c_str());
			//}
		}
	}

	//枚举牌型测试
	//filename char const* 文件读取手牌 cardsList.ini
    void CGameLogic::TestEnumCards(char const* filename)
    {
		std::vector<std::string> lines;
		//readFile(filename, lines, ";;");
		assert(lines.size() >= 2);
		//1->文件读取手牌 0->随机生成13张牌
		int flag = atoi(lines[0].c_str());
		//默认最多枚举多少组墩，开元/德胜是3组
		int size = atoi(lines[1].c_str());
		//1->文件读取手牌 0->随机生成13张牌
        if (flag > 0)
        {
			//assert(lines.size() == 3);
			CGameLogic g;
			//handinfo_t hand;
			uint8_t cards[MAX_COUNT + 10] = { 0 };
			//line[2]构造一副手牌13张
			int n = CGameLogic::MakeCardList(lines[2], cards, MAX_COUNT);
			//assert(n == 13);
			//手牌排序
			CGameLogic::SortCards(cards, n, true, true, true);
			printf("=================================================\n\n");
			//一副手牌
			//CGameLogic::PrintCardList(cards, n);
			//手牌牌型分析
			//int c = CGameLogic::AnalyseCards(cards, n, size, hand);
			//查看所有枚举牌型
			//hand.rootEnumList->PrintEnumCards(false, TyAll);
			//查看手牌特殊牌型
			//hand.PrintSpecCards();
			//查看手牌枚举三墩牌型
			//hand.PrintEnumCards(false);
			//查看重复牌型和散牌
			//hand.classify.PrintCardList();
			//printf("--- *** c = %d %s\n\n\n\n", c, hand.StringSpecialTy().c_str());
			//{
			//	HandTy ty;
			//	ty = CGameLogic::GetDunCardHandTy(DunFirst, cards, 3);
			//	printf("%s\n", CGameLogic::StringHandTy(ty).c_str());
			//	ty = CGameLogic::GetDunCardHandTy(DunSecond, cards + 3, 5);
			//	printf("%s\n", CGameLogic::StringHandTy(ty).c_str());
			//	ty = CGameLogic::GetDunCardHandTy(DunLast, cards + 8, 5);
			//	printf("%s\n", CGameLogic::StringHandTy(ty).c_str());
			//}
		}
        else
        {
			TestEnumCards(size);
		}
	}
	
	void CGameLogic::EnumTree::Reset() {
		//按同花顺/铁支/葫芦/同花/顺子/三条/两对/对子顺序依次进行
		v123sc.clear();
		v40.clear();
		v32.clear();
		vsc.clear();
		v123.clear();
		v30.clear();
		v22.clear();
		v20.clear();
	}

	//打印枚举牌型
	void CGameLogic::EnumTree::PrintEnumCards(bool reverse/* = false*/, HandTy ty/* = TyAllBase*/) {
		switch (ty)
		{
		case Ty20:		PrintEnumCards("对子", ty, v20, reverse);		break;//对子
		case Ty22:		PrintEnumCards("两对", ty, v22, reverse);		break;//两对
		case Ty30:		PrintEnumCards("三条", ty, v30, reverse);		break;//三条
		case Ty123:		PrintEnumCards("顺子", ty, v123, reverse);		break;//顺子
		case Tysc:		PrintEnumCards("同花", ty, vsc, reverse);		break;//同花
		case Ty32:		PrintEnumCards("葫芦", ty, v32, reverse);		break;//葫芦
		case Ty40:		PrintEnumCards("铁支", ty, v40, reverse);		break;//铁支
		case Ty123sc:	PrintEnumCards("同花顺", ty, v123sc, reverse);	break;//同花顺
		case Ty123scRoyal:	PrintEnumCards("皇家同花顺", ty, v123sc, reverse);	break;//皇家同花顺
		case TyAll:
		default: {
			for (int i = Ty123sc; i >= Ty20; --i) {
				PrintEnumCards(reverse, (HandTy)(i));
			}
			break;
		}
		}
	}

	//打印枚举牌型
	void CGameLogic::EnumTree::PrintEnumCards(std::string const& name, HandTy ty, std::vector<std::vector<uint8_t>> const& src, bool reverse) {
		if (src.size() > 0) {
			if (reverse) {
				for (std::vector<std::vector<uint8_t>>::const_reverse_iterator it = src.rbegin();
					it != src.rend(); ++it) {
					PrintCardList(&it->front(), it->size());
				}
			}
			else {
				for (std::vector<std::vector<uint8_t>>::const_iterator it = src.begin();
					it != src.end(); ++it) {
					PrintCardList(&it->front(), it->size());
				}
			}
		}
	}
};