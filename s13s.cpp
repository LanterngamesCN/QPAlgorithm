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
#include <map>

#include "cfg.h"
#include "funcC.h"
#include "s13s.h"
#include "weights.h"
#include "StdRandom.h"

//protobuf测试
#include "s13s.Message.pb.h"
#include "pb2Json.h"

namespace S13S {

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
			////// 特殊牌型
		case TyThreesc: return "TyThreesc";
		case TyThree123: return "TyThree123";
		case TySix20:	return "TySix20";
		case TyFive2030:	return "TyFive2030";
		case TyFour30:	return "TyFour30";
		case TyTwo3220:	return "TyTwo3220";
		case TyAllOneColor:	return "TyAllOneColor";
		case TyAllSmall:	return "TyAllSmall";
		case TyAllBig:	return "TyAllBig";
		case TyThree40:	return "TyThree40";
		case TyThree123sc:	return "TyThree123sc";
		case Ty12Royal:	return "Ty12Royal";
		case TyOneDragon:	return "TyOneDragon";
		case TyZZQDragon:	return "TyZZQDragon";
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

#if 0
	//求组合C(n,1)*C(n,1)...*C(n,1)
	//f(k)=C(n,1)
	//Multi(k)=f(1)*f(2)...*f(k)
	//n int 访问广度 由c4,c3,c2计算得到(4/3/2/1)
	//k int 访问深度
	//clr bool true->区分花色的所有组合
	//clr bool false->不区分花色的所有组合
	//深度优先遍历，由浅到深，广度遍历，由里向外
	int CGameLogic::DepthVisit(int c4, int c3, int c2,
		int k,
		uint8_t(*const*const psrc)[4],
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst,
		std::vector<int> const& vec, bool clr) {
		int c = 0;
		static int const KDEPTH = MaxSZ;
		int e[KDEPTH + 1] = { 0 };
		e[0] = k;
		//dst.clear();
		//ctx.clear();
		return DepthC(c4, c3, c2, k, e, c, psrc, ctx, dst, vec, clr);
	}

	//递归求组合C(n,1)*C(n,1)...*C(n,1)
	//f(k)=C(n,1)
	//Multi(k)=f(1)*f(2)...*f(k)
	//n int 访问广度 由c4,c3,c2计算得到(4/3/2/1)
	//k int 访问深度
	//clr bool true->区分花色的所有组合
	//clr bool false->不区分花色的所有组合
	//深度优先遍历，由浅到深，广度遍历，由里向外
	int CGameLogic::DepthC(int c4, int c3, int c2,
		int k, int *e, int& c,
		uint8_t(*const*const psrc)[4],
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst,
		std::vector<int> const& vec, bool clr) {
		//c4,c3,c2计算n(4/3/2/1)
		int n = clr ? 4 : 1;
		if (vec[k - 1] < c4) {
			n = clr ? 4 : 1;
		}
		else if (vec[k - 1] < (c4 + c3)) {
			n = clr ? 4/*3*/ : 1;
		}
		else {
			n = clr ? 4/*2*/ : 1;
		}
		//不区分花色取一张就够了
		for (int i = n; i > 0; --i) {
			if ((*psrc[vec[k - 1]])[i - 1] == 0) {
				continue;
			}
			assert(k > 0);
			//psrc第vec[j - 1]组第e[j] - 1张(0/1/2/3)
			//psrc第vec[j - 1]组第e[j] - 1张(0/1/2)
			//psrc第vec[j - 1]组第e[j] - 1张(0/1)
			e[k] = i;
			if (k > 1) {
				DepthC(c4, c3, c2, k - 1, e, c, psrc, ctx, dst, vec, clr);
			}
			else {
				++c;
				std::vector<uint8_t> v;
				std::vector<short> w;
				//深度1...e[0]
				for (int j = e[0]; j > 0; --j) {
					//printf("%d", e[j]);
					//printf("%s", StringCard((*psrc[vec[j - 1]])[e[j] - 1]).c_str());
					v.push_back((*psrc[vec[j - 1]])[e[j] - 1]);
					{
						//psrc第r组/牌数c ///
						uint8_t r = vec[j - 1];
						uint8_t c = 0;
						while (c < 4 && (*psrc[r])[c] > 0) {
							++c;
						}
						//当前组合位于psrc的ctx信息
						short ctx = (short)(((0xFF & r) << 8) | (0xFF & c));
						w.push_back(ctx);
					}
				}
				//printf(",");
				dst.push_back(v);
				//v.clear();
				ctx.push_back(w);
				//w.clear();
			}
		}
		return c;
	}
#endif

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

#if 0
	//枚举单张组合牌(分别从四张/三张/对子中任抽一张组合牌)
	//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
	//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
	//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
	//dst std::vector<std::vector<uint8_t>>& 存放单张组合牌//////
	//clr bool true->区分花色的所有组合 false->不区分花色的所有组合
	int CGameLogic::EnumCombineCards(
		uint8_t(**const psrc)[4],
		uint8_t(*const src4)[4], int const c4,
		uint8_t(*const src3)[4], int const c3,
		uint8_t(*const src2)[4], int const c2,
		std::vector<std::vector<short>>& ctx,
		std::vector<std::vector<uint8_t>>& dst, bool clr) {
		//uint8_t(*src[6])[4] = { 0 };
		//typedef uint8_t(*Ptr)[4];
		//Ptr psrc[6] = { 0 };
		int c = 0;
		dst.clear();
		ctx.clear();
		//所有重复四张牌型
		for (int i = 0; i < c4; ++i) {
			psrc[c++] = &src4[i];
		}
		//所有重复三张牌型
		for (int i = 0; i < c3; ++i) {
			psrc[c++] = &src3[i];
		}
		//所有重复二张牌型
		for (int i = 0; i < c2; ++i) {
			psrc[c++] = &src2[i];
		}
		int n = c;
		CFuncC fnC;
		std::vector<std::vector<int>> vec;
		//psrc组与组之间按牌值升序排列(从小到大)
		SortCards_src(psrc, n, true, true);
		//////从n组里面选取任意1/2/3...k...n组的组合数 //////
		//C(n,1),C(n,2),C(n,3)...C(n,k)...C(n,n)
		for (int k = 1; k <= n; ++k) {
			int c = fnC.FuncC(n, k, vec);
			//printf("\n--- *** ------------------- C(%d,%d)=%d\n", n, k, c);
			for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
				it != vec.end(); ++it) {
				assert(k == it->size());
				//printf("\n--- *** start\n");
#if 0
				for (std::vector<int>::const_iterator ir = it->begin();
					ir != it->end(); ++ir) {
					printf("%d", *ir);
				}
 				printf("\n---\n");
#endif
// 				for (std::vector<int>::const_iterator ir = it->begin();
// 					ir != it->end(); ++ir) {
// 					if (*ir < c4) {
// 						PrintCardList(*psrc[*ir], 4);
// 					}
// 					else if (*ir < (c4 + c3)) {
// 						PrintCardList(*psrc[*ir], 3);
// 					}
// 					else {
// 						PrintCardList(*psrc[*ir], 2);
// 					}
//				}
				//printf("---\n");
				//f(k)=C(n,1)
				//Multi(k)=f(1)*f(2)...*f(k)
				//////从选取的k组中分别任取一张牌的组合数 //////
				int c = DepthVisit(c4, c3, c2, k, psrc, ctx, dst, *it, clr);
				//printf("\n--- *** end c=%d\n", c);
			}
		}
		return dst.size();
	}

	static bool ExistVec(std::vector<std::vector<uint8_t>>& dst, std::vector<uint8_t>& v) {
		for (std::vector<std::vector<uint8_t>>::const_iterator it = dst.begin();
			it != dst.end(); ++it) {
			if (it->size() == v.size()) {
				int i = 0;
				for (; i != it->size(); ++i) {
					if (v[i] != (*it)[i]) {
						break;
					}
				}
				if (i == it->size())
					return true;
			}
		}
		return false;
	}

	//从补位合并后的单张组合牌中枚举连续牌型///////
	//src uint8_t const* 单张组合牌源，去重后的余牌/散牌与单张组合牌补位合并
	//n int 抽取n张(3/5/13)
	//dst1 std::vector<std::vector<uint8_t>>& 存放所有连续牌型(非同花)
	//dst2 std::vector<std::vector<uint8_t>>& 存放所有连续牌型(同花)
	int CGameLogic::EnumConsecCardsMarkLoc(uint8_t const* src, int len, int n,
		std::vector<short>& ctx,
		std::vector<std::vector<short>>& dstctx,
		std::vector<std::vector<uint8_t>>& dst) {
		assert(len > 0);
		int i = 0, j = 0/*, k = 0*/, s;
		uint8_t v_src_pre = 0;
	next:
		while (i < len) {
			s = i++;
			v_src_pre = GetCardValue(src[s]);
			if (v_src_pre > 0) {
				break;
			}
		}
		if (s + n <= len) {
			for (; i < len; ++i) {
				//src中当前的牌值
				uint8_t v_src_cur = GetCardValue(src[i]);
				//牌位有值则连续
				if (v_src_cur > 0) {
					//收集到n张牌后返回
					if (i - s + 1 == n) {
						//缓存符合要求的牌型
						std::vector<uint8_t> v(&src[s], &src[s] + n);
						if (!ExistVec(dst, v)) {
							dst.push_back(v);
							dstctx.push_back(ctx);
							//printf("--- *** dst s:%d i:%d\n", s, i);
							PrintCardList(&v.front(), n);
						}
						++s;
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
#endif
	
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

#if 0
	//枚举所有n张连续牌型(同花顺/顺子)，先去重再补位，最后遍历查找
	//去重后的余牌/散牌与单张组合牌补位合并//////
	//src std::vector<std::vector<uint8_t>> const& 所有单张组合牌源///////
	//cpy uint8_t const* 去重后的余牌/散牌(除去四张/三张/对子)牌源///////
	//n int 抽取n张(3/5/13)
	//dst1 std::vector<std::vector<uint8_t>>& 存放所有连续牌型(非同花)
	//dst2 std::vector<std::vector<uint8_t>>& 存放所有连续牌型(同花)
	void CGameLogic::EnumConsecCards(
		std::vector<std::vector<uint8_t>> const& src,
		std::vector<std::vector<short>>& ctx,
		uint8_t const* cpy, int const cpylen, int n,
		std::vector<std::vector<short>>& dstctx,
		std::vector<std::vector<uint8_t>>& dst) {
		int i = 0;
		dstctx.clear();
		//用A2345678910JQK来占位 size = MaxSZ
		uint8_t mark[MaxSZ] = { 0 };
		//各单张组合牌分别与去重后的余牌/散牌补位合并
		for (std::vector<std::vector<uint8_t>>::const_iterator it = src.begin();
			it != src.end(); ++it) {
			//用去重后的余牌/散牌进行补位
			FillCardsMarkLoc(mark, MaxSZ, cpy, cpylen, true);
			//printf("\n\n");
			//PrintCardList(&it->front(), it->size());
			//PrintCardList(mark, MaxSZ, false);
			//用当前单张组合牌进行补位合并
			FillCardsMarkLoc(mark, MaxSZ, &it->front(), it->size(), false);
			//PrintCardList(mark, MaxSZ, false);
			//从补位合并后的单张组合牌中枚举连续牌型
			EnumConsecCardsMarkLoc(mark, MaxSZ, n, ctx[i++], dstctx, dst);
			//printf("\n\n\n");
		}
		printf("n = %d\n\n", dst.size());
	}
#endif

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
		return CGameLogic::CompareCards(&a->front(), &b->front(), a->size(), true, TyNil) > 0;
 	}
	//同花顺之间/同花之间/顺子之间比较大小
 	static bool As123scbyPointL(std::vector<uint8_t> const* a, std::vector<uint8_t> const* b) {
		return CGameLogic::CompareCards(&a->front(), &b->front(), a->size(), true, TyNil) < 0;
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
#if 0
		//枚举单张组合牌(分别从四张/三张/对子中任抽一张组合牌)
		std::vector<std::vector<uint8_t>> all;
		std::vector<std::vector<short>> allctx;
		EnumCombineCards(psrc, dst4, c4, dst3, c3, dst2, c2, allctx, all, clr);
		printf("\n\nall = %d\n", all.size());
		//枚举所有五张连续牌型(同花顺/顺子)
		EnumConsecCards(all, allctx, cpy, cpylen, 5, ctx, dst);
		assert(allctx.size() == all.size());
#else
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
#endif
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
		return sp0 < sp1;
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
		return sp0 > sp1;
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
							if (0 == CompareCardPointBy(&(src[i])[0], 5, cc, 5, false)) {
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
							if (0 == CompareCardPointBy(&(src[i])[0], 5, cc, 5, false)) {
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
						if (0 == CompareCardPointBy(&(src[i])[0], 3, &it3->front(), 3, false)) {
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
		return p0 < p1;
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
		return p0 > p1;
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
						if (0 == CompareCardPointBy(&(src[i])[0], 4, cc, 4, false)) {
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
					if (0 == CompareCardPointBy(&(src[i])[0], 2, &it3->front(), 2, false)) {
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

	//是否连号n张牌
	bool CGameLogic::CheckConsecCards(uint8_t const* src, int len)
	{
		//手牌按牌值从小到大排序(A23...QK)
		//SortCards(cards, len, true, true, true);
		//判断第二张牌到最后一张牌的连续性([A]23....QK)
		for (int i = 1; i < len - 1; ++i) {
			uint8_t v0 = GetCardValue(src[i]);
			uint8_t v1 = GetCardValue(src[i + 1]);
			if (v0 + 1 != v1) {
				//有一个不连续
				return false;
			}
		}
		//第二张牌到最后一张牌连续
		uint8_t v0 = GetCardValue(src[0]);
		uint8_t v1 = GetCardValue(src[1]);
		if (v0 + 1 == v1) {
			//前两张牌也连续
			return true;
		}
		//前两张牌不连续，判断是否A...K
		if (v0 + 12 == GetCardValue(src[len - 1])) {
			//手牌按牌点从小到大排序([A]...QKA)
			//SortCards(src, len, false, true, true);
			return true;
		}
		return false;
	}

	//是否同花n张牌
	bool CGameLogic::CheckSameColorCards(uint8_t const* src, int len)
	{
		for (int i = 0; i < len - 1; ++i) {
			uint8_t c0 = GetCardColor(src[i]);
			uint8_t c1 = GetCardColor(src[i + 1]);
			if (c0 != c1) {
				//有不同花色的牌
				return false;
			}
		}
		return true;
	}

	//至尊青龙/一条龙(十三水)/十二皇族
	HandTy CGameLogic::CheckDragonRoyal(uint8_t const* src, int len) {
		HandTy specialTy = TyNil;
		{
			//十三张连续牌
			if (CheckConsecCards(src, len)) {
				if (CheckSameColorCards(src, len)) {
					//至尊青龙：同花A到K的牌型，A2345678910JQK
					return specialTy = TyZZQDragon;
				}
				//一条龙(十三水)：A到K的牌型，非同花，A2345678910JQK
				return specialTy = TyOneDragon;
			}
		}
		{
			//十二皇族：十三张全是J，Q，K，A的牌型
			int i = 0;
			while (i < len) {
				uint8_t v = GetCardValue(src[i++]);
				if (v != A) {
					if (v >= J) {
						//牌值从小到大
						specialTy = Ty12Royal;
					}
					break;
				}
			}
		}
		return specialTy;
	}
	
	//凑一色：全是红牌(方块/红心)或黑牌(黑桃/梅花)
	HandTy CGameLogic::CheckAllOneColor(uint8_t const* src, int len) {
		HandTy specialTy = TyAllOneColor;
		uint8_t c0 = GetCardColor(src[0]);
		switch (c0)
		{
		case Spade:
		case Club:
			for (int i = 1; i < len; ++i) {
				uint8_t ci = GetCardColor(src[i]);
				if (ci == Heart || ci == Diamond) {
					return specialTy = TyNil;
				}
			}
			break;
		case Heart:
		case Diamond:
			for (int i = 1; i < len; ++i) {
				uint8_t ci = GetCardColor(src[i]);
				if (ci == Spade || ci == Club) {
					return specialTy = TyNil;
				}
			}
			break;
		}
		return specialTy;
	}
	
	//全大：全是8至A的牌型
	HandTy CGameLogic::CheckAllBig(uint8_t const* src, int len) {
		HandTy specialTy = TyNil;
		int i = 0;
		while (i < len) {
			uint8_t v = GetCardValue(src[i++]);
			if (v != A) {
				if (v >= 8) {
					//全大/牌值从小到大
					specialTy = TyAllBig;
				}
				break;
			}
		}
		return specialTy;
	}
	
	//全小：全是2至8的牌型
	HandTy CGameLogic::CheckAllSmall(uint8_t const* src, int len) {
		HandTy specialTy = TyNil;
		if (GetCardValue(src[0]) != A) {
			if (GetCardValue(src[len - 1]) <= 8) {
				//全小/牌值从小到大
				specialTy = TyAllSmall;
			}
		}
		return specialTy;
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

	//手牌牌型分析(特殊牌型判断/枚举三墩组合)，算法入口 /////////
	//src uint8_t const* 一副手牌(13张)
	//n int 最大枚举多少组墩(头墩&中墩&尾墩加起来为一组)
	//chairID int 玩家座椅ID
	//hand handinfo_t& 保存手牌信息
	int CGameLogic::AnalyseHandCards(uint8_t const* src, int len, int n, handinfo_t& hand) {
		int c = 0;
		uint8_t cpy[MaxSZ] = { 0 }, cpy2[MaxSZ] = { 0 }, cpy3[MaxSZ] = { 0 };
		int cpylen = 0, cpylen2 = 0, cpylen3 = 0;

		uint8_t const* psrc = NULL, *psrc2 = NULL;
		int lensrc = 0, lensrc2 = 0;
		
		//叶子节点(头墩)/子节点(中墩)/根节点(尾墩)
		int cursorLeaf = 0, cursorChild = 0, cursorRoot = 0;
		HandTy tyLeaf = TyNil, tyChild = TyNil, tyRoot = TyNil;
		EnumTree::CardData const *leaf = NULL, *child = NULL, *root = NULL;
		
		//防止重复添加
		std::map<uint64_t, bool> masks;
		uint64_t maskRoot = 0, maskChild = 0;

		hand.Init();
		//叶子节点列表
		//枚举几组最优墩(头墩&中墩&尾墩加起来为一组)，由叶子节点向上往根节点遍历
		std::vector<EnumTree::TraverseTreeNode>& leafList = hand.leafList;
		//根节点：初始枚举所有牌型列表
		EnumTree *& rootEnumList = hand.rootEnumList;
		assert(rootEnumList != NULL);
		hand.Reset();
#ifdef _MEMORY_LEAK_DETECK_
		printf("\n--- *** hand[%d]init MemoryCount = %d\n", hand.chairID, hand.rootEnumList->MemoryCount_);
#endif
		//枚举尾墩/5张 //////
		EnumCards(src, len, 5, hand.classify, *rootEnumList, DunLast);
	entry_root_iterator:
		while (c < n) {
			//返回一个枚举牌型及对应的余牌
			//按同花顺/铁支/葫芦/同花/顺子/三条/两对/对子/散牌的顺序
			memset(cpy, 0, MaxSZ * sizeof(uint8_t));
			if (!rootEnumList->GetNextEnumItem(src, len, root, tyRoot, cursorRoot, cpy, cpylen)) {
				break;
			}
			
			//printf("取尾墩 = [%s] ", StringHandTy(tyRoot).c_str());
			//PrintCardList(&root->front(), root->size());
			//rootEnumList->PrintCursorEnumCards();

			//除去尾墩后的余牌
			psrc = cpy;
			lensrc = cpylen;
			
			//子节点：根节点当前枚举项牌型对应余牌枚举子项列表
			EnumTree*& childEnumList = rootEnumList->GetCursorChild(cursorRoot);
			assert(childEnumList == NULL);
			childEnumList = new EnumTree();
#ifdef _MEMORY_LEAK_DETECK_
			++hand.rootEnumList->MemoryCount_;
#endif
			//指向父节点/对应父节点游标位置
			childEnumList->parent_ = rootEnumList;
			childEnumList->parentcursor_ = cursorRoot;
			
			//计算根节点游标掩码
			maskRoot = (uint64_t)((cursorRoot + 1) & 0xFFFFFFFF);

			classify_t classify = { 0 };
			//从余牌中枚举中墩/5张 //////
			EnumCards(psrc, lensrc, 5, classify, *childEnumList, DunSecond);
			
			//printf("\n\n\n--- *** --------------------------------------------------\n");
			//childEnumList->PrintEnumCards(false, TyAllBase);
			//printf("--- *** --------------------------------------------------\n\n\n");

		entry_child_iterator:
			while (c < n) {
				//返回一个枚举牌型及对应的余牌
				//按同花顺/铁支/葫芦/同花/顺子/三条/两对/对子/散牌的顺序
				memset(cpy2, 0, MaxSZ * sizeof(uint8_t));
				if (!childEnumList->GetNextEnumItem(psrc, lensrc, child, tyChild, cursorChild, cpy2, cpylen2)) {
					
					//printf("--- *** --------------------------------------------------\n");
					//rootEnumList->PrintCursorEnumCards();
					//printf("--- *** --------------------------------------------------\n");
					
					std::map<uint64_t, bool>::const_iterator it = masks.find(maskRoot);
					if (it == masks.end()) {
						//根节点为叶子节点，记录尾墩
						leafList.push_back(EnumTree::TraverseTreeNode(rootEnumList, cursorRoot));
						masks[maskRoot] = true;
						++c;
					}
					break;
				}
				//跳过三墩牌组合出现倒水情况
				{
					//牌型不同比牌型
					if (tyChild != tyRoot) {
						if (tyChild > tyRoot) {
							continue;
						}
					}
					else {
						//牌型相同从大到小比点数，葫芦牌型比较三张的大小(中间的牌)
						if (CGameLogic::CompareCards(
							&root->front(), root->size(),
							&child->front(), child->size(), false, tyChild) < 0) {
							continue;
						}
					}
				}
				
				//printf("\n取中墩 = [%s] ", StringHandTy(tyChild).c_str());
				//PrintCardList(&child->front(), child->size());
				//childEnumList->PrintCursorEnumCards();

				//除去中墩后的余牌
				psrc2 = cpy2;
				lensrc2 = cpylen2;

				//叶子节点：子节点当前枚举项牌型对应余牌枚举子项列表
				EnumTree*& leafEnumList = childEnumList->GetCursorChild(cursorChild);
				assert(leafEnumList == NULL);
				leafEnumList = new EnumTree();
#ifdef _MEMORY_LEAK_DETECK_
				++hand.rootEnumList->MemoryCount_;
#endif
				//指向父节点/对应父节点游标位置
				leafEnumList->parent_ = childEnumList;
				leafEnumList->parentcursor_ = cursorChild;
				
				//计算子节点游标掩码
				maskChild = (uint64_t)(((cursorChild + 1) & 0xFFFFFFFF) << 24) | (uint64_t)((cursorRoot + 1) & 0xFFFFFFFF);
				//printf("cursorRoot: %d cursorChild: %d\n", cursorRoot + 1, cursorChild + 1);
				
				classify_t classify = { 0 };
				//从余牌中枚举头墩/3张 //////
				EnumCards(psrc2, lensrc2, 3, classify, *leafEnumList, DunFirst);
				while (c < n) {
					//返回一个枚举牌型及对应的余牌
					//按同花顺/铁支/葫芦/同花/顺子/三条/两对/对子/散牌的顺序
					memset(cpy3, 0, MaxSZ * sizeof(uint8_t));
					if (!leafEnumList->GetNextEnumItem(psrc2, lensrc2, leaf, tyLeaf, cursorLeaf, cpy3, cpylen3)) {
						
						//printf("--- *** --------------------------------------------------\n");
						//childEnumList->PrintCursorEnumCards();
						//rootEnumList->PrintCursorEnumCards();
						//printf("--- *** --------------------------------------------------\n");
						
						std::map<uint64_t, bool>::const_iterator it = masks.find(maskChild);
						if (it == masks.end()) {
							//子节点为叶子节点，记录中墩和尾墩，由叶子节点向上查找根节点
							leafList.push_back(EnumTree::TraverseTreeNode(childEnumList, cursorChild));
							masks[maskChild] = true;
							masks[maskRoot] = true;
							++c;
						}
						break;
					}
					//跳过三墩牌组合出现倒水情况
					{
						//牌型不同比牌型
						if (tyLeaf != tyChild) {
							if(tyLeaf == Ty30 || tyLeaf == Ty20) {
								if (tyLeaf > tyChild) {
									continue;
								}
							}
						}
						else {
							if (tyLeaf == Ty30 || tyLeaf == Ty20) {
								//牌型相同从大到小比点数，葫芦牌型比较三张的大小(中间的牌)
								if (CGameLogic::CompareCards(
									&child->front(), child->size(),
									&leaf->front(), leaf->size(), false, tyLeaf) < 0) {
									continue;
								}
							}
						}
					}
					/*
					--- *** --------------------------------------------------
					--- *** 第[1]墩 - 同花顺[Ty123sc]：♦5 ♦6 ♦7
					--- *** 第[2]墩 - 铁支[Ty40]：♦2 ♣2 ♥2 ♠2
					--- *** 第[3]墩 - 同花顺[Ty123sc]：♦8 ♦9 ♦10 ♦J ♦Q
					--- *** *** *** *** *** 余牌：♣Q
					--- *** --------------------------------------------------
					--- *** --------------------------------------------------
					--- *** 第[1]墩 - 同花[Tysc]：♦5 ♦6 ♦Q
					--- *** 第[2]墩 - 铁支[Ty40]：♦2 ♣2 ♥2 ♠2
					--- *** 第[3]墩 - 同花顺[Ty123sc]：♦7 ♦8 ♦9 ♦10 ♦J
					--- *** *** *** *** *** 余牌：♣Q
					--- *** --------------------------------------------------
					--- *** --------------------------------------------------
					--- *** 第[1]墩 - 同花[Tysc]：♦5 ♦J ♦Q
					--- *** 第[2]墩 - 铁支[Ty40]：♦2 ♣2 ♥2 ♠2
					--- *** 第[3]墩 - 同花顺[Ty123sc]：♦6 ♦7 ♦8 ♦9 ♦10
					--- *** *** *** *** *** 余牌：♣Q
					--- *** --------------------------------------------------
					--- *** --------------------------------------------------
					--- *** 第[1]墩 - 同花顺[Ty123sc]：♦10 ♦J ♦Q
					--- *** 第[2]墩 - 铁支[Ty40]：♦2 ♣2 ♥2 ♠2
					--- *** 第[3]墩 - 同花顺[Ty123sc]：♦5 ♦6 ♦7 ♦8 ♦9
					--- *** *** *** *** *** 余牌：♣Q
					--- *** --------------------------------------------------
					*/
					//std::map<uint64_t, bool>::const_iterator it = masks.find(maskChild);
					//if (it == masks.end()) {	
						//printf("\n取头墩 = [%s] ", StringHandTy(tyLeaf).c_str());
						//PrintCardList(&leaf->front(), leaf->size());
						//leafEnumList->PrintCursorEnumCards();
						
						//printf("--- *** --------------------------------------------------\n");
						//leafEnumList->PrintCursorEnumCards();
						//childEnumList->PrintCursorEnumCards();
						//rootEnumList->PrintCursorEnumCards();
						//printf("--- *** *** *** *** *** 余牌：%s\n", StringCards(cpy3, cpylen3).c_str());
						//printf("--- *** --------------------------------------------------\n");

						//叶子节点作为叶子节点，记录头墩，中墩和尾墩，由叶子节点向上查找父节点和根节点
						leafList.push_back(EnumTree::TraverseTreeNode(leafEnumList, cursorLeaf));
						masks[maskChild] = true;
						masks[maskRoot] = true;
						++c;
						//重新从根节点开始迭代游标 //////
						goto entry_root_iterator;
					//}
				}
			}
		}
#ifdef _MEMORY_LEAK_DETECK_
		printf("--- *** hand[%d]malloc MemoryCount = %d\n", hand.chairID, hand.rootEnumList->MemoryCount_);
#endif
		hand.CalcHandCardsType(src, len);
		return c;
	}
	
	static bool GlobalCompareCardsByScore(CGameLogic::groupdun_t const* src, CGameLogic::groupdun_t const* dst) {
		//printf("\n\n--- *** --------------------------------------------------\n");
		//const_cast<CGameLogic::groupdun_t*&>(src)->PrintCardList(DunFirst);
		//const_cast<CGameLogic::groupdun_t*&>(src)->PrintCardList(DunSecond);
		//const_cast<CGameLogic::groupdun_t*&>(src)->PrintCardList(DunLast);
		//printf("--- *** --------------------------------------------------\n");
		//const_cast<CGameLogic::groupdun_t*&>(dst)->PrintCardList(DunFirst);
		//const_cast<CGameLogic::groupdun_t*&>(dst)->PrintCardList(DunSecond);
		//const_cast<CGameLogic::groupdun_t*&>(dst)->PrintCardList(DunLast);
		//printf("--- *** --------------------------------------------------\n");
		int bv = CGameLogic::CompareCardsByScore(src, dst);
		//printf("--- *** *** *** *** *** 结果 %d\n\n", bv);
		return bv > 0;
	}
	
	//确定手牌牌型
	void CGameLogic::handinfo_t::CalcHandCardsType(uint8_t const* src, int len) {
		
		//叶子节点(头墩)/子节点(中墩)/根节点(尾墩)
		int cursorLeaf = 0, cursorChild = 0, cursorRoot = 0;
		HandTy tyLeaf = TyNil, tyChild = TyNil, tyRoot = TyNil;
		EnumTree::CardData const *leaf = NULL, *child = NULL, *root = NULL;
		
		//临时保存枚举几组解
		std::vector<groupdun_t> enum_groups_tmp;
		
		//至尊青龙/一条龙(十三水)/十二皇族
		HandTy specialTy = CheckDragonRoyal(src, len);
		//跳过遍历节点继续下一个节点
		bool continueflag = false;

		//遍历枚举出来的每组牌型(头墩&中墩&尾墩加起来为一组) ////////////////////////////
		for (std::vector<EnumTree::TraverseTreeNode>::iterator it = leafList.begin();
			it != leafList.end(); ++it) {
		continue_:
			if (continueflag) {
				continueflag = false;
				continue;
			}
			//枚举MAX_ENUMSZ组最优解，深度不够，可能枚举不到特殊牌型
			//if (enum_groups_tmp.size() >= MAX_ENUMSZ) {
			//	break;
			//}
			//从叶子节点往根节点遍历 ////////////////////////////
			EnumTree::TraverseTreeNode& leafTraverseNode = *it;
			EnumTree*& nodeLeaf = leafTraverseNode.first;//
			cursorLeaf = leafTraverseNode.second;//
			switch (nodeLeaf->dt_)
			{
			//////////////////////////////////////////////////////////////
			case DunLast: {
				groupdun_t group;
				//尾墩
				{
					EnumTree::TreeNode& treeNode = nodeLeaf->tree[cursorLeaf];
					EnumTree::EnumItem& leafItem = treeNode.first;
					tyRoot = leafItem.first;
					root = leafItem.second;
					group.assign(DunLast, tyRoot, &root->front(), root->size());
				}
				assert(root->size() <= 5);
				//classify_t classify = { 0 };
				//EnumTree enumList;
				//printf("\n--- *** ---- start DunLast\n");				
				//group.PrintCardList(DunLast);
				//组墩后剩余牌/散牌
				uint8_t cpy[MaxSZ] = { 0 };
				int cpylen = 0, offset = 0;
				CGameLogic::GetLeftCards(src, len, group.duns, cpy, cpylen);
				CGameLogic::SortCards(cpy, cpylen, false, true, true);
				//printf("---------\n");
				//CGameLogic::PrintCardList(cpy, cpylen, true);
// 				{
// 					//如果余牌含有重复牌型则跳过
// 					typedef uint8_t(*Ptr)[4];
// 					Ptr psrc[6] = { 0 };
// 					int c = 0, c4 = 0, c3 = 0, c2 = 0, cpylen1 = 0;
// 					uint8_t cpy1[MaxSZ] = { 0 };
// 					//所有重复四张牌型
// 					int const size4 = 3;
// 					uint8_t dst4[size4][4] = { 0 };
// 					//所有重复三张牌型
// 					int const size3 = 4;
// 					uint8_t dst3[size3][4] = { 0 };
// 					//所有重复二张牌型
// 					int const size2 = 6;
// 					uint8_t dst2[size2][4] = { 0 };
// 					//返回去重后的余牌/散牌cpy
// 					c = RemoveRepeatCards(cpy, cpylen, psrc, dst4, c4, dst3, c3, dst2, c2, cpy1, cpylen1);
// 					if (c4 > 0 || c3 > 0 || c2 > 0) {
// 						//group.PrintCardList(DunLast);
// 						//CGameLogic::PrintCardList(cpy, cpylen, true);
// 						//assert(false);
// 						break;
// 					}
// 				}
				//补充尾墩(最小补牌)
				{
					int c = group.needC(DunLast);
					if (c > 0) {
						//跳过补牌后出现牌型变化情况
						{
							for (int i = 0; i < c; ++i) {
								uint8_t r = cpy[offset + i];
								for (int j = 0; j < group.duns[DunLast].GetC(); ++j) {
									if (CGameLogic::GetCardValue(r) == CGameLogic::GetCardValue(group.duns[DunLast].cards[j])) {
										continueflag = true;
										goto continue_;
									}
								}
							}
						}
						assert(offset < cpylen);
						group.append(DunLast, &cpy[offset], c);
						offset += c;
						//补牌后判断单墩牌型是否变化
						HandTy ty = GetDunCardHandTy(DunLast, group.duns[DunLast].cards, group.duns[DunLast].GetC());
						if (ty != group.duns[DunLast].ty_) {
							break;
						}
						//补牌后判断是否倒水
						if (IsInverted(DunLast, group.duns[DunLast].cards, group.duns[DunLast].GetC(), group.duns[DunLast].ty_, &group)) {
							break;
						}
					}
				}
				//补充头墩(次大补牌)
				{
					assert(offset < cpylen);
					group.assign(DunFirst, Tysp, &cpy[offset], 3);
					offset += 3;
					//补牌后判断单墩牌型是否变化
					HandTy ty = GetDunCardHandTy(DunFirst, group.duns[DunFirst].cards, group.duns[DunFirst].GetC());
					if (ty == Ty123sc || ty == Tysc || ty == Ty123) {
						ty = Tysp;
					}
					if (ty != group.duns[DunFirst].ty_) {
						break;
					}
					//补牌后判断是否倒水
					if (IsInverted(DunFirst, group.duns[DunFirst].cards, group.duns[DunFirst].GetC(), group.duns[DunFirst].ty_, &group)) {
						break;
					}
				}
				//补充中墩(最大补牌)
				{
					assert(offset < cpylen);
					group.assign(DunSecond, Tysp, &cpy[offset], 5);
					offset += 5;
					assert(offset == cpylen);
					//补牌后判断单墩牌型是否变化
					HandTy ty = GetDunCardHandTy(DunSecond, group.duns[DunSecond].cards, group.duns[DunSecond].GetC());
					if (ty != group.duns[DunSecond].ty_) {
						break;
					}
					//补牌后判断是否倒水
					if (IsInverted(DunSecond, group.duns[DunSecond].cards, group.duns[DunSecond].GetC(), group.duns[DunSecond].ty_, &group)) {
						break;
					}
				}
				//头墩最大单张与中墩次大单张交换
				{
					for (int i = 0; i < group.duns[DunFirst].GetC(); ++i) {
						assert((group.duns[DunFirst].GetC() - 1 - i) >= 0);
						std::swap(group.duns[DunFirst].cards[group.duns[DunFirst].GetC() - 1 - i],
							group.duns[DunSecond].cards[group.duns[DunSecond].GetC() - 2 - i]);
					}
				}
				//group.PrintCardList(DunFirst);
				//group.PrintCardList(DunSecond);
				//group.PrintCardList(DunLast);
				bool existRepeated = false;
				for (std::vector<groupdun_t>::const_iterator it = enum_groups_tmp.begin();
					it != enum_groups_tmp.end(); ++it) {
					if (CGameLogic::CompareCards(
						it->duns[DunLast].cards, it->duns[DunLast].c, it->duns[DunLast].ty_,
						group.duns[DunLast].cards, group.duns[DunLast].c, group.duns[DunLast].ty_, false, true) == 0 &&
						CGameLogic::CompareCards(
							it->duns[DunSecond].cards, it->duns[DunSecond].c, it->duns[DunSecond].ty_,
							group.duns[DunSecond].cards, group.duns[DunSecond].c, group.duns[DunSecond].ty_, false, true) == 0 &&
						CGameLogic::CompareCards(
							it->duns[DunFirst].cards, it->duns[DunFirst].c, it->duns[DunFirst].ty_,
							group.duns[DunFirst].cards, group.duns[DunFirst].c, group.duns[DunFirst].ty_, false, true) == 0) {
						existRepeated = true;
						break;
					}
				}
				if (!existRepeated && enum_groups_tmp.size() < MAX_ENUMSZ) {
					enum_groups_tmp.push_back(group);
				}
				break;
			}
			//////////////////////////////////////////////////////////////
			case DunSecond: {
				groupdun_t group;
				//中墩
				{
					EnumTree::TreeNode& treeNode = nodeLeaf->tree[cursorLeaf];
					EnumTree::EnumItem& leafItem = treeNode.first;
					tyChild = leafItem.first;
					child = leafItem.second;
					group.assign(DunSecond, tyChild, &child->front(), child->size());
				}
				//尾墩
				EnumTree *nodeRoot = nodeLeaf->parent_;//
				cursorRoot = nodeLeaf->parentcursor_;//
				{
					assert(nodeRoot != NULL);
					EnumTree::TreeNode& treeNode = nodeRoot->tree[cursorRoot];
					EnumTree::EnumItem& rootItem = treeNode.first;
					tyRoot = rootItem.first;
					root = rootItem.second;
					group.assign(DunLast, tyRoot, &root->front(), root->size());
				}
				assert(child->size() <= 5);
				//classify_t classify = { 0 };
				//EnumTree enumList;
				//printf("\n--- *** ---- start DunSecond\n");
				//group.PrintCardList(DunSecond);
				//group.PrintCardList(DunLast);
				//组墩后剩余牌/散牌
				uint8_t cpy[MaxSZ] = { 0 };
				int cpylen = 0, offset = 0;
				CGameLogic::GetLeftCards(src, len, group.duns, cpy, cpylen);
				CGameLogic::SortCards(cpy, cpylen, false, true, true);
				//printf("---------\n");
				//CGameLogic::PrintCardList(cpy, cpylen, true);
#if 1
// 				{
// 					//如果余牌含有重复牌型则跳过
// 					typedef uint8_t(*Ptr)[4];
// 					Ptr psrc[6] = { 0 };
// 					int c = 0, c4 = 0, c3 = 0, c2 = 0, cpylen1 = 0;
// 					uint8_t cpy1[MaxSZ] = { 0 };
// 					//所有重复四张牌型
// 					int const size4 = 3;
// 					uint8_t dst4[size4][4] = { 0 };
// 					//所有重复三张牌型
// 					int const size3 = 4;
// 					uint8_t dst3[size3][4] = { 0 };
// 					//所有重复二张牌型
// 					int const size2 = 6;
// 					uint8_t dst2[size2][4] = { 0 };
// 					//返回去重后的余牌/散牌cpy
// 					c = RemoveRepeatCards(cpy, cpylen, psrc, dst4, c4, dst3, c3, dst2, c2, cpy1, cpylen1);
// 					if (c4 > 0 || c3 > 0 || c2 > 0) {
// 						//group.PrintCardList(DunSecond);
// 						//group.PrintCardList(DunLast);
// 						//CGameLogic::PrintCardList(cpy, cpylen, true);
// 						//assert(false);
// 						break;
// 					}
// 				}
#else
// 				{
// 					CGameLogic::EnumCards(cpy, cpylen, 3, classify, enumList, DunFirst);
// 					if (enumList.v30.size() > 0) {
// 						tyLeaf = Ty30;//三条
// 						leaf = &enumList.v30[0];
// 						goto restart;
// 					}
// 					else if (enumList.v22.size() > 0) {
// 						tyLeaf = Ty22;//两对
// 					}
// 					else if (enumList.v20.size() > 0) {
// 						tyLeaf = Ty20;//对子
// 						leaf = &enumList.v20[0];
// 						goto restart;
// 					}
// 				}
#endif
				//补充尾墩
				{
					int c = group.needC(DunLast);
					if (c > 0) {
						//跳过补牌后出现牌型变化情况
						{
							for (int i = 0; i < c; ++i) {
								uint8_t r = cpy[offset + i];
								for (int j = 0; j < group.duns[DunLast].GetC(); ++j) {
									if (CGameLogic::GetCardValue(r) == CGameLogic::GetCardValue(group.duns[DunLast].cards[j])) {
										continueflag = true;
										goto continue_;
									}
								}
							}
						}
						assert(offset < cpylen);
						group.append(DunLast, &cpy[offset], c);
						offset += c;
						//补牌后判断单墩牌型是否变化
						HandTy ty = GetDunCardHandTy(DunLast, group.duns[DunLast].cards, group.duns[DunLast].GetC());
						if (ty != group.duns[DunLast].ty_) {
							break;
						}
						//补牌后判断是否倒水
						if (IsInverted(DunLast, group.duns[DunLast].cards, group.duns[DunLast].GetC(), group.duns[DunLast].ty_, &group)) {
							break;
						}
					}
				}
				//补充中墩
				{
					int c = group.needC(DunSecond);
					if (c > 0) {
						//跳过补牌后出现牌型变化情况
						{
							for (int i = 0; i < c; ++i) {
								uint8_t r = cpy[offset + i];
								for (int j = 0; j < group.duns[DunSecond].GetC(); ++j) {
									if (CGameLogic::GetCardValue(r) == CGameLogic::GetCardValue(group.duns[DunSecond].cards[j])) {
										continueflag = true;
										goto continue_;
									}
								}
							}
						}
						assert(offset < cpylen);
						group.append(DunSecond, &cpy[offset], c);
						offset += c;
						//补牌后判断单墩牌型是否变化
						HandTy ty = GetDunCardHandTy(DunSecond, group.duns[DunSecond].cards, group.duns[DunSecond].GetC());
						if (ty != group.duns[DunSecond].ty_) {
							break;
						}
						//补牌后判断是否倒水
						if (IsInverted(DunSecond, group.duns[DunSecond].cards, group.duns[DunSecond].GetC(), group.duns[DunSecond].ty_, &group)) {
							break;
						}
					}
				}
				//补充头墩
				{
					assert(offset < cpylen);
					group.assign(DunFirst, Tysp, &cpy[offset], 3);
					offset += 3;
					assert(offset == cpylen);
					//补牌后判断单墩牌型是否变化
					HandTy ty = GetDunCardHandTy(DunFirst, group.duns[DunFirst].cards, group.duns[DunFirst].GetC());
					if (ty == Ty123sc || ty == Tysc || ty == Ty123) {
						ty = Tysp;
					}
					if (ty != group.duns[DunFirst].ty_) {
						break;
					}
					//补牌后判断是否倒水
					if (IsInverted(DunFirst, group.duns[DunFirst].cards, group.duns[DunFirst].GetC(), group.duns[DunFirst].ty_, &group)) {
						break;
					}
				}
				//group.PrintCardList(DunFirst);
				//group.PrintCardList(DunSecond);
				//group.PrintCardList(DunLast);
 				bool existRepeated = false;
 				for (std::vector<groupdun_t>::const_iterator it = enum_groups_tmp.begin();
 					it != enum_groups_tmp.end(); ++it) {
 					if (CGameLogic::CompareCards(
							it->duns[DunLast].cards, it->duns[DunLast].c, it->duns[DunLast].ty_,
 							group.duns[DunLast].cards, group.duns[DunLast].c, group.duns[DunLast].ty_, false, true) == 0 &&
						CGameLogic::CompareCards(
							it->duns[DunSecond].cards, it->duns[DunSecond].c, it->duns[DunSecond].ty_,
							group.duns[DunSecond].cards, group.duns[DunSecond].c, group.duns[DunSecond].ty_, false, true) == 0 &&
						CGameLogic::CompareCards(
							it->duns[DunFirst].cards, it->duns[DunFirst].c, it->duns[DunFirst].ty_,
							group.duns[DunFirst].cards, group.duns[DunFirst].c, group.duns[DunFirst].ty_, false, true) == 0) {
						existRepeated = true;
						break;
					}
 				}
				if (!existRepeated && enum_groups_tmp.size() < MAX_ENUMSZ) {
					enum_groups_tmp.push_back(group);
				}
				break;
			}
			//////////////////////////////////////////////////////////////
			case DunFirst: {
				groupdun_t group;
				//头墩
				{
					EnumTree::TreeNode& treeNode = nodeLeaf->tree[cursorLeaf];
					EnumTree::EnumItem& leafItem = treeNode.first;
					tyLeaf = leafItem.first;
					leaf = leafItem.second;
					assert(leaf->size() <= 3);
					//group.assign(DunFirst, tyLeaf, &leaf->front(), leaf->size());
				}
				//中墩
				EnumTree *nodeChild = nodeLeaf->parent_;//
				cursorChild = nodeLeaf->parentcursor_;//
				{
					assert(nodeChild != NULL);
					EnumTree::TreeNode& treeNode = nodeChild->tree[cursorChild];
					EnumTree::EnumItem& childItem = treeNode.first;
					tyChild = childItem.first;
					child = childItem.second;
					group.assign(DunSecond, tyChild, &child->front(), child->size());
				}
				//尾墩
				EnumTree *nodeRoot = nodeChild->parent_;//
				cursorRoot = nodeChild->parentcursor_;//
				{
					assert(nodeRoot != NULL);
					EnumTree::TreeNode& treeNode = nodeRoot->tree[cursorRoot];
					EnumTree::EnumItem& rootItem = treeNode.first;
					tyRoot = rootItem.first;
					root = rootItem.second;
					group.assign(DunLast, tyRoot, &root->front(), root->size());
				}
				//如果不是至尊青龙/一条龙(十三水)/十二皇族
				if (specialTy != TyZZQDragon && specialTy != TyOneDragon && specialTy != Ty12Royal) {
					if (tyRoot == Ty123sc && tyChild == Ty123sc && tyLeaf == Ty123sc) {
						//如果不是三同花顺
						if (specialTy != TyThree123sc) {
							//三同花顺
							specialTy = group.specialTy = TyThree123sc;
						}
					}
					else if (tyRoot == Ty123 && tyChild == Ty123 && tyLeaf == Ty123) {
						//如果不是三同花顺且不是三顺子
						if (specialTy != TyThree123sc && specialTy != TyThree123) {
							//三顺子
							specialTy = group.specialTy = TyThree123;
						}
					}
					//Fixed BUG：三同花：尾墩(Tysc)/中墩(Tysc)/头墩(Tysc/Ty123sc)
					else if (tyRoot == Tysc && tyChild == Tysc && (tyLeaf == Tysc || tyLeaf == Ty123sc)) {
						//如果不是三同花顺且不是三顺子且不是三同花
						if (specialTy != TyThree123sc && specialTy != TyThree123 && specialTy != TyThreesc) {
							//三同花
							specialTy = group.specialTy = TyThreesc;
						}
					}
				}
				//头墩非对子/非三张，整墩非三同花顺/非三顺子/非三同花，则修改头墩为乌龙
				if (tyLeaf != Ty20 && tyLeaf != Ty30 && group.specialTy != TyThree123sc && group.specialTy != TyThree123 && group.specialTy != TyThreesc) {
					tyLeaf = Tysp;
				}
				assert(leaf->size() <= 3);
				classify_t classify = { 0 };
				EnumTree enumList;
				//printf("\n--- *** ---- start DunFirst\n");				
				//group.PrintCardList(DunSecond);
				//group.PrintCardList(DunLast);
			restart:
				//若叶子节点为对子或者三条，则放入头墩，否则当作散牌与余牌合并处理
				if (tyLeaf == Ty20 || tyLeaf == Ty30) {
					group.assign(DunFirst, tyLeaf, &leaf->front(), leaf->size());
					//group.PrintCardList(DunFirst);
				}
				//组墩后剩余牌/散牌
				uint8_t cpy[MaxSZ] = { 0 };
				int cpylen = 0, offset = 0;
				CGameLogic::GetLeftCards(src, len, group.duns, cpy, cpylen);
				CGameLogic::SortCards(cpy, cpylen, false, true, true);
				//printf("---------\n");
				//CGameLogic::PrintCardList(cpy, cpylen, true);
				if (group.duns[DunFirst].GetC() == 0) {
					//如果头墩为空
					CGameLogic::EnumCards(cpy, cpylen, 3, classify, enumList, DunFirst);
					if (enumList.v30.size() > 0) {
						tyLeaf = Ty30;//三条
						leaf = &enumList.v30[0];
						goto restart;
					}
					else if (enumList.v20.size() > 0) {
						tyLeaf = Ty20;//对子
						leaf = &enumList.v20[0];
						goto restart;
					}
				}
// 				{
// 					//如果余牌含有重复牌型则跳过
// 					typedef uint8_t(*Ptr)[4];
// 					Ptr psrc[6] = { 0 };
// 					int c = 0, c4 = 0, c3 = 0, c2 = 0, cpylen1 = 0;
// 					uint8_t cpy1[MaxSZ] = { 0 };
// 					//所有重复四张牌型
// 					int const size4 = 3;
// 					uint8_t dst4[size4][4] = { 0 };
// 					//所有重复三张牌型
// 					int const size3 = 4;
// 					uint8_t dst3[size3][4] = { 0 };
// 					//所有重复二张牌型
// 					int const size2 = 6;
// 					uint8_t dst2[size2][4] = { 0 };
// 					//返回去重后的余牌/散牌cpy
// 					c = RemoveRepeatCards(cpy, cpylen, psrc, dst4, c4, dst3, c3, dst2, c2, cpy1, cpylen1);
// 					if (c4 > 0 || c3 > 0 || c2 > 0) {
// 						//group.PrintCardList(DunFirst);
// 						//group.PrintCardList(DunSecond);
// 						//group.PrintCardList(DunLast);
// 						//CGameLogic::PrintCardList(cpy, cpylen, true);
// 						//assert(false);
// 						break;
// 					}
// 				}
				//补充尾墩
				{
					int c = group.needC(DunLast);
					if (c > 0) {
						//跳过补牌后出现牌型变化情况
						{
							for (int i = 0; i < c; ++i) {
								uint8_t r = cpy[offset + i];
								for (int j = 0; j < group.duns[DunLast].GetC(); ++j) {
									if (CGameLogic::GetCardValue(r) == CGameLogic::GetCardValue(group.duns[DunLast].cards[j])) {
										continueflag = true;
										goto continue_;
									}
								}
							}
						}
						assert(offset < cpylen);
						group.append(DunLast, &cpy[offset], c);
						offset += c;
						//补牌后判断单墩牌型是否变化
						HandTy ty = GetDunCardHandTy(DunLast, group.duns[DunLast].cards, group.duns[DunLast].GetC());
						if (ty != group.duns[DunLast].ty_) {
							break;
						}
						//补牌后判断是否倒水
						if (IsInverted(DunLast, group.duns[DunLast].cards, group.duns[DunLast].GetC(), group.duns[DunLast].ty_, &group)) {
							break;
						}
					}
				}
				//补充中墩
				{
					int c = group.needC(DunSecond);
					if (c > 0) {
						//跳过补牌后出现牌型变化情况
						{
							for (int i = 0; i < c; ++i) {
								uint8_t r = cpy[offset + i];
								for (int j = 0; j < group.duns[DunSecond].GetC(); ++j) {
									if (CGameLogic::GetCardValue(r) == CGameLogic::GetCardValue(group.duns[DunSecond].cards[j])) {
										continueflag = true;
										goto continue_;
									}
								}
							}
						}
						assert(offset < cpylen);
						group.append(DunSecond, &cpy[offset], c);
						offset += c;
						//补牌后判断单墩牌型是否变化
						HandTy ty = GetDunCardHandTy(DunSecond, group.duns[DunSecond].cards, group.duns[DunSecond].GetC());
						if (ty != group.duns[DunSecond].ty_) {
							break;
						}
						//补牌后判断是否倒水
						if (IsInverted(DunSecond, group.duns[DunSecond].cards, group.duns[DunSecond].GetC(), group.duns[DunSecond].ty_, &group)) {
							break;
						}
					}
				}
				//补充头墩
				{
					int c = group.needC(DunFirst);
					if (c > 0) {
						//跳过补牌后出现牌型变化情况
						{
							for (int i = 0; i < c; ++i) {
								uint8_t r = cpy[offset + i];
								for (int j = 0; j < group.duns[DunFirst].GetC(); ++j) {
									if (CGameLogic::GetCardValue(r) == CGameLogic::GetCardValue(group.duns[DunFirst].cards[j])) {
										continueflag = true;
										goto continue_;
									}
								}
							}
						}
						assert(offset < cpylen);
						group.append(DunFirst, &cpy[offset], c);
						offset += c;
						assert(offset == cpylen);
						//补牌后判断单墩牌型是否变化
						HandTy ty = GetDunCardHandTy(DunFirst, group.duns[DunFirst].cards, group.duns[DunFirst].GetC());
						if (ty == Ty123sc || ty == Tysc || ty == Ty123) {
							ty = Tysp;
						}
						if (ty != group.duns[DunFirst].ty_) {
							break;
						}
						//补牌后判断是否倒水
						if (IsInverted(DunFirst, group.duns[DunFirst].cards, group.duns[DunFirst].GetC(), group.duns[DunFirst].ty_, &group)) {
							break;
						}
					}
				}
				//group.PrintCardList(DunFirst);
				//group.PrintCardList(DunSecond);
				//group.PrintCardList(DunLast);
				//该组是特殊牌型(三同花顺/三顺子/三同花)
				if (group.specialTy == TyThree123sc || group.specialTy == TyThree123 || group.specialTy == TyThreesc) {
					//如果是三顺子，除去三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					HandTy specialTy = group.specialTy;
					//当作普通牌型，头墩散牌
					group.specialTy = TyNil;
					group.duns[DunFirst].ty_ = Tysp;
					enum_groups_tmp.push_back(group);
					//特殊牌型放在枚举几组最优解前面
					group.specialTy = specialTy;
					group.duns[DunFirst].ty_ = group.specialTy;
					group.duns[DunSecond].ty_ = group.specialTy;
					group.duns[DunLast].ty_ = group.specialTy;
					spec_groups.push_back(group);
				}
				else {
					bool existRepeated = false;
					for (std::vector<groupdun_t>::const_iterator it = enum_groups_tmp.begin();
						it != enum_groups_tmp.end(); ++it) {
						if (CGameLogic::CompareCards(
							it->duns[DunLast].cards, it->duns[DunLast].c, it->duns[DunLast].ty_,
							group.duns[DunLast].cards, group.duns[DunLast].c, group.duns[DunLast].ty_, false, true) == 0 &&
							CGameLogic::CompareCards(
								it->duns[DunSecond].cards, it->duns[DunSecond].c, it->duns[DunSecond].ty_,
								group.duns[DunSecond].cards, group.duns[DunSecond].c, group.duns[DunSecond].ty_, false, true) == 0 &&
							CGameLogic::CompareCards(
								it->duns[DunFirst].cards, it->duns[DunFirst].c, it->duns[DunFirst].ty_,
								group.duns[DunFirst].cards, group.duns[DunFirst].c, group.duns[DunFirst].ty_, false, true) == 0) {
							existRepeated = true;
							break;
						}
					}
					if (!existRepeated && enum_groups_tmp.size() < MAX_ENUMSZ) {
						enum_groups_tmp.push_back(group);
					}
				}
				break;
			}
			}
		}
		//至尊青龙/一条龙(十三水)/十二皇族
		if (specialTy == TyZZQDragon || specialTy == TyOneDragon || specialTy == Ty12Royal) {
			CGameLogic::groupdun_t group;
			group.specialTy = specialTy;
			group.assign(DunFirst, group.specialTy, src, group.needC(DunFirst));
			group.assign(DunSecond, group.specialTy, src + group.GetC(), group.needC(DunSecond));
			group.assign(DunLast, group.specialTy, src + group.GetC(), group.needC(DunLast));
			assert(spec_groups.size() == 0);
			spec_groups.push_back(group);
		}
		//三同花顺
		else if (specialTy == TyThree123sc) {
			assert(spec_groups.size() == 1);
		}
		else {
			if (classify.c4 == 3) {
				//三分天下(三套炸弹)
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c4; ++i) {
						memcpy(cards + k, &(classify.dst4[i])[0], 4);
						k += 4;
					}
					assert(classify.cpylen == 1);
					memcpy(cards + k, classify.cpy, classify.cpylen);
					k += classify.cpylen;
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TyThree40;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
			else if (CheckAllBig(src, len) == TyAllBig) {
				//全大/牌值从小到大
				CGameLogic::groupdun_t group;
				group.specialTy = TyAllBig;
				group.assign(DunFirst, group.specialTy, src, group.needC(DunFirst));
				group.assign(DunSecond, group.specialTy, src + group.GetC(), group.needC(DunSecond));
				group.assign(DunLast, group.specialTy, src + group.GetC(), group.needC(DunLast));
				//除去三顺子/三同花
				if (spec_groups.size() > 0) {
					assert(spec_groups.size() == 1);
					spec_groups.pop_back();
				}
				spec_groups.push_back(group);
			}
			else if (CheckAllSmall(src, len) == TyAllSmall) {
				//全小/牌值从小到大
				CGameLogic::groupdun_t group;
				group.specialTy = TyAllSmall;
				group.assign(DunFirst, group.specialTy, src, group.needC(DunFirst));
				group.assign(DunSecond, group.specialTy, src + group.GetC(), group.needC(DunSecond));
				group.assign(DunLast, group.specialTy, src + group.GetC(), group.needC(DunLast));
				//除去三顺子/三同花
				if (spec_groups.size() > 0) {
					assert(spec_groups.size() == 1);
					spec_groups.pop_back();
				}
				spec_groups.push_back(group);
			}
			else if (CheckAllOneColor(src, len) == TyAllOneColor) {
				//凑一色：全是红牌(方块/红心)或黑牌(黑桃/梅花)
				CGameLogic::groupdun_t group;
				group.specialTy = TyAllOneColor;
				group.assign(DunFirst, group.specialTy, src, group.needC(DunFirst));
				group.assign(DunSecond, group.specialTy, src + group.GetC(), group.needC(DunSecond));
				group.assign(DunLast, group.specialTy, src + group.GetC(), group.needC(DunLast));
				//除去三顺子/三同花
				if (spec_groups.size() > 0) {
					assert(spec_groups.size() == 1);
					spec_groups.pop_back();
				}
				spec_groups.push_back(group);
			}
#if 0 //先屏蔽掉双怪冲三
			else if (classify.c3 == 2 && classify.c2 == 3) {
				//双怪冲三
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c3; ++i) {
						memcpy(cards + k, &(classify.dst3[i])[0], 3);
						k += 3;
					}
					for (int i = 0; i < classify.c2; ++i) {
						memcpy(cards + k, &(classify.dst2[i])[0], 2);
						k += 2;
					}
					assert(classify.cpylen == 1);
					memcpy(cards + k, classify.cpy, classify.cpylen);
					k += classify.cpylen;
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TyTwo3220;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
#endif
			else if (classify.c3 == 4) {
				//四套三条(四套冲三)
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c3; ++i) {
						memcpy(cards + k, &(classify.dst3[i])[0], 3);
						k += 3;
					}
					assert(classify.cpylen == 1);
					memcpy(cards + k, classify.cpy, classify.cpylen);
					k += classify.cpylen;
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TyFour30;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
			//三个三条加上一套炸弹
			else if (classify.c3 == 3 && classify.c4 == 1) {
				//四套三条(四套冲三)
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c3; ++i) {
						memcpy(cards + k, &(classify.dst3[i])[0], 3);
						k += 3;
					}
					for (int i = 0; i < classify.c4; ++i) {
						memcpy(cards + k, &(classify.dst4[i])[0], 4);
						k += 4;
					}
					assert(classify.cpylen == 0);
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TyFour30;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
			else if (classify.c2 == 5 && classify.c3 == 1) {
				//五对三条(五对冲三)
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c3; ++i) {
						memcpy(cards + k, &(classify.dst3[i])[0], 3);
						k += 3;
					}
					for (int i = 0; i < classify.c2; ++i) {
						memcpy(cards + k, &(classify.dst2[i])[0], 2);
						k += 2;
					}
					assert(classify.cpylen == 0);
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TyFive2030;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
			else if (classify.c2 == 6) {
				//六对半
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c2; ++i) {
						memcpy(cards + k, &(classify.dst2[i])[0], 2);
						k += 2;
					}
					assert(classify.cpylen == 1);
					memcpy(cards + k, classify.cpy, classify.cpylen);
					k += classify.cpylen;
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TySix20;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
			//四个对子加上一套炸弹加上一张单张
			else if (classify.c2 == 4 && classify.c4 == 1) {
				//六对半
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c2; ++i) {
						memcpy(cards + k, &(classify.dst2[i])[0], 2);
						k += 2;
					}
					for (int i = 0; i < classify.c4; ++i) {
						memcpy(cards + k, &(classify.dst4[i])[0], 4);
						k += 4;
					}
					assert(classify.cpylen == 1);
					memcpy(cards + k, classify.cpy, classify.cpylen);
					k += classify.cpylen;
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TySix20;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
			//两个对子加上两套炸弹加上一张单张
			else if (classify.c2 == 2 && classify.c4 == 2) {
				//六对半
				uint8_t cards[MAX_COUNT] = { 0 };
				{
					int k = 0;
					for (int i = 0; i < classify.c2; ++i) {
						memcpy(cards + k, &(classify.dst2[i])[0], 2);
						k += 2;
					}
					for (int i = 0; i < classify.c4; ++i) {
						memcpy(cards + k, &(classify.dst4[i])[0], 4);
						k += 4;
					}
					assert(classify.cpylen == 1);
					memcpy(cards + k, classify.cpy, classify.cpylen);
					k += classify.cpylen;
					assert(k == MAX_COUNT);
				}
				{
					CGameLogic::groupdun_t group;
					group.specialTy = TySix20;
					group.assign(DunFirst, group.specialTy, cards, group.needC(DunFirst));
					group.assign(DunSecond, group.specialTy, cards + group.GetC(), group.needC(DunSecond));
					group.assign(DunLast, group.specialTy, cards + group.GetC(), group.needC(DunLast));
					//除去三顺子/三同花
					if (spec_groups.size() > 0) {
						assert(spec_groups.size() == 1);
						spec_groups.pop_back();
					}
					spec_groups.push_back(group);
				}
			}
		}
		if (enum_groups_tmp.size() == 0) {
			CGameLogic::PrintCardList(src, len);
		}
		assert(enum_groups_tmp.size() > 0);
		assert(enum_groups.size() == 0);
		std::vector<groupdun_t const*> v;
#if 1
		//尾墩最大/中墩最大/头墩最大
		groupdun_t const* maxLast = NULL, *maxSecond = NULL, *maxFirst = NULL;
		for (std::vector<groupdun_t>::const_iterator it = enum_groups_tmp.begin();
			it != enum_groups_tmp.end(); ++it) {
			//const_cast<groupdun_t&>(*it).PrintCardList(DunFirst);
			//const_cast<groupdun_t&>(*it).PrintCardList(DunSecond);
			//const_cast<groupdun_t&>(*it).PrintCardList(DunLast);
			if (it == enum_groups_tmp.begin()) {
				maxFirst = maxSecond = maxLast = &*it;
			}
			else {
				{
					//不比较散牌/单张
					//不比较次大的对子(两对之间)
					//不比较葫芦的对子(葫芦之间)
					int bv = CGameLogic::CompareCards(
						it->duns[DunSecond].cards, it->duns[DunSecond].c, it->duns[DunSecond].ty_,
						maxSecond->duns[DunSecond].cards, maxSecond->duns[DunSecond].c, maxSecond->duns[DunSecond].ty_, false, false, false, false);
					if (bv > 0) {
						maxSecond = &*it;
					}
					else if (bv == 0) {
						//没有赋值则赋值当前
						if (maxSecond == maxLast) {
							maxSecond = &*it;
						}
					}
				}
				{
					//不比较散牌/单张
					//不比较次大的对子(两对之间)
					//不比较葫芦的对子(葫芦之间)
					int bv = CGameLogic::CompareCards(
						it->duns[DunFirst].cards, it->duns[DunFirst].c, it->duns[DunFirst].ty_,
						maxFirst->duns[DunFirst].cards, maxFirst->duns[DunFirst].c, maxFirst->duns[DunFirst].ty_, false, false, false, false);
					if (bv > 0) {
						maxFirst = &*it;
					}
					else if (bv == 0) {
						//没有赋值则赋值当前
						//指向maxSecond则赋值当前
						if (maxFirst == maxLast || maxFirst == maxSecond) {
							maxFirst = &*it;
						}
					}
				}
			}
		}
		//尾墩最大
		if (maxLast) {
			v.push_back(maxLast);
		}
		//中墩最大
		if (maxSecond && maxSecond != maxLast) {
			v.push_back(maxSecond);
		}
		//头墩最大
		if (maxFirst && maxFirst != maxSecond && maxFirst != maxLast) {
			v.push_back(maxFirst);
		}
#else
		for (std::vector<groupdun_t>::const_iterator it = enum_groups_tmp.begin();
			it != enum_groups_tmp.end(); ++it) {
			if (v.size() >= MAX_ENUMSZ) {
				break;
			}
			v.push_back(&*it);
		}
#endif
		//按输赢得水排序(不计打枪/全垒打)
		std::sort(&v.front(), &v.front() + v.size(), GlobalCompareCardsByScore);
		for (std::vector<groupdun_t const*>::const_iterator it = v.begin();
			it != v.end(); ++it) {
			enum_groups.push_back(**it);
		}
		assert(enum_groups.size() > 0);
		assert(groups.size() == 0);
		//特殊牌型
		for (std::vector<groupdun_t>::const_iterator it = spec_groups.begin();
			it != spec_groups.end(); ++it) {
			assert(spec_groups.size() == 1);
			groups.push_back(&*it);
		}
		//枚举牌型(几组最优解)
		for (std::vector<groupdun_t>::const_iterator it = enum_groups.begin();
			it != enum_groups.end(); ++it) {
			groups.push_back(&*it);
		}
		assert(groups.size() > 0);
	}
	
	//src与dst两组之间按输赢得水比大小(不计打枪/全垒打)
	//src uint8_t const* 一组牌(13张)
	//dst uint8_t const* 一组牌(13张)
	//sp bool 是否比较散牌/单张
	//sd bool 是否比较次大的对子(两对之间比较)
	//dz bool 是否比较葫芦的对子(葫芦之间比较)
	int CGameLogic::CompareCardsByScore(groupdun_t const* psrc, groupdun_t const* pdst, bool sp, bool sd, bool dz) {
		std::vector<groupdun_t const*> groups;
		//groups[0]指向psrc
		groups.push_back(psrc);
		//groups[1]指向pdst
		groups.push_back(pdst);
		s13s::CMD_S_CompareCards compareCards[2];
		int c = 0;
		int chairIDs[MaxEnumSZ] = { 0 };
		for (int i = 0; i < groups.size(); ++i) {
			//用于求两两组合
			chairIDs[c++] = i;
			//比牌玩家桌椅ID
			compareCards[i].mutable_player()->set_chairid(i);
			//桌椅玩家选择一组墩(含头墩/中墩/尾墩)
			s13s::GroupDunData* player_group = compareCards[i].mutable_player()->mutable_group();
			//从哪墩开始的
			player_group->set_start(S13S::DunFirst);
			//总体对应特殊牌型 ////////////
			player_group->set_specialty(groups[i]->specialTy);
			//获取座椅玩家确定的三墩牌型
			groupdun_t const *player_select = groups[i];
			//compareCards[i]对应groups[i]
			const_cast<groupdun_t*&>(groups[i])->deltascore = 0;
		}
		std::vector<std::vector<int>> vec;
		CFuncC fnC;
		fnC.FuncC(c, 2, vec);
		//CFuncC::Print(vec);
		for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
			it != vec.end(); ++it) {
			assert(it->size() == 2 && (*it)[0] > (*it)[1]);//两两比牌
			int src_chairid = chairIDs[(*it)[0]];//chairIDs[1] pdst椅子ID
			int dst_chairid = chairIDs[(*it)[1]];//chairIDs[0] psrc椅子ID
			assert(src_chairid < groups.size());
			assert(dst_chairid < groups.size());
			//获取src确定的三墩牌型
			S13S::CGameLogic::groupdun_t const *src = groups[src_chairid]; assert(src == pdst);//[1]指向pdst
			//获取dst确定的三墩牌型
			S13S::CGameLogic::groupdun_t const *dst = groups[dst_chairid]; assert(dst == psrc);//[0]指向psrc
			{
				//追加src比牌对象 ////////////
				s13s::ComparePlayer* src_peer = compareCards[src_chairid].add_peers();
				{
					//比牌对象桌椅ID
					src_peer->set_chairid(dst_chairid);
					//比牌对象选择一组墩
					s13s::GroupDunData* src_peer_select = src_peer->mutable_group();
					//从哪墩开始的
					src_peer_select->set_start(S13S::DunFirst);
					//总体对应特殊牌型 ////////////
					src_peer_select->set_specialty(dst->specialTy);
				}
				//追加src比牌结果 ////////////
				s13s::CompareResult* src_result = compareCards[src_chairid].add_results();
			}
			{
				//追加dst比牌对象 ////////////
				s13s::ComparePlayer* dst_peer = compareCards[dst_chairid].add_peers();
				{
					//比牌对象桌椅ID
					dst_peer->set_chairid(src_chairid);
					//比牌对象选择一组墩
					s13s::GroupDunData* dst_peer_select = dst_peer->mutable_group();
					//从哪墩开始的
					dst_peer_select->set_start(S13S::DunFirst);
					//总体对应特殊牌型 ////////////
					dst_peer_select->set_specialty(src->specialTy);
				}
				//追加dst比牌结果 ////////////
				s13s::CompareResult* dst_result = compareCards[dst_chairid].add_results();
			}
			//////////////////////////////////////////////////////////////
			//比较特殊牌型
			if (src->specialTy >= TyThreesc || dst->specialTy >= TyThreesc) {
				int winner = -1, loser = -1;
				if (src->specialTy != dst->specialTy) {
					//牌型不同比牌型
					if (src->specialTy > dst->specialTy) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (src->specialTy < dst->specialTy) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						assert(false);
					}
				}
				else {
					//牌型相同，和了
				}
				if (winner == -1) {
					//和了
					winner = src_chairid; loser = dst_chairid;
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//src比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//src输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//dst比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//dst输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//至尊青龙获胜赢32水
				else if (groups[winner]->specialTy == TyZZQDragon) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(32);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-32);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//一条龙获胜赢30水
				else if (groups[winner]->specialTy == TyOneDragon) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(30);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-30);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//十二皇族获胜赢24水
				else if (groups[winner]->specialTy == Ty12Royal) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(24);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-24);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//三同花顺获胜赢20水
				else if (groups[winner]->specialTy == TyThree123sc) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(20);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-20);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//三分天下获胜赢20水
				else if (groups[winner]->specialTy == TyThree40) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(20);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-20);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//全大获胜赢10水
				else if (groups[winner]->specialTy == TyAllBig) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(10);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-10);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//全小获胜赢10水
				else if (groups[winner]->specialTy == TyAllSmall) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(10);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-10);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//凑一色获胜赢10水
				else if (groups[winner]->specialTy == TyAllOneColor) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(10);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-10);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//双怪冲三获胜赢8水
				else if (groups[winner]->specialTy == TyTwo3220) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(8);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-8);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//四套三条获胜赢6水
				else if (groups[winner]->specialTy == TyFour30) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(6);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-6);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//五对三条获胜赢5水
				else if (groups[winner]->specialTy == TyFive2030) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(5);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-5);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//六对半获胜赢4水
				else if (groups[winner]->specialTy == TySix20) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(4);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-4);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//三顺子获胜赢4水
				else if (groups[winner]->specialTy == TyThree123) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//赢家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(4);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-4);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				//三同花获胜赢3水
				else if (groups[winner]->specialTy == TyThreesc) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//头墩输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(3);
							//赢的牌型
							item->set_ty(groups[winner]->specialTy);
							//对方牌型
							item->set_peerty(groups[loser]->specialTy);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//输家输赢信息
						s13s::CompareItem* item = result->mutable_specitem();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-3);
							//输的牌型
							item->set_ty(groups[loser]->specialTy);
							//对方牌型
							item->set_peerty(groups[winner]->specialTy);
						}
					}
				}
				continue;
			}
			//////////////////////////////////////////////////////////////
			//比较头墩
			{
				//单墩比牌，头墩要么三条/对子/乌龙，同花顺/同花/顺子都要改成乌龙
				if (src->duns[S13S::DunFirst].ty_ == S13S::Ty123sc ||
					src->duns[S13S::DunFirst].ty_ == S13S::Tysc ||
					src->duns[S13S::DunFirst].ty_ == S13S::Ty123) {
					//assert(src->duns[S13S::DunFirst].ty_ == S13S::Tysp);
					const_cast<S13S::CGameLogic::groupdun_t*>(src)->duns[S13S::DunFirst].ty_ = S13S::Tysp;
				}
				//单墩比牌，头墩要么三条/对子/乌龙，同花顺/同花/顺子都要改成乌龙
				if (dst->duns[S13S::DunFirst].ty_ == S13S::Ty123sc ||
					dst->duns[S13S::DunFirst].ty_ == S13S::Tysc ||
					dst->duns[S13S::DunFirst].ty_ == S13S::Ty123) {
					//assert(dst->duns[S13S::DunFirst].ty_ == S13S::Tysp);
					const_cast<S13S::CGameLogic::groupdun_t*>(dst)->duns[S13S::DunFirst].ty_ = S13S::Tysp;
				}
				int winner = -1, loser = -1;
				int dt = S13S::DunFirst;
				if (src->duns[dt].ty_ != dst->duns[dt].ty_) {
					//牌型不同比牌型
					if (src->duns[dt].ty_ > dst->duns[dt].ty_) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (src->duns[dt].ty_ < dst->duns[dt].ty_) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						assert(false);
					}
				}
				else {
					//牌型相同比大小
					assert(src->duns[dt].GetC() == 3);
					assert(dst->duns[dt].GetC() == 3);
					int bv = S13S::CGameLogic::CompareCards(
						src->duns[dt].cards, dst->duns[dt].cards, dst->duns[dt].GetC(), false, dst->duns[dt].ty_, sp, sd, dz);
					if (bv > 0) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (bv < 0) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						//头墩和
						//assert(false);
					}
				}
				if (winner == -1) {
					//头墩和
					winner = src_chairid; loser = dst_chairid;
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//src比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//头墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//dst比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//头墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				//乌龙/对子获胜赢1水
				else if (groups[winner]->duns[dt].ty_ == S13S::Tysp ||
						groups[winner]->duns[dt].ty_ == S13S::Ty20) {
						{
							int index = compareCards[winner].results_size() - 1;
							assert(index >= 0);
							//赢家比牌结果 ////////////
							s13s::CompareResult* result = compareCards[winner].mutable_results(index);
							//头墩输赢信息
							s13s::CompareItem* item = result->add_items();
							{
								//赢了
								item->set_winlost(1);
								//赢分
								item->set_score(1);
								//赢的牌型
								item->set_ty(groups[winner]->duns[dt].ty_);
								//对方牌型
								item->set_peerty(groups[loser]->duns[dt].ty_);
							}
						}
						{
							int index = compareCards[loser].results_size() - 1;
							assert(index >= 0);
							//输家比牌结果 ////////////
							s13s::CompareResult* result = compareCards[loser].mutable_results(index);
							//头墩输赢信息
							s13s::CompareItem* item = result->add_items();
							{
								//输了
								item->set_winlost(-1);
								//输分
								item->set_score(-1);
								//输的牌型
								item->set_ty(groups[loser]->duns[dt].ty_);
								//对方牌型
								item->set_peerty(groups[winner]->duns[dt].ty_);
							}
						}
				}
				//三条摆头墩获胜赢3水
				else if (groups[winner]->duns[dt].ty_ == S13S::Ty30) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//头墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(3);
							//赢的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//头墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-3);
							//输的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				else {
					assert(false);
				}
			}
			//////////////////////////////////////////////////////////////
			//比较中墩
			{
				int winner = -1, loser = -1;
				int dt = S13S::DunSecond;
				if (src->duns[dt].ty_ != dst->duns[dt].ty_) {
					//牌型不同比牌型
					if (src->duns[dt].ty_ > dst->duns[dt].ty_) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (src->duns[dt].ty_ < dst->duns[dt].ty_) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						assert(false);
					}
				}
				else {
					//牌型相同比大小
					assert(src->duns[dt].GetC() == 5);
					assert(dst->duns[dt].GetC() == 5);
					int bv = S13S::CGameLogic::CompareCards(
						src->duns[dt].cards, dst->duns[dt].cards, dst->duns[dt].GetC(), false, dst->duns[dt].ty_, sp, sd, dz);
					if (bv > 0) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (bv < 0) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						//中墩和
						//assert(false);
					}
				}
				if (winner == -1) {
					//中墩和
					winner = src_chairid; loser = dst_chairid;
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//src比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//dst比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				//乌龙/对子/两对/三条/顺子/同花获胜赢1水
				else if (groups[winner]->duns[dt].ty_ == S13S::Tysp ||
						groups[winner]->duns[dt].ty_ == S13S::Ty20 ||
						groups[winner]->duns[dt].ty_ == S13S::Ty22 ||
						groups[winner]->duns[dt].ty_ == S13S::Ty30 ||
						groups[winner]->duns[dt].ty_ == S13S::Ty123 ||
						groups[winner]->duns[dt].ty_ == S13S::Tysc) {
						{
							int index = compareCards[winner].results_size() - 1;
							assert(index >= 0);
							//赢家比牌结果 ////////////
							s13s::CompareResult* result = compareCards[winner].mutable_results(index);
							//中墩输赢信息
							s13s::CompareItem* item = result->add_items();
							{
								//赢了
								item->set_winlost(1);
								//赢分
								item->set_score(1);
								//赢的牌型
								item->set_ty(groups[winner]->duns[dt].ty_);
								//对方牌型
								item->set_peerty(groups[loser]->duns[dt].ty_);
							}
						}
						{
							int index = compareCards[loser].results_size() - 1;
							assert(index >= 0);
							//输家比牌结果 ////////////
							s13s::CompareResult* result = compareCards[loser].mutable_results(index);
							//中墩输赢信息
							s13s::CompareItem* item = result->add_items();
							{
								//输了
								item->set_winlost(-1);
								//输分
								item->set_score(-1);
								//输的牌型
								item->set_ty(groups[loser]->duns[dt].ty_);
								//对方牌型
								item->set_peerty(groups[winner]->duns[dt].ty_);
							}
						}
				}
				//葫芦摆中墩获胜赢2水
				else if (groups[winner]->duns[dt].ty_ == S13S::Ty32) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(2);
							//赢的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-2);
							//输的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				//铁支摆中墩获胜赢8水
				else if (groups[winner]->duns[dt].ty_ == S13S::Ty40) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(8);
							//赢的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-8);
							//输的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				//同花顺摆中墩获胜赢10水
				else if (groups[winner]->duns[dt].ty_ == S13S::Ty123sc) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(10);
							//赢的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//中墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-10);
							//输的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				else {
					assert(false);
				}
			}
			//////////////////////////////////////////////////////////////
			//比较尾墩
			{
				int winner = -1, loser = -1;
				int dt = S13S::DunLast;
				if (src->duns[dt].ty_ != dst->duns[dt].ty_) {
					//牌型不同比牌型
					if (src->duns[dt].ty_ > dst->duns[dt].ty_) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (src->duns[dt].ty_ < dst->duns[dt].ty_) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						assert(false);
					}
				}
				else {
					//牌型相同比大小
					assert(src->duns[dt].GetC() == 5);
					assert(dst->duns[dt].GetC() == 5);
					int bv = S13S::CGameLogic::CompareCards(
						src->duns[dt].cards, dst->duns[dt].cards, dst->duns[dt].GetC(), false, dst->duns[dt].ty_, sp, sd, dz);
					if (bv > 0) {
						winner = src_chairid; loser = dst_chairid;
					}
					else if (bv < 0) {
						winner = dst_chairid; loser = src_chairid;
					}
					else {
						//尾墩和
						//assert(false);
					}
				}
				if (winner == -1) {
					//尾墩和
					winner = src_chairid; loser = dst_chairid;
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//src比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//尾墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//dst比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//尾墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//和了
							item->set_winlost(0);
							//和分
							item->set_score(0);
							//和的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				//乌龙/对子/两对/三条/顺子/同花/葫芦获胜赢1水
				else if (groups[winner]->duns[dt].ty_ == S13S::Tysp ||
						groups[winner]->duns[dt].ty_ == S13S::Ty20 ||
						groups[winner]->duns[dt].ty_ == S13S::Ty22 ||
						groups[winner]->duns[dt].ty_ == S13S::Ty30 ||
						groups[winner]->duns[dt].ty_ == S13S::Ty123 ||
						groups[winner]->duns[dt].ty_ == S13S::Tysc ||
						groups[winner]->duns[dt].ty_ == S13S::Ty32) {
						{
							int index = compareCards[winner].results_size() - 1;
							assert(index >= 0);
							//赢家比牌结果 ////////////
							s13s::CompareResult* result = compareCards[winner].mutable_results(index);
							//尾墩输赢信息
							s13s::CompareItem* item = result->add_items();
							{
								//赢了
								item->set_winlost(1);
								//赢分
								item->set_score(1);
								//赢的牌型
								item->set_ty(groups[winner]->duns[dt].ty_);
								//对方牌型
								item->set_peerty(groups[loser]->duns[dt].ty_);
							}
						}
						{
							int index = compareCards[loser].results_size() - 1;
							assert(index >= 0);
							//输家比牌结果 ////////////
							s13s::CompareResult* result = compareCards[loser].mutable_results(index);
							//尾墩输赢信息
							s13s::CompareItem* item = result->add_items();
							{
								//输了
								item->set_winlost(-1);
								//输分
								item->set_score(-1);
								//输的牌型
								item->set_ty(groups[loser]->duns[dt].ty_);
								//对方牌型
								item->set_peerty(groups[winner]->duns[dt].ty_);
							}
						}
				}
				//铁支摆尾墩获胜赢4水
				else if (groups[winner]->duns[dt].ty_ == S13S::Ty40) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//尾墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(4);
							//赢的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//尾墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-4);
							//输的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				//同花顺摆尾墩获胜赢5水
				else if (groups[winner]->duns[dt].ty_ == S13S::Ty123sc) {
					{
						int index = compareCards[winner].results_size() - 1;
						assert(index >= 0);
						//赢家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[winner].mutable_results(index);
						//尾墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//赢了
							item->set_winlost(1);
							//赢分
							item->set_score(5);
							//赢的牌型
							item->set_ty(groups[winner]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[loser]->duns[dt].ty_);
						}
					}
					{
						int index = compareCards[loser].results_size() - 1;
						assert(index >= 0);
						//输家比牌结果 ////////////
						s13s::CompareResult* result = compareCards[loser].mutable_results(index);
						//尾墩输赢信息
						s13s::CompareItem* item = result->add_items();
						{
							//输了
							item->set_winlost(-1);
							//输分
							item->set_score(-5);
							//输的牌型
							item->set_ty(groups[loser]->duns[dt].ty_);
							//对方牌型
							item->set_peerty(groups[winner]->duns[dt].ty_);
						}
					}
				}
				else {
					{
						assert(false);
					}
				}
			}
		}
		//玩家对其它玩家打枪
		std::map<uint8_t, std::vector<uint8_t>> shootIds;
		//统计判断打枪/全垒打
		for (int i = 0; i < groups.size(); ++i) {
			if (true) {
				//判断是否全垒打
				int shootc = 0;
				//当前比牌玩家
				s13s::PlayerItem const& player = compareCards[i].player();
				//遍历玩家所有比牌对象
				for (int j = 0; j < compareCards[i].peers_size(); ++j) {
					s13s::ComparePlayer const& peer = compareCards[i].peers(j);
					s13s::CompareResult const& result = compareCards[i].results(j);
					int winc = 0, lostc = 0, sumscore = 0;
					//自己特殊牌型或对方特殊牌型
					assert(result.items_size() == (
						player.group().specialty() >= S13S::TyThreesc ||
						peer.group().specialty() >= S13S::TyThreesc) ? 0 : 3);
					//玩家与当前比牌对象比头/中/尾三墩输赢得水总分，不考虑打枪
					for (int d = 0; d < result.items_size(); ++d) {
						if (result.items(d).winlost() == 1) {
							++winc;
						}
						else if (result.items(d).winlost() == -1) {
							++lostc;
						}
						sumscore += result.items(d).score();
					}
					//三墩不考虑打枪输赢得水总分 赢分+/和分0/输分-
					const_cast<s13s::CompareResult&>(result).set_score(sumscore);
					//特殊牌型不参与打枪
					if (player.group().specialty() >= S13S::TyThreesc ||
						peer.group().specialty() >= S13S::TyThreesc) {
						const_cast<s13s::CompareResult&>(result).set_shoot(0);//-1被打枪/0不打枪/1打枪
						continue;
					}
					if (winc == result.items_size()) {
						//玩家三墩全部胜过比牌对象，则玩家对比牌对象打枪，中枪者付给打枪者2倍的水
						const_cast<s13s::CompareResult&>(result).set_shoot(1);//-1被打枪/0不打枪/1打枪
						//统计当前玩家打枪次数
						++shootc;
						//玩家对比牌对象打枪
						shootIds[i].push_back(peer.chairid());
					}
					else if (lostc == result.items_size()) {
						//比牌对象三墩全部胜过玩家，则比牌对象对玩家打枪，中枪者付给打枪者2倍的水
						const_cast<s13s::CompareResult&>(result).set_shoot(-1);//-1被打枪/0不打枪/1打枪
					}
					else {
						const_cast<s13s::CompareResult&>(result).set_shoot(0);//-1被打枪/0不打枪/1打枪
					}
				}
				if (shootc == compareCards[i].peers_size() && compareCards[i].peers_size() > 1) {
					//全垒打，玩家三墩全部胜过其它玩家，且至少打2枪，中枪者付给打枪者4倍的水
					compareCards[i].set_allshoot(1);//-1被全垒打/0无全垒打/1全垒打
					//其它比牌对象都是被全垒打
					for (int k = 0; k < groups.size(); ++k) {
						if (k != i) {
							if (true) {
								//-1被全垒打/0无全垒打/1全垒打
								compareCards[k].set_allshoot(-1);
								//allshoot=-1被全垒打，记下全垒打桌椅ID
								compareCards[k].set_fromchairid(i);
							}
						}
					}
				}
			}
		}
		//其它玩家之间打枪
		for (int i = 0; i < groups.size(); ++i) {
			if (true) {
				//当前比牌玩家
				s13s::PlayerItem const& player = compareCards[i].player();
				//遍历玩家所有比牌对象
				for (int j = 0; j < compareCards[i].peers_size(); ++j) {
					s13s::ComparePlayer const& peer = compareCards[i].peers(j);
					//s13s::CompareResult const& result = compareCards[i].results(j);
					std::map<uint8_t, std::vector<uint8_t>>::const_iterator it = shootIds.find(peer.chairid());
					if (it != shootIds.end()) {
						for (std::vector<uint8_t>::const_iterator ir = it->second.begin();
							ir != it->second.end(); ++ir) {
							//排除当前玩家
							if (*ir != i) {
								assert(player.chairid() == i);
								const_cast<s13s::ComparePlayer&>(peer).add_shootids(*ir);
							}
						}
					}
				}
			}
		}
		//计算包括打枪/全垒打在内输赢得水总分
		for (int i = 0; i < groups.size(); ++i) {
			if (true) {
				//玩家输赢得水总分，不含特殊牌型输赢分
				int deltascore = 0;
				//遍历玩家所有比牌对象
				for (int j = 0; j < compareCards[i].peers_size(); ++j) {
					s13s::ComparePlayer const& peer = compareCards[i].peers(j);
					s13s::CompareResult const& result = compareCards[i].results(j);
					//1打枪(对当前比牌对象打枪)
					if (result.shoot() == 1) {
						//1全垒打
						if (compareCards[i].allshoot() == 1) {
							//-1被全垒打/0无全垒打/1全垒打
							deltascore += 4 * result.score();
						}
						else {
							//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
							if (compareCards[i].allshoot() == -1) {
							}
							else {
							}
							//-1被打枪/0不打枪/1打枪
							deltascore += 2 * result.score();
						}
					}
					//-1被打枪(被当前比牌对象打枪)
					else if (result.shoot() == -1) {
						//-1被全垒打
						if (compareCards[i].allshoot() == -1) {
							//被当前比牌对象全垒打
							if (peer.chairid() == compareCards[i].fromchairid()) {
								//-1被全垒打/0无全垒打/1全垒打
								deltascore += 4 * result.score();
							}
							//被另外比牌对象全垒打
							else {
								//-1被打枪/0不打枪/1打枪
								deltascore += 2 * result.score();
							}
						}
						else {
							//-1被打枪/0不打枪/1打枪
							deltascore += 2 * result.score();
						}
					}
					//0不打枪(与当前比牌对象互不打枪)
					else {
						//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
						if (compareCards[i].allshoot() == -1) {
						}
						else {
							//一定不是全垒打，-1被打枪/0不打枪/1打枪
							assert(compareCards[i].allshoot() == 0);
						}
						assert(result.shoot() == 0);
						deltascore += result.score();
					}
				}
				//玩家输赢得水总分，不含特殊牌型输赢分
				compareCards[i].set_deltascore(deltascore);
				//compareCards[i]对应groups[i]
				const_cast<groupdun_t*&>(groups[i])->deltascore = compareCards[i].deltascore();
				//printf("玩家[%d]输赢得水总分[%d]，不含特殊牌型输赢分\n", i, compareCards[i].deltascore());
			}
		}
		//座椅玩家与其余玩家按墩比输赢分
		//不含打枪/全垒打与打枪/全垒打分开计算
		//计算特殊牌型输赢分
		//统计含打枪/全垒打/特殊牌型输赢得水总分
		for (int i = 0; i < groups.size(); ++i) {
			if (true) {

				//三墩打枪/全垒打输赢算水
				int allshootscore = 0;
				//当前比牌玩家
				s13s::PlayerItem const& player = compareCards[i].player();
				//遍历各墩(头墩/中墩/尾墩)
				for (int d = S13S::DunFirst; d <= S13S::DunLast; ++d) {
					//按墩比输赢分
					int sumscore = 0;
					//按墩计算打枪输赢分
					int sumshootscore = 0;
					{
						//自己特殊牌型，不按墩比牌，输赢水数在specitem中
						if (player.group().specialty() >= S13S::TyThreesc) {
						}
						else {
							//遍历比牌对象
							for (int j = 0; j < compareCards[i].peers_size(); ++j) {
								s13s::ComparePlayer const& peer = compareCards[i].peers(j);
								s13s::CompareResult const& result = compareCards[i].results(j);
								//对方特殊牌型，不按墩比牌，输赢水数在specitem中
								if (peer.group().specialty() >= S13S::TyThreesc) {
									continue;
								}
								//累加指定墩输赢得水积分，不含打枪/全垒打
								sumscore += result.items(d).score();
								//1打枪(对当前比牌对象打枪)
								if (result.shoot() == 1) {
									//1全垒打
									if (compareCards[i].allshoot() == 1) {
										//-1被全垒打/0无全垒打/1全垒打
										sumshootscore += 3/*4*/ * result.items(d).score();
									}
									else {
										//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
										if (compareCards[i].allshoot() == -1) {
										}
										else {
										}
										//-1被打枪/0不打枪/1打枪
										sumshootscore += 1/*2*/ * result.items(d).score();
									}
								}
								//-1被打枪(被当前比牌对象打枪)
								else if (result.shoot() == -1) {
									//-1被全垒打
									if (compareCards[i].allshoot() == -1) {
										//被当前比牌对象全垒打
										if (peer.chairid() == compareCards[i].fromchairid()) {
											//-1被全垒打/0无全垒打/1全垒打
											sumshootscore += 3/*4*/ * result.items(d).score();
										}
										//被另外比牌对象全垒打
										else {
											//-1被打枪/0不打枪/1打枪
											sumshootscore += 1/*2*/ * result.items(d).score();
										}
									}
									else {
										//-1被打枪/0不打枪/1打枪
										sumshootscore += 1/*2*/ * result.items(d).score();
									}
								}
								//0不打枪(与当前比牌对象互不打枪)
								else {
									//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
									if (compareCards[i].allshoot() == -1) {
									}
									else {
										//一定不是全垒打，-1被打枪/0不打枪/1打枪
										assert(compareCards[i].allshoot() == 0);
									}
									assert(result.shoot() == 0);
									//sumshootscore += 0/*1*/ * result.items(d).score();
								}
							}
						}
					}
					//座椅玩家输赢得水积分(头/中/尾/特殊)
					compareCards[i].add_itemscores(sumscore);
					//按墩计算打枪/全垒打输赢分
					compareCards[i].add_itemscorepure(sumshootscore);
					//三墩打枪/全垒打输赢算水
					allshootscore += sumshootscore;
				}
				{
					//特殊牌型输赢算水(无打枪/全垒打)
					int sumspecscore = 0;
					{
						//遍历比牌对象
						for (int j = 0; j < compareCards[i].peers_size(); ++j) {
							s13s::ComparePlayer const& peer = compareCards[i].peers(j);
							s13s::CompareResult const& result = compareCards[i].results(j);
							//自己普通牌型，对方特殊牌型
							//自己特殊牌型，对方普通牌型
							//自己特殊牌型，对方特殊牌型
							if (result.has_specitem()) {
								//累加特殊比牌算水
								sumspecscore += result.specitem().score();
							}
						}
					}
					//座椅玩家输赢得水积分(头/中/尾/特殊)
					compareCards[i].add_itemscores(sumspecscore);
					//三墩打枪/全垒打输赢算水 + 特殊牌型输赢算水(无打枪/全垒打)
					compareCards[i].add_itemscorepure(allshootscore + sumspecscore);
					//玩家输赢得水总分，含打枪/全垒打，含特殊牌型输赢分
					int32_t deltascore = compareCards[i].deltascore();
					compareCards[i].set_deltascore(deltascore + sumspecscore);
					//compareCards[i]对应groups[i]
					const_cast<groupdun_t*&>(groups[i])->deltascore = compareCards[i].deltascore();
					//printf("玩家[%d]输赢得水总分[%d]，含打枪/全垒打，含特殊牌型输赢分\n", i, compareCards[i].deltascore());
				}
			}
		}
		//比牌对方输赢得水总分
		for (int i = 0; i < groups.size(); ++i) {
			if (true) {
				for (int j = 0; j < compareCards[i].peers_size(); ++j) {
					s13s::ComparePlayer const& peer = compareCards[i].peers(j);
					int32_t deltascore = compareCards[peer.chairid()].deltascore();
					const_cast<s13s::ComparePlayer&>(peer).set_deltascore(deltascore);
				}
			}
		}
		//两组牌按输赢得水比大小相同的情况下
		if (psrc->deltascore == pdst->deltascore) {
			//尾墩大的摆前面
			if (psrc->duns[DunLast].ty_ > pdst->duns[DunLast].ty_) {
				return 1;
			}
			//尾墩相同，中墩大的摆前面
			else if (psrc->duns[DunLast].ty_ == pdst->duns[DunLast].ty_ &&
				psrc->duns[DunSecond].ty_ > pdst->duns[DunSecond].ty_) {
				return 1;
			}
			//尾墩相同[，中墩相同]，头敦大的摆前面
			else if (psrc->duns[DunLast].ty_ == pdst->duns[DunLast].ty_ &&
				/*psrc->duns[DunSecond].ty_ == pdst->duns[DunSecond].ty_ &&*/
				psrc->duns[DunFirst].ty_ > pdst->duns[DunFirst].ty_) {
				return 1;
			}
		}
		return psrc->deltascore - pdst->deltascore;
	}

	//初始化牌墩
	void CGameLogic::EnumTree::Init(DunTy dt) {
		//按同花顺/铁支/葫芦/同花/顺子/三条/两对/对子顺序依次进行
		{
			c = 0;
			//同花顺
			for (std::vector<CardData>::const_iterator it = v123sc.begin();
				it != v123sc.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty123sc, &*it), NULL);
			}
			//铁支
			for (std::vector<CardData>::const_iterator it = v40.begin();
				it != v40.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty40, &*it), NULL);
			}
			//葫芦
			for (std::vector<CardData>::const_iterator it = v32.begin();
				it != v32.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty32, &*it), NULL);
			}
			//同花
			for (std::vector<CardData>::const_iterator it = vsc.begin();
				it != vsc.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Tysc, &*it), NULL);
			}
			//顺子
			for (std::vector<CardData>::const_iterator it = v123.begin();
				it != v123.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty123, &*it), NULL);
			}
			//三条
			for (std::vector<CardData>::const_iterator it = v30.begin();
				it != v30.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty30, &*it), NULL);
			}
			//两对
			for (std::vector<CardData>::const_iterator it = v22.begin();
				it != v22.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty22, &*it), NULL);
			}
			//对子
			for (std::vector<CardData>::const_iterator it = v20.begin();
				it != v20.end(); ++it) {
				assert(c < MaxEnumSZ);
				tree[c++] = std::make_pair<EnumItem, EnumTree*>(std::make_pair(Ty20, &*it), NULL);
			}
			//printf("-- *** tree.size = %d\n", c);
		}
		{
			//标识头/中/尾墩
			dt_ = dt;
			//初始化游标
			cursor_ = -1;
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
		c = 0;
		cursor_ = -1;
	}

	//重置游标
	void CGameLogic::EnumTree::ResetCursor() {
		cursor_ = -1;
	}

	//返回下一个游标
	bool CGameLogic::EnumTree::GetNextCursor(int& cursor) {
		if (++cursor_ < c) {
			cursor = cursor_;
			return true;
		}
		return false;
	}

	//释放new内存
	void CGameLogic::EnumTree::Release() {
		
		//子节点/根节点游标
		int cursorChild = 0, cursorRoot = 0;
		EnumTree* const& rootEnumList = this;
		
		rootEnumList->ResetCursor();
		while (true) {
			//返回根节点下一个游标 //////
			if (!rootEnumList->GetNextCursor(cursorRoot)) {
				break;
			}
			//子节点
			EnumTree*& childEnumList = rootEnumList->GetCursorChild(cursorRoot);
			//如果非空
			if (childEnumList) {
				childEnumList->ResetCursor();
				while (true) {
					//返回子节点下一个游标 //////
					if (!childEnumList->GetNextCursor(cursorChild)) {
						break;
					}
					//叶子节点
					EnumTree*& leafEnumList = childEnumList->GetCursorChild(cursorChild);
					//如果非空
					if (leafEnumList) {
						delete leafEnumList;//删除叶子节点
						leafEnumList = NULL;
						//printf("--- *** 删除叶子节点\n");
#ifdef _MEMORY_LEAK_DETECK_
						--MemoryCount_;
#endif
					}
				}
				//删除子节点
				delete childEnumList;
				childEnumList = NULL;
				//printf("--- *** 删除子节点\n");
#ifdef _MEMORY_LEAK_DETECK_
				--MemoryCount_;
#endif
			}
		}
	}
	
	static std::string StringDunTy(DunTy ty) {
		switch (ty) {
		case DunFirst: return "头墩";
		case DunSecond: return "中墩";
		case DunLast: return "尾墩";
		}
	}

	//打印枚举牌型
	void CGameLogic::EnumTree::PrintEnumCards(bool reverse/* = false*/, HandTy ty/* = TyAllBase*/) {
		switch (ty)
		{
		case Tysp:		PrintEnumCards("乌龙", ty,*vsp, reverse);		break;//乌龙
		case Ty20:		PrintEnumCards("对子", ty, v20, reverse);		break;//对子
		case Ty22:		PrintEnumCards("两对", ty, v22, reverse);		break;//两对
		case Ty30:		PrintEnumCards("三条", ty, v30, reverse);		break;//三条
		case Ty123:		PrintEnumCards("顺子", ty, v123, reverse);		break;//顺子
		case Tysc:		PrintEnumCards("同花", ty, vsc, reverse);		break;//同花
		case Ty32:		PrintEnumCards("葫芦", ty, v32, reverse);		break;//葫芦
		case Ty40:		PrintEnumCards("铁支", ty, v40, reverse);		break;//铁支
		case Ty123sc:	PrintEnumCards("同花顺", ty, v123sc, reverse);	break;//同花顺
		case TyAllBase:
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
			printf("--- *** ------------------------- 枚举%s %s[%s] 数量[%d]\n",
				StringDunTy(dt_).c_str(), name.c_str(), StringHandTy(ty).c_str(), src.size());
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

	//打印游标处枚举牌型
	void CGameLogic::EnumTree::PrintCursorEnumCards() {
		EnumTree::EnumItem const* item = GetCursorItem(cursor_);
		if (item) {
			switch (item->first)
			{
			case Tysp:break;
			case Ty20:		PrintCursorEnumCards("对子", item->first, *item->second);	break;//对子
			case Ty22:		PrintCursorEnumCards("两对", item->first, *item->second);	break;//两对
			case Ty30:		PrintCursorEnumCards("三条", item->first, *item->second);	break;//三条
			case Ty123:		PrintCursorEnumCards("顺子", item->first, *item->second);	break;//顺子
			case Tysc:		PrintCursorEnumCards("同花", item->first, *item->second);	break;//同花
			case Ty32:		PrintCursorEnumCards("葫芦", item->first, *item->second);	break;//葫芦
			case Ty40:		PrintCursorEnumCards("铁支", item->first, *item->second);	break;//铁支
			case Ty123sc:	PrintCursorEnumCards("同花顺", item->first, *item->second);	break;//同花顺
			}
		}
	}
	
	//打印游标处枚举牌型
	void CGameLogic::EnumTree::PrintCursorEnumCards(std::string const& name, HandTy ty, std::vector<uint8_t> const& src) {
		printf("--- *** 第[%d]墩 - %s[%s]：", dt_ + 1, name.c_str(), StringHandTy(ty).c_str());
		PrintCardList(&src[0], src.size());
	}
	
	//返回游标处枚举牌型
	CGameLogic::EnumTree::EnumItem const* CGameLogic::EnumTree::GetCursorItem(int cursor) {
		return cursor < c ? &tree[cursor].first : NULL;
	}
	
	//返回游标处枚举牌型对应余牌枚举子项列表指针
	CGameLogic::EnumTree*& CGameLogic::EnumTree::GetCursorChild(int cursor)/* __attribute__((noreturn))*/ {
		assert(cursor < c);
		return tree[cursor].second;
	}

	//返回下一个枚举牌型(从大到小返回)
	bool CGameLogic::EnumTree::GetNextEnumItem(uint8_t const* src, int len,
		CGameLogic::EnumTree::CardData const*& dst, HandTy& ty,
		int& cursor, uint8_t *cpy, int& cpylen) {
		ty = TyNil;
		cpylen = 0;
		if (++cursor_ < c) {
			ty = tree[cursor_].first.first;
			dst = tree[cursor_].first.second;
			cursor = cursor_;
			for (int i = 0; i < len; ++i) {
				CardData::const_iterator it;
				for (it = dst->begin(); it != dst->end(); ++it) {
					if (src[i] == *it) {
						//src[i]在dst中存在了
						break;
					}
				}
				//不存在的话添加到余牌中
				if (it == dst->end()) {
					cpy[cpylen++] = src[i];
				}
			}
			return true;
		}
		return false;
	}
	
	//判断手动摆牌是否倒水
	//dt DunTy 指定为哪墩
	//src uint8_t const* 选择的一组牌(5张或3张)
	//len int 3/5张，头墩3张/中墩5张/尾墩5张
	//ty HandTy 指定墩牌型
	bool CGameLogic::handinfo_t::IsInverted(DunTy dt, uint8_t const* src, int len, HandTy ty) {
		S13S::CGameLogic::groupdun_t const *dst = &manual_group;
		return IsInverted(dt, src, len, ty, dst);
	}
	
	//判断手动摆牌是否倒水
	//dt DunTy 指定为哪墩
	//src uint8_t const* 选择的一组牌(5张或3张)
	//len int 3/5张，头墩3张/中墩5张/尾墩5张
	//ty HandTy 指定墩牌型
	//group groupdun_t const* 一组牌墩
	bool CGameLogic::handinfo_t::IsInverted(DunTy dt, uint8_t const* src, int len, HandTy ty, groupdun_t const* group) {
		S13S::CGameLogic::groupdun_t const *dst = group;
		switch (dt)
		{
		case DunFirst: {
			if (ty == S13S::Ty123sc || ty == S13S::Tysc || ty == S13S::Ty123) {
				ty = S13S::Tysp;
			}
			//比较对子和三条牌型大小
			assert(
				ty == S13S::Tysp ||
				ty == S13S::Ty20 ||
				ty == S13S::Ty30);
			//不得大于中墩
			if (dst->needC(DunSecond) == 0) {
				//牌型不同比牌型
				if (ty != dst->duns[DunSecond].ty_) {
					if (ty > dst->duns[DunSecond].ty_) {
						return true;
					}
				}
				//牌型相同比大小
				else if (S13S::CGameLogic::CompareCards(
					src, len,
					dst->duns[DunSecond].cards, dst->duns[DunSecond].GetC(), false, ty) > 0) {
					return true;
				}
			}
			//不得大于尾墩
			if (dst->needC(DunLast) == 0) {
				//牌型不同比牌型
				if (ty != dst->duns[DunLast].ty_) {
					if (ty > dst->duns[DunLast].ty_) {
						return true;
					}
				}
				//牌型相同比大小
				else if (S13S::CGameLogic::CompareCards(
					src, len,
					dst->duns[DunLast].cards, dst->duns[DunLast].GetC(), false, ty) > 0) {
					return true;
				}
			}
			break;
		}
		case DunSecond:{
			//不得小于头墩
			if (dst->needC(DunFirst) == 0) {
				//牌型不同比牌型
				if (ty != dst->duns[DunFirst].ty_) {
					if (ty < dst->duns[DunFirst].ty_) {
						return true;
					}
				}
				//牌型相同比大小
				else if (S13S::CGameLogic::CompareCards(
					src, len,
					dst->duns[DunFirst].cards, dst->duns[DunFirst].GetC(), false, ty) < 0) {
					return true;
				}
			}
			//不得大于尾墩
			if (dst->needC(DunLast) == 0) {
				//牌型不同比牌型
				if (ty != dst->duns[DunLast].ty_) {
					if (ty > dst->duns[DunLast].ty_) {
						return true;
					}
				}
				//牌型相同比大小
				else if (S13S::CGameLogic::CompareCards(
					src, len,
					dst->duns[DunLast].cards, dst->duns[DunLast].GetC(), false, ty) > 0) {
					return true;
				}
			}
			break;
		}
		case DunLast: {
			//不得小于头墩
			if (dst->needC(DunFirst) == 0) {
				//牌型不同比牌型
				if (ty != dst->duns[DunFirst].ty_) {
					if (ty < dst->duns[DunFirst].ty_) {
						return true;
					}
				}
				//牌型相同比大小
				else if (S13S::CGameLogic::CompareCards(
					src, len,
					dst->duns[DunFirst].cards, dst->duns[DunFirst].GetC(), false, ty) < 0) {
					return true;
				}
			}
			//不得小于中墩
			if (dst->needC(DunSecond) == 0) {
				//牌型不同比牌型
				if (ty != dst->duns[DunSecond].ty_) {
					if (ty < dst->duns[DunSecond].ty_) {
						return true;
					}
				}
				//牌型相同比大小
				else if (S13S::CGameLogic::CompareCards(
					src, len,
					dst->duns[DunSecond].cards, dst->duns[DunSecond].GetC(), false, ty) < 0) {
					return true;
				}
			}
			break;
		}
		default:
			break;
		}
		return false;
	}
	
	//手动选牌组墩，给指定墩(头/中/尾墩)选择一组牌(头墩3/中墩5/尾墩5)
	//dt DunTy 指定为哪墩
	//src uint8_t const* 选择的一组牌(5张或3张)
	//len int 3/5张，头墩3张/中墩5张/尾墩5张
	//ty HandTy 指定墩牌型
	bool CGameLogic::handinfo_t::SelectAs(DunTy dt, uint8_t const* src, int len, HandTy ty) {
		assert(dt > DunNil && dt < DunMax);
		if (manual_group.duns[dt].GetC() > 0) {
			return false;
		}
		manual_group.assign(dt, ty, src, len);
		if (manual_group_index == -1) {
			if (
				manual_group.duns[DunFirst].GetC() == 3 &&
				manual_group.duns[DunSecond].GetC() == 5 &&
				manual_group.duns[DunLast].GetC() == 5) {
				//构成完整三墩牌当作一组牌放入groups中
				groups.push_back(&manual_group);
				assert(groups.size() > 0);
				manual_group_index = groups.size() - 1;
			}
		}
		return true;
 	}
	
	//重置手动摆牌
	void CGameLogic::handinfo_t::ResetManual() {
		if (manual_group_index != -1) {
			groups.pop_back();
			manual_group_index = -1;
		}
		manual_group.Reset();
	}
	
	//手动摆牌清空重置指定墩的牌
	bool CGameLogic::handinfo_t::ResetManual(DunTy dt) {
		assert(dt > DunNil && dt < DunMax);
		if (manual_group_index != -1) {
			groups.pop_back();
			manual_group_index = -1;
		}
		//清空重置指定墩的牌
		if (manual_group.duns[dt].needC() == 0) {
			//manual_group.duns[dt].c = 0;
			manual_group.duns[dt].Reset();
			return true;
		}
		return false;
	}
	
	//手牌确定三墩牌型
	//groupindex int 若 >=0 从enum_groups中选择一组，对应groups中索引
	//groupindex int 若 <= -1 指向manual_group对应groups中索引
	bool CGameLogic::handinfo_t::Select(int groupindex) {
		if (groupindex >= 0) {
			if (spec_groups.size() > 0) {
				assert(spec_groups.size() == 1);
				//枚举几组最优解必须存在
				assert(enum_groups.size() > 0);
				//不能超过枚举几组解范围
				assert(groupindex < spec_groups.size() + enum_groups.size());
				//选择特殊牌型/枚举一组
				select_group_index = groupindex;
			}
			else {
				//枚举几组最优解必须存在
				assert(enum_groups.size() > 0);
				//不能超过枚举几组解范围
				assert(groupindex < enum_groups.size());
				//从枚举最优解中选择一组
				//直接选择枚举中的一组进行比牌
				select_group_index = groupindex;
			}
		}
		else {
			//没有进行过手动任意摆牌
			//if (!HasManualGroup()) {
			//	assert(GetManualC() < MAX_COUNT);
			//	return false;
			//}
			//否则指向手动摆牌组墩
			assert(manual_group_index != -1);
			assert(GetManualC() == MAX_COUNT);
			//手动摆牌构成的一组进行比牌
			select_group_index = manual_group_index;
		}
		return select_group_index != -1;
	}
	
	//返回手牌确定的三墩牌型
	CGameLogic::groupdun_t const* CGameLogic::handinfo_t::GetSelected() {
		assert(select_group_index != -1);
		assert(select_group_index >= 0);
		assert(groups.size() > 0);
		assert(select_group_index < groups.size());
		return groups[select_group_index];
	}
	
	//返回组墩后剩余牌/散牌
	//src uint8_t const* 一副手牌13张
	//cpy uint8_t *cpy 组墩后剩余牌 cpylen int& 余牌数量
	void CGameLogic::handinfo_t::GetLeftCards(uint8_t const* src, int len, uint8_t *cpy, int& cpylen) {
		CGameLogic::GetLeftCards(src, len, manual_group.duns, cpy, cpylen);
	}
	
	//打印指定墩牌型
	void CGameLogic::groupdun_t::PrintCardList(DunTy dt) {
		assert(dt < DunMax);
		switch (duns[dt].ty_)
		{
		//普通牌型
		case Tysp:		PrintCardList("乌龙", dt, duns[dt].ty_);	break;//乌龙
		case Ty20:		PrintCardList("对子", dt, duns[dt].ty_);	break;//对子
		case Ty22:		PrintCardList("两对", dt, duns[dt].ty_);	break;//两对
		case Ty30:		PrintCardList("三条", dt, duns[dt].ty_);	break;//三条
		case Ty123:		PrintCardList("顺子", dt, duns[dt].ty_);	break;//顺子
		case Tysc:		PrintCardList("同花", dt, duns[dt].ty_);	break;//同花
		case Ty32:		PrintCardList("葫芦", dt, duns[dt].ty_);	break;//葫芦
		case Ty40:		PrintCardList("铁支", dt, duns[dt].ty_);	break;//铁支
		case Ty123sc:	PrintCardList("同花顺", dt, duns[dt].ty_); break;//同花顺
		//特殊牌型
		case TyThreesc:		PrintCardList("三同花", dt, duns[dt].ty_); break;//三同花
		case TyThree123:	PrintCardList("三顺子", dt, duns[dt].ty_); break;//三顺子
		case TySix20:		PrintCardList("六对半", dt, duns[dt].ty_); break;//六对半
		case TyFive2030:	PrintCardList("五对三条", dt, duns[dt].ty_); break;//五对三条
		case TyFour30:		PrintCardList("四套三条", dt, duns[dt].ty_); break;//四套三条
		case TyTwo3220:		PrintCardList("双怪冲三", dt, duns[dt].ty_); break;//双怪冲三
		case TyAllOneColor:	PrintCardList("凑一色", dt, duns[dt].ty_); break;//凑一色
		case TyAllSmall:	PrintCardList("全小", dt, duns[dt].ty_); break;//全小
		case TyAllBig:		PrintCardList("全大", dt, duns[dt].ty_); break;//全大
		case TyThree40:		PrintCardList("三分天下", dt, duns[dt].ty_); break;//三分天下
		case TyThree123sc:	PrintCardList("三同花顺", dt, duns[dt].ty_); break;//三同花顺
		case Ty12Royal:		PrintCardList("十二皇族", dt, duns[dt].ty_); break;//十二皇族
		case TyOneDragon:	PrintCardList("一条龙", dt, duns[dt].ty_); break;//一条龙
		case TyZZQDragon:	PrintCardList("至尊青龙", dt, duns[dt].ty_); break;//至尊青龙
		}
	}

	//打印指定墩牌型
	void CGameLogic::groupdun_t::PrintCardList(std::string const& name, DunTy dt, HandTy ty) {
		printf("--- *** 第[%d]墩 - %s[%s]：", dt + 1, name.c_str(), StringHandTy(ty).c_str());
		CGameLogic::PrintCardList(duns[dt].cards, duns[dt].c, true);
	}

	//初始化
	void CGameLogic::handinfo_t::Init() {
		if (rootEnumList == NULL) {
			//必须用成员结构体指针形式来new结构体成员对象，否则类成员变量数据会错乱，
			//只要类成员结构体嵌入vector/string这些STL对象会出问题，编译器bug???
			rootEnumList = new EnumTree();
		}
	}
	
	void CGameLogic::handinfo_t::Reset() {
		//chairID = -1;
		//specialTy_ = TyNil;
		manual_group_index = -1;
		select_group_index = -1;
		manual_group.Reset();
		enum_groups.clear();
		spec_groups.clear();
		leafList.clear();
		groups.clear();
		if (rootEnumList) {
			rootEnumList->Release();
			rootEnumList->Reset();
#ifdef _MEMORY_LEAK_DETECK_
			printf("--- *** hand[%d]free MemoryCount = %d\n\n", chairID, rootEnumList->MemoryCount_);
#endif
		}
		memset(&classify, 0, sizeof(classify_t));
	}
	
	//打印全部枚举墩牌型
	void CGameLogic::handinfo_t::PrintEnumCards(bool reverse) {
		CGameLogic::handinfo_t::PrintCardList(enum_groups, reverse);
	}
	
	//打印特殊牌型
	void CGameLogic::handinfo_t::PrintSpecCards() {
		CGameLogic::handinfo_t::PrintCardList(spec_groups, false);
	}
	
	//打印牌型列表
	void CGameLogic::handinfo_t::PrintCardList(std::vector<groupdun_t>& rgroups, bool reverse) {
		if(reverse) {
			int i = rgroups.size();
			//倒序从小到大输出
			for (std::vector<groupdun_t>::reverse_iterator it = rgroups.rbegin();
				it != rgroups.rend(); ++it) {
				printf("\n--- *** --------------------------------------------------\n");
				if (it->specialTy != TyNil) {
					printf("--- *** 第[%d]组 %s\n", i--, StringSpecialTy(it->specialTy).c_str());
				}
				else {
					printf("--- *** 第[%d]组\n", i--);
				}
				it->PrintCardList(DunFirst);
				it->PrintCardList(DunSecond);
				it->PrintCardList(DunLast);
				printf("--- *** --------------------------------------------------\n\n");
			}
		}
		else {
			int i = 0;
			for (std::vector<groupdun_t>::iterator it = rgroups.begin();
				it != rgroups.end(); ++it) {
				printf("\n--- *** --------------------------------------------------\n");
				if (it->specialTy != TyNil) {
					printf("--- *** 第[%d]组 %s\n", i++ + 1, StringSpecialTy(it->specialTy).c_str());
				}
				else {
					printf("--- *** 第[%d]组\n", i++ + 1);
				}
				it->PrintCardList(DunFirst);
				it->PrintCardList(DunSecond);
				it->PrintCardList(DunLast);
				printf("--- *** --------------------------------------------------\n\n");
			}
		}
	}
	
	static std::string CNStringTy(char const* name, char const* ty) {
		char ch[50] = { 0 };
		sprintf(ch, "%s[%s]", name, ty);
		return ch;
	}
	
	//返回特殊牌型字符串
	std::string CGameLogic::handinfo_t::StringSpecialTy() {
		HandTy specialTy = TyNil;
		if (spec_groups.size() > 0) {
			assert(spec_groups.size() == 1);
			specialTy = spec_groups.front().specialTy;
		}
		return StringSpecialTy(specialTy);
	}

	//返回特殊牌型字符串
	std::string CGameLogic::handinfo_t::StringSpecialTy(HandTy specialTy) {
		switch (specialTy)
		{
		case TyThreesc:		return CNStringTy("三同花", "TyThreesc");
		case TyThree123:	return CNStringTy("三顺子", "TyThree123");
		case TySix20:		return CNStringTy("六对半", "TySix20");
		case TyFive2030:	return CNStringTy("五对三条", "TyFive2030");
		case TyFour30:		return CNStringTy("四套三条", "TyFour30");
		case TyTwo3220:		return CNStringTy("双怪冲三", "TyTwo3220");
		case TyAllOneColor:	return CNStringTy("凑一色", "TyAllOneColor");
		case TyAllSmall:	return CNStringTy("全小", "TyAllSmall");
		case TyAllBig:		return CNStringTy("全大", "TyAllBig");
		case TyThree40:		return CNStringTy("三分天下", "TyThree40");
		case TyThree123sc:	return CNStringTy("三同花顺", "TyThree123sc");
		case Ty12Royal:		return CNStringTy("十二皇族", "Ty12Royal");
		case TyOneDragon:	return CNStringTy("一条龙", "TyOneDragon");
		case TyZZQDragon:	return CNStringTy("至尊青龙", "TyZZQDragon");
		}
		return "";
	}
	
	//返回组墩后剩余牌/散牌
	//src uint8_t const* 一副手牌13张
	//duns dundata_t const* 一组墩(头/中/尾墩)
	//cpy uint8_t *cpy 组墩后剩余牌 cpylen int& 余牌数量
	void CGameLogic::GetLeftCards(uint8_t const* src, int len,
		dundata_t const* duns, uint8_t *cpy, int& cpylen) {
		cpylen = 0;
		bool bok = false;
		//遍历一副手牌查找当前牌
		for (int i = 0; i < len; ++i) {
		next:
			if (bok) {
				bok = false;
				continue;
			}
			//遍历每一墩牌
			for (int j = DunLast; j >= DunFirst; --j) {
				//遍历每一墩里面的每张牌
				for (int c = 0; c < duns[j].c; ++c) {
					if (src[i] == duns[j].cards[c]) {
						//src[i]在duns[j]中存在了
						bok = true;
						goto next;
					}
				}
			}
			//不存在的话添加到余牌中
			cpy[cpylen++] = src[i];
		}
	}

	//按照尾墩5张/中墩5张/头墩3张依次抽取枚举普通牌型
	//src uint8_t const* 手牌余牌(13/8/3)，初始13张，按5/5/3依次抽，余牌依次为13/8/3
	//n int 抽取n张(5/5/3) 第一次抽5张余8张，第二次抽5张余3张，第三次取余下3张抽完
	//classify classify_t& 存放分类信息(所有重复四张/三张/二张/散牌/余牌)
	//enumList EnumTree& 存放枚举墩牌型列表数据 dt DunTy 指定为第几墩
	void CGameLogic::EnumCards(uint8_t const* src, int len,
		int n, classify_t& classify, EnumTree& enumList, DunTy dt) {
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
		//初始化牌墩
		enumList.Init(dt);
	}

	//单墩牌型判断(3/5张牌)
	//dt DunTy 指定为第几墩
	//src uint8_t const* 一墩5张或3张的牌
	HandTy CGameLogic::GetDunCardHandTy(DunTy dt, uint8_t const* src, int len)
	{
		HandTy ty = Tysp;//散牌
		assert(dt > DunNil && dt < DunMax);
		assert(len == (dt == DunFirst ? 3 : 5));
		uint8_t psrc[5] = { 0 };
		memcpy(psrc, src, len);
		SortCards(psrc, len, true, true, true);
		classify_t classify = { 0 };
		EnumTree enumList;
		EnumCards(psrc, len, len, classify, enumList, dt);
		if (enumList.v123sc.size() > 0) {
			ty = Ty123sc;//同花顺
		}
		else if (enumList.v40.size() > 0) {
			ty = Ty40;//铁支
		}
		else if (enumList.v32.size() > 0) {
			ty = Ty32;//葫芦
		}
		else if (enumList.vsc.size() > 0) {
			ty = Tysc;//同花
		}
		else if (enumList.v123.size() > 0) {
			ty = Ty123;//顺子
		}
		else if (enumList.v30.size() > 0) {
			ty = Ty30;//三条
		}
		else if (enumList.v22.size() > 0) {
			ty = Ty22;//两对
		}
		else if (enumList.v20.size() > 0) {
			ty = Ty20;//对子
		}
		return ty;
	}
	
	//牌型相同的src与dst比大小，牌数相同
	//src uint8_t const* 单墩牌(3/5张)
	//dst uint8_t const* 单墩牌(3/5张)
	//clr bool 是否比花色
	//ty HandTy 比较的两单墩牌的普通牌型
	int CGameLogic::CompareCards(uint8_t const* src, uint8_t const* dst, int n, bool clr, HandTy ty, bool sp, bool sd, bool dz) {
		return CompareCards(src, n, dst, n, clr, ty, sp, sd, dz);
	}
	
	//牌型相同按牌点从大到小顺序逐次比点
	//src uint8_t const* srcLen张牌
	//dst uint8_t const* dstLen张牌
	//clr bool 是否比花色
	int CGameLogic::CompareCardPointBy(
		uint8_t const* src, int srcLen,
		uint8_t const* dst, int dstLen, bool clr) {
		uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
		assert(srcLen > 0);
		assert(dstLen > 0);
		memcpy(psrc, src, srcLen);
		memcpy(pdst, dst, dstLen);
		SortCards(psrc, srcLen, false, true, true);
		SortCards(pdst, dstLen, false, true, true);
		//牌型相同按顺序比点，先将src/dst按牌点升序排
		int i = srcLen - 1, j = dstLen - 1;
		while(i >= 0 && j >= 0) {
			uint8_t p0 = GetCardPoint(psrc[i--]);
			uint8_t p1 = GetCardPoint(pdst[j--]);
			if (p0 != p1) {
				return p0 - p1;
			}
		}
		if (clr) {
			//点数相同按顺序比花色
			int i = srcLen - 1, j = dstLen - 1;
			while (i >= 0 && j >= 0) {
				uint8_t c0 = GetCardColor(src[i--]);
				uint8_t c1 = GetCardColor(dst[j--]);
				if (c0 != c1) {
					return c0 - c1;
				}
			}
		}
		return 0;
	}
	
	//任意牌型的src与dst两墩之间比大小
	//src uint8_t const* srcLen张牌
	//srcTy HandTy src牌型
	//dst uint8_t const* dstLen张牌
	//dstTy HandTy dst牌型
	//clr bool 是否比花色
	//sp bool 是否比较散牌/单张
	//sd bool 是否比较次大的对子(两对之间比较)
	//dz bool 是否比较葫芦的对子(葫芦之间比较)
	int CGameLogic::CompareCards(
		uint8_t const* src, int srcLen, HandTy srcTy,
		uint8_t const* dst, int dstLen, HandTy dstTy, bool clr, bool sp, bool sd, bool dz) {
		if (srcTy != dstTy) {
			//牌型不同比牌型
			return srcTy - dstTy;
		}
		//牌型相同比大小
		return CompareCards(src, srcLen, dst, dstLen, clr, srcTy, sp, sd, dz);
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
		uint8_t const* dst, int dstLen, bool clr, HandTy ty, bool sp, bool sd, bool dz) {
		switch (ty) {
		//同花顺之间比较
		case Ty123sc: return sp ? CompareCardPointBy(src, srcLen, dst, dstLen, clr) : 0;
		//同花之间比较
		case Tysc: return sp ? CompareCardPointBy(src, srcLen, dst, dstLen, clr) : 0;
		//顺子之间比较
		case Ty123: return sp ? CompareCardPointBy(src, srcLen, dst, dstLen, clr) : 0;
		//散牌之间比较
		case Tysp: return sp ? CompareCardPointBy(src, srcLen, dst, dstLen, clr) : 0;
		//铁支之间比较
		case Ty40: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			SortCards(psrc, srcLen, true, true, true);
			SortCards(pdst, dstLen, true, true, true);

			//一副牌的话四张肯定不一样，多副牌的话比完四张还要比单张
			int c4_0 = 0, cpylen_0 = 0;
			uint8_t dst4_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size4 = 3;
				c4_0 = EnumRepeatCards(psrc, srcLen, 4, dst4_0, size4, cpy_0, cpylen_0);
				if (c4_0 != 1) {
					S13S::CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c4_0 == 1);
			}
			int c4_1 = 0, cpylen_1 = 0;
			uint8_t dst4_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size4 = 3;
				c4_1 = EnumRepeatCards(pdst, dstLen, 4, dst4_1, size4, cpy_1, cpylen_1);
				if (c4_1 != 1) {
					S13S::CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c4_1 == 1);
			}
			uint8_t p0 = GetCardPoint(dst4_0[0][0]);
			uint8_t p1 = GetCardPoint(dst4_1[0][0]);
			//先比较四张大小，再比较剩余单张大小
			if (p0 != p1) {
				return p0 - p1;
			}
			if (sp && cpylen_0 && cpylen_1) {
				//四张大小相同，比较单张大小
				return GetCardPoint(cpy_0[0]) - GetCardPoint(cpy_1[0]);
			}
			break;
		}
		//葫芦之间比较
		case Ty32: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			SortCards(psrc, srcLen, true, true, true);
			SortCards(pdst, dstLen, true, true, true);

			//一副牌的话三张肯定不一样，多副牌的话比完三张还要比对子
			int c3_0 = 0, cpylen_0 = 0;
			uint8_t dst3_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_0 = EnumRepeatCards(psrc, srcLen, 3, dst3_0, size3, cpy_0, cpylen_0);
				if (c3_0 != 1) {
					S13S::CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c3_0 == 1);
			}
			int c3_1 = 0, cpylen_1 = 0;
			uint8_t dst3_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_1 = EnumRepeatCards(pdst, dstLen, 3, dst3_1, size3, cpy_1, cpylen_1);
				if (c3_1 != 1) {
					S13S::CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c3_1 == 1);
			}
			//先比较三张的大小，再比较对子大小
			uint8_t p0 = GetCardPoint(dst3_0[0][0]);
			uint8_t p1 = GetCardPoint(dst3_1[0][0]);
			if (p0 != p1) {
				return p0 - p1;
			}
			if (dz) {
				//三张大小相同，比较对子大小
				return GetCardPoint(cpy_0[0]) - GetCardPoint(cpy_1[0]);
			}
			break;
		}
		//三条之间比较
		case Ty30: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			SortCards(psrc, srcLen, true, true, true);
			SortCards(pdst, dstLen, true, true, true);

			//一副牌的话三张肯定不一样，多副牌的话比完三张还要比散牌
			int c3_0 = 0, cpylen_0 = 0;
			uint8_t dst3_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_0 = EnumRepeatCards(psrc, srcLen, 3, dst3_0, size3, cpy_0, cpylen_0);
				if (c3_0 != 1) {
					S13S::CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c3_0 == 1);
			}
			int c3_1 = 0, cpylen_1 = 0;
			uint8_t dst3_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size3 = 4;
				c3_1 = EnumRepeatCards(pdst, dstLen, 3, dst3_1, size3, cpy_1, cpylen_1);
				if (c3_1 != 1) {
					S13S::CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c3_1 == 1);
			}
			uint8_t p0 = GetCardPoint(dst3_0[0][0]);
			uint8_t p1 = GetCardPoint(dst3_1[0][0]);
			//先比较三张的大小，再比较其余散牌大小
			if (p0 != p1) {
				return p0 - p1;
			}
			if (sp && cpylen_0 && cpylen_1) {
				//三张大小相同，比较散牌大小
				return CompareCardPointBy(cpy_0, cpylen_0, cpy_1, cpylen_1, clr);
			}
			break;
		}
		//两对之间比较 ♦A ♣A ♣8 ♣K ♠8
		case Ty22: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			SortCards(psrc, srcLen, true, true, true);
			SortCards(pdst, dstLen, true, true, true);

			int c2_0 = 0, cpylen_0 = 0;
			uint8_t dst2_0[2][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_0 = EnumRepeatCards(psrc, srcLen, 2, dst2_0, size2, cpy_0, cpylen_0);
				if (c2_0 != 2) {
					S13S::CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c2_0 == 2);
			}
			int c2_1 = 0, cpylen_1 = 0;
			uint8_t dst2_1[2][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_1 = EnumRepeatCards(pdst, dstLen, 2, dst2_1, size2, cpy_1, cpylen_1);
				if (c2_1 != 2) {
					S13S::CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c2_1 == 2);
			}
			//先比较最大的对子，再比较次大的对子，最后比较单张
			uint8_t s0, p0;
			if (GetCardPoint(dst2_0[0][0]) > GetCardPoint(dst2_0[1][0])) {
				s0 = GetCardPoint(dst2_0[0][0]);
				p0 = GetCardPoint(dst2_0[1][0]);
			}
			else {
				s0 = GetCardPoint(dst2_0[1][0]);
				p0 = GetCardPoint(dst2_0[0][0]);
			}
			uint8_t s1, p1;
			if (GetCardPoint(dst2_1[0][0]) > GetCardPoint(dst2_1[1][0])) {
				s1 = GetCardPoint(dst2_1[0][0]);
				p1 = GetCardPoint(dst2_1[1][0]);
			}
			else {
				s1 = GetCardPoint(dst2_1[1][0]);
				p1 = GetCardPoint(dst2_1[0][0]);
			}
			if (s0 != s1) {
				//比较最大的对子
				return s0 - s1;
			}
			if (sd && p0 != p1) {
				//比较次大的对子
				return p0 - p1;
			}
			if (sp && cpylen_0 && cpylen_1) {
				//比较单张的大小
				return GetCardPoint(cpy_0[0]) - GetCardPoint(cpy_1[0]);
			}
			break;
		}
		//对子之间比较
		case Ty20: {
			assert(srcLen > 0);
			assert(dstLen > 0);
			uint8_t psrc[MaxSZ] = { 0 }, pdst[MaxSZ] = { 0 };
			memcpy(psrc, src, srcLen);
			memcpy(pdst, dst, dstLen);
			SortCards(psrc, srcLen, true, true, true);
			SortCards(pdst, dstLen, true, true, true);

			int c2_0 = 0, cpylen_0 = 0;
			uint8_t dst2_0[1][4] = { 0 };
			uint8_t cpy_0[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_0 = EnumRepeatCards(psrc, srcLen, 2, dst2_0, size2, cpy_0, cpylen_0);
				if (c2_0 != 1) {
					S13S::CGameLogic::PrintCardList(src, srcLen);
				}
				assert(c2_0 == 1);
			}
			int c2_1 = 0, cpylen_1 = 0;
			uint8_t dst2_1[1][4] = { 0 };
			uint8_t cpy_1[MaxSZ] = { 0 };
			{
				int const size2 = 6;
				c2_1 = EnumRepeatCards(pdst, dstLen, 2, dst2_1, size2, cpy_1, cpylen_1);
				if (c2_1 != 1) {
					S13S::CGameLogic::PrintCardList(dst, dstLen);
				}
				assert(c2_1 == 1);
			}
			uint8_t p0 = GetCardPoint(dst2_0[0][0]);
			uint8_t p1 = GetCardPoint(dst2_1[0][0]);
			//先比较对子大小，再比较其余散牌大小
			if (p0 != p1) {
				return p0 - p1;
			}
			if (sp && cpylen_0 && cpylen_1) {
				//对子大小相同，比较散牌大小
				return CompareCardPointBy(cpy_0, cpylen_0, cpy_1, cpylen_1, clr);
			}
			break;
		}
		//默认情况
		case TyNil:
		default: return sp ? CompareCardPointBy(src, srcLen, dst, dstLen, clr) : 0;
		}
		//assert(false);
		return 0;
	}

	//枚举牌型测试
	void CGameLogic::TestEnumCards(int size) {
		CGameLogic g;
		handinfo_t hand;
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
			int c = CGameLogic::AnalyseHandCards(cards, MAX_COUNT, size, hand);
			//有特殊牌型时
			//pause = (hand.specialTy_ != S13S::TyNil);
			//有三同花顺/三同花/三顺子时
			//pause = ((hand.specialTy_ == SSS::TyThree123) || (hand.specialTy_ == SSS::TyThree123sc));
			//没有重复四张，有2个重复三张和3个重复二张
			//pause = (hand.classify.c4 == 0 &&
			//	hand.classify.c3 >= 2 &&
			//	hand.classify.c2 >= 3);
			//if (pause) {
				//查看所有枚举牌型
				//hand.rootEnumList->PrintEnumCards(false, TyAllBase);
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
	void CGameLogic::TestEnumCards(char const* filename) {
		std::vector<std::string> lines;
		readFile(filename, lines, ";;");
		assert(lines.size() >= 2);
		//1->文件读取手牌 0->随机生成13张牌
		int flag = atoi(lines[0].c_str());
		//默认最多枚举多少组墩，开元/德胜是3组
		int size = atoi(lines[1].c_str());
		//1->文件读取手牌 0->随机生成13张牌
		if (flag > 0) {
			//assert(lines.size() == 3);
			CGameLogic g;
			handinfo_t hand;
			uint8_t cards[MAX_COUNT + 10] = { 0 };
			//line[2]构造一副手牌13张
			int n = CGameLogic::MakeCardList(lines[2], cards, MAX_COUNT);
			//assert(n == 13);
			//手牌排序
			CGameLogic::SortCards(cards, n, true, true, true);
			printf("=================================================\n\n");
			//一副手牌
			CGameLogic::PrintCardList(cards, n);
			//手牌牌型分析
			int c = CGameLogic::AnalyseHandCards(cards, n, size, hand);
			//查看所有枚举牌型
			//hand.rootEnumList->PrintEnumCards(false, TyAllBase);
			//查看手牌特殊牌型
			//hand.PrintSpecCards();
			//查看手牌枚举三墩牌型
			hand.PrintEnumCards(false);
			//查看重复牌型和散牌
			hand.classify.PrintCardList();
			printf("--- *** c = %d %s\n\n\n\n", c, hand.StringSpecialTy().c_str());
			//{
			//	S13S::HandTy ty;
			//	ty = S13S::CGameLogic::GetDunCardHandTy(S13S::DunFirst, cards, 3);
			//	printf("%s\n", S13S::CGameLogic::StringHandTy(ty).c_str());
			//	ty = S13S::CGameLogic::GetDunCardHandTy(S13S::DunSecond, cards + 3, 5);
			//	printf("%s\n", S13S::CGameLogic::StringHandTy(ty).c_str());
			//	ty = S13S::CGameLogic::GetDunCardHandTy(S13S::DunLast, cards + 8, 5);
			//	printf("%s\n", S13S::CGameLogic::StringHandTy(ty).c_str());
			//}
		}
		else {
			TestEnumCards(size);
		}
	}
	
	//玩家发牌测试
	void  CGameLogic::TestPlayerCards() {
		//游戏逻辑类
		S13S::CGameLogic				g;
		//所有玩家手牌
		uint8_t							handCards[GAME_PLAYER][MAX_COUNT];
		//手牌牌型分析结果
		S13S::CGameLogic::handinfo_t	handInfos[GAME_PLAYER];
		//最多枚举多少组最优墩，开元/德胜是3组
		int enum_group_sz = 15;
		//初始化
		g.InitCards();
		//洗牌
		g.ShuffleCards();
		do {
			{
				//给各个玩家发牌
			restart:
				assert(GAME_PLAYER <= 4);
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//余牌不够则重新洗牌，然后重新分发给各个玩家
						if (g.Remaining() < MAX_COUNT) {
							g.ShuffleCards();
							goto restart;
						}
						//发牌
						g.DealCards(MAX_COUNT, &(handCards[i])[0]);
					}
				}
			}
			{
				//各个玩家手牌分析
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//手牌排序
						S13S::CGameLogic::SortCards(&(handCards[i])[0], MAX_COUNT, true, true, true);
						printf("\n\n========================================================================\n");
						printf("--- *** chairID = [%d]\n", i);
						//一副手牌
						S13S::CGameLogic::PrintCardList(&(handCards[i])[0], MAX_COUNT);
						//手牌牌型分析
						int c = S13S::CGameLogic::AnalyseHandCards(&(handCards[i])[0], MAX_COUNT, enum_group_sz, handInfos[i]);
						//查看所有枚举牌型
						handInfos[i].rootEnumList->PrintEnumCards(false, S13S::Ty123sc);
						//查看手牌特殊牌型
						handInfos[i].PrintSpecCards();
						//查看手牌枚举三墩牌型
						handInfos[i].PrintEnumCards();
						//查看重复牌型和散牌
						handInfos[i].classify.PrintCardList();
						printf("--- *** c = %d %s\n\n\n\n", c, handInfos[i].StringSpecialTy().c_str());
					}
				}
			}
		} while ('q' != getchar());
	}

	//开始游戏测试
	void CGameLogic::TestProtoCards() {
		//游戏逻辑类
		S13S::CGameLogic				g;
		//所有玩家手牌
		uint8_t							handCards[GAME_PLAYER][MAX_COUNT];
		//手牌牌型分析结果
		S13S::CGameLogic::handinfo_t	handInfos[GAME_PLAYER];
		//最多枚举多少组最优墩，开元/德胜是3组
		int enum_group_sz = 3;
		//初始化
		g.InitCards();
		//洗牌
		g.ShuffleCards();
		do {
			{
				//给各个玩家发牌
			restart:
				assert(GAME_PLAYER <= 4);
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//余牌不够则重新洗牌，然后重新分发给各个玩家
						if (g.Remaining() < MAX_COUNT) {
							g.ShuffleCards();
							goto restart;
						}
						//发牌
						g.DealCards(MAX_COUNT, &(handCards[i])[0]);
					}
				}
			}
			{
				//各个玩家手牌分析
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//手牌排序
						S13S::CGameLogic::SortCards(&(handCards[i])[0], MAX_COUNT, true, true, true);
						printf("\n\n========================================================================\n");
						printf("--- *** chairID = [%d]\n", i);
						//一副手牌
						S13S::CGameLogic::PrintCardList(&(handCards[i])[0], MAX_COUNT);
						//手牌牌型分析
						int c = S13S::CGameLogic::AnalyseHandCards(&(handCards[i])[0], MAX_COUNT, enum_group_sz, handInfos[i]);
						//查看所有枚举牌型
						//handInfos[i].rootEnumList->PrintEnumCards(false, S13S::Ty123sc);
						//查看手牌特殊牌型
						handInfos[i].PrintSpecCards();
						//查看手牌枚举三墩牌型
						handInfos[i].PrintEnumCards();
						//查看重复牌型和散牌
						handInfos[i].classify.PrintCardList();
						printf("--- *** c = %d %s\n\n\n\n", c, handInfos[i].StringSpecialTy().c_str());
					}
				}
			}
			//开始游戏测试
			{
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						printf("\n\n========================================================================\n");
						printf("--- *** chairID = [%d]\n", i);
						//一副手牌
						S13S::CGameLogic::PrintCardList(&(handCards[i])[0], MAX_COUNT);
						//查看重复牌型和散牌
						handInfos[i].classify.PrintCardList();
						//查看所有枚举牌型
						handInfos[i].rootEnumList->PrintEnumCards(false, S13S::Ty123sc);
						//查看手牌特殊牌型
						handInfos[i].PrintSpecCards();
						//查看手牌枚举三墩牌型
						handInfos[i].PrintEnumCards();
						//游戏开始，填充相关数据
						s13s::CMD_S_GameStart reqdata;
						//一副手牌
						s13s::HandCards* handcards = reqdata.mutable_handcards();
						//一副13张手牌
						handcards->set_cards(&(handCards[i])[0], MAX_COUNT);
						//标记手牌特殊牌型
						handcards->set_specialty(handInfos[i].SpecialTy());
						int j = 0;
						for (std::vector<S13S::CGameLogic::groupdun_t>::const_iterator it = handInfos[i].spec_groups.begin();
							it != handInfos[i].spec_groups.end(); ++it) {
							assert(handInfos[i].spec_groups.size() == 1);
							//特殊牌型放在枚举几组最优解前面
							s13s::GroupDunData* group = handcards->add_groups();
							//从哪墩开始的
							group->set_start(it->start);
							//总体对应特殊牌型
							group->set_specialty(it->specialTy);
							//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
							printf("第[%d]组\t=>>\t", j++ + 1);
							for (int k = S13S::DunFirst; k <= S13S::DunLast; ++k) {
								//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
								s13s::DunData* dun_i = group->add_duns();
								//标记0-头/1-中/2-尾
								dun_i->set_id(k);
								//墩对应普通牌型
								dun_i->set_ty(it->duns[k].ty_);
								//墩对应牌数c(3/5/5)
								dun_i->set_c(it->duns[k].c);
								//墩牌数据(头墩(3)/中墩(5)/尾墩(5))
								dun_i->set_cards(it->duns[k].cards, it->duns[k].c);
								printf("dun[%d] c:=%d\t", k, it->duns[k].c);
							}
							printf("\n\n");
						}
						for (std::vector<S13S::CGameLogic::groupdun_t>::const_iterator it = handInfos[i].enum_groups.begin();
							it != handInfos[i].enum_groups.end(); ++it) {
							//枚举几组最优墩
							s13s::GroupDunData* group = handcards->add_groups();
							//从哪墩开始的
							group->set_start(it->start);
							//总体对应特殊牌型
							group->set_specialty(it->specialTy);
							//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
							printf("第[%d]组\t=>>\t", j++ + 1);
							for (int k = S13S::DunFirst; k <= S13S::DunLast; ++k) {
								//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
								s13s::DunData* dun_i = group->add_duns();
								//标记0-头/1-中/2-尾
								dun_i->set_id(k);
								//墩对应普通牌型
								dun_i->set_ty(it->duns[k].ty_);
								//墩对应牌数c(3/5/5)
								dun_i->set_c(it->duns[k].c);
								//墩牌数据(头墩(3)/中墩(5)/尾墩(5))
								dun_i->set_cards(it->duns[k].cards, it->duns[k].c);
								printf("dun[%d] c:=%d\t", k, it->duns[k].c);
							}
							printf("\n\n");
						}
						{
							//序列化std::string
							std::string data = reqdata.SerializeAsString();
							std::string const& typeName = reqdata.GetTypeName();//typename
							//反序列化
							s13s::CMD_S_GameStart rspdata;
							rspdata.ParseFromArray(data.c_str(), data.length());
							//转换json格式
							std::string jsonstr;
							PB2JSON::Pb2Json::PbMsg2JsonStr(rspdata, jsonstr, true);
							printf("\n--- *** %s\n", typeName.c_str());
							printf("%s\n\n", jsonstr.c_str());
						}
						{
							//序列化bytes
							int len = reqdata.ByteSizeLong();//len
							uint8_t *data = new uint8_t[len];
							reqdata.SerializeToArray(data, len);//data
							std::string const& typeName = reqdata.GetTypeName();//typename
							//反序列化
							s13s::CMD_S_GameStart rspdata;
							rspdata.ParseFromArray(data, len);
							delete[] data;
							//转换json格式
							std::string jsonstr;
							PB2JSON::Pb2Json::PbMsg2JsonStr(rspdata, jsonstr, true);
							printf("\n--- *** %s\n", typeName.c_str());
							printf("%s\n\n", jsonstr.c_str());
						}
						{
							//序列化bytes
							int len = reqdata.ByteSizeLong();//len
							uint8_t *data = new uint8_t[len];
							reqdata.SerializeWithCachedSizesToArray(data);//data
							std::string const& typeName = reqdata.GetTypeName();//typename
							//反序列化
							s13s::CMD_S_GameStart rspdata;
							rspdata.ParseFromArray(data, len);
							delete[] data;
							//转换json格式
							std::string jsonstr;
							PB2JSON::Pb2Json::PbMsg2JsonStr(rspdata, jsonstr, true);
							printf("\n--- *** %s\n", typeName.c_str());
							printf("%s\n\n", jsonstr.c_str());
						}
						printf("--- *** chairID = [%d] c = %d %s\n\n\n\n", i, handInfos[i].spec_groups.size() + handInfos[i].enum_groups.size(), handInfos[i].StringSpecialTy().c_str());
					}
				}
			}
		} while ('q' != getchar());
	}

	//手动摆牌测试
	void CGameLogic::TestManualCards() {
		//游戏逻辑类
		S13S::CGameLogic				g;
		//所有玩家手牌
		uint8_t							handCards[GAME_PLAYER][MAX_COUNT];
		//手牌牌型分析结果
		S13S::CGameLogic::handinfo_t	handInfos[GAME_PLAYER];
		//最多枚举多少组最优墩，开元/德胜是3组
		int enum_group_sz = 3;
		//初始化
		g.InitCards();
		//洗牌
		g.ShuffleCards();
		do {
			{
				//给各个玩家发牌
			restart:
				assert(GAME_PLAYER <= 4);
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//余牌不够则重新洗牌，然后重新分发给各个玩家
						if (g.Remaining() < MAX_COUNT) {
							g.ShuffleCards();
							goto restart;
						}
						//发牌
						g.DealCards(MAX_COUNT, &(handCards[i])[0]);
					}
				}
			}
			{
				//各个玩家手牌分析
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//手牌排序
						S13S::CGameLogic::SortCards(&(handCards[i])[0], MAX_COUNT, true, true, true);
						printf("\n\n========================================================================\n");
						printf("--- *** chairID = [%d]\n", i);
						//一副手牌
						S13S::CGameLogic::PrintCardList(&(handCards[i])[0], MAX_COUNT);
						//手牌牌型分析
						int c = S13S::CGameLogic::AnalyseHandCards(&(handCards[i])[0], MAX_COUNT, enum_group_sz, handInfos[i]);
						//查看所有枚举牌型
						handInfos[i].rootEnumList->PrintEnumCards(false, S13S::Ty123sc);
						//查看手牌特殊牌型
						handInfos[i].PrintSpecCards();
						//查看手牌枚举三墩牌型
						handInfos[i].PrintEnumCards();
						//查看重复牌型和散牌
						handInfos[i].classify.PrintCardList();
						printf("--- *** c = %d %s\n\n\n\n", c, handInfos[i].StringSpecialTy().c_str());
					}
				}
			}
			//手动摆牌测试
			{
				//遍历各个座椅玩家
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//服务器应答数据
						s13s::CMD_S_ManualCards rspdata;
						for (int k = 1; k <= 4; ++k) {
							//客户端请求数据
							s13s::CMD_C_ManualCards reqdata;
							//if (!reqdata.ParseFromArray(data, dataLen)) {
							//	return false;
							//}
							switch (k) {
							case 1: {
								//第一次点击手动摆牌按钮，获取初始所有枚举牌型，reqdata传空数据
								break;
							}
							case 2: {
								//第二次从13张手牌中随机抽5张放入尾墩
								int j = RandomBetween(0, MAX_COUNT - 5);
								reqdata.set_dt(S13S::DunLast);
								reqdata.set_cards(&(handCards[i])[j], 5);
								break;
							}
							case 3: {
								assert(rspdata.cpy().size() == 8);
								//第三次从余下手牌中随机抽5张放入中墩
								int j = RandomBetween(0, rspdata.cpy().size() - 5);
								reqdata.set_dt(S13S::DunSecond);
								reqdata.set_cards(rspdata.cpy().c_str() + j, 5);
								break;
							}
							case 4: {
								assert(rspdata.cpy().size() == 3);
								//第四次从余下手牌中随机抽3张放入头墩
								int j = RandomBetween(0, rspdata.cpy().size() - 3);
								assert(j == 0);
								reqdata.set_dt(S13S::DunFirst);
								reqdata.set_cards(rspdata.cpy().c_str() + j, 3);
								break;
							}
							}
							{
								//转换json格式
								std::string jsonstr;
								PB2JSON::Pb2Json::PbMsg2JsonStr(reqdata, jsonstr, true);
								std::string const& typeName = reqdata.GetTypeName();//typename
								printf("\n--- *** %s\n", typeName.c_str());
								printf("%s\n\n", jsonstr.c_str());
							}
							rspdata.Clear();
							//服务器应答数据
							//s13s::CMD_S_ManualCards rspdata;
							S13S::CGameLogic::EnumTree enumList;
							S13S::CGameLogic::EnumTree* rootEnumList = NULL;
							//客户端选择了哪一墩
							S13S::DunTy dt = (S13S::DunTy)(reqdata.dt());
							//客户端选择了哪些牌
							uint8_t * cards = (uint8_t *)reqdata.cards().c_str();
							//客户端选择了几张牌
							int len = reqdata.cards().size();
							if (len > 0) {
								S13S::CGameLogic::SortCards(cards, len, true, true, true);
								//S13S::CGameLogic::PrintCardList(cards, len);
								//对客户端选择的一组牌，进行单墩牌型判断
								S13S::HandTy ty = S13S::CGameLogic::GetDunCardHandTy(dt, cards, len);
								//手动摆牌，头墩要么三条/对子/乌龙，同花顺/同花/顺子都要改成乌龙
								if (dt == S13S::DunFirst && (ty == S13S::Ty123sc || ty == S13S::Tysc || ty == S13S::Ty123)) {
									ty = S13S::Tysp;
								}
								//判断手动摆牌是否倒水
								if (handInfos[i].IsInverted(dt, cards, len, ty)) {
									handInfos[i].ResetManual();
									continue;
								}
								//手动选牌组墩，给指定墩(头/中/尾墩)选择一组牌
								if (!handInfos[i].SelectAs(dt, cards, len, ty)) {
									continue;
								}
								//返回组墩后剩余牌
								uint8_t cpy[S13S::MaxSZ] = { 0 };
								int cpylen = 0;
								handInfos[i].GetLeftCards(&(handCards[i])[0], MAX_COUNT, cpy, cpylen);
								//3/8/10
								len = handInfos[i].GetManualC() < 10 ? 5 : 3;
								//从余牌中枚举所有牌型
								S13S::CGameLogic::classify_t classify = { 0 };
								S13S::CGameLogic::EnumCards(cpy, cpylen, len, classify, enumList, dt);
								//指向新的所有枚举牌型
								rootEnumList = &enumList;
								//客户端选择了哪一墩，标记0-头/1-中/2-尾
								rspdata.set_dt(dt);
								//墩对应牌型
								rspdata.set_ty(ty);
								//剩余牌
								rspdata.set_cpy(cpy, cpylen);
							}
							else {
								//重新摆牌重置手动摆牌
								handInfos[i].ResetManual();
								assert(handInfos[i].GetManualC() == 0);
								//指向初始所有枚举牌型
								rootEnumList = handInfos[i].rootEnumList;
							}
							//所有枚举牌型
							s13s::EnumCards* cards_enums = rspdata.mutable_enums();
							assert(rootEnumList != NULL);
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v123sc.begin();
								it != rootEnumList->v123sc.end(); ++it) {
								cards_enums->add_v123sc(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v40.begin();
								it != rootEnumList->v40.end(); ++it) {
								cards_enums->add_v40(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v32.begin();
								it != rootEnumList->v32.end(); ++it) {
								cards_enums->add_v32(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->vsc.begin();
								it != rootEnumList->vsc.end(); ++it) {
								cards_enums->add_vsc(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v123.begin();
								it != rootEnumList->v123.end(); ++it) {
								cards_enums->add_v123(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v30.begin();
								it != rootEnumList->v30.end(); ++it) {
								cards_enums->add_v30(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v22.begin();
								it != rootEnumList->v22.end(); ++it) {
								cards_enums->add_v22(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v20.begin();
								it != rootEnumList->v20.end(); ++it) {
								cards_enums->add_v20(&it->front(), it->size());
							}
							{
								//序列化std::string
								//std::string data = rspdata.SerializeAsString();
								//序列化bytes
								int len = rspdata.ByteSizeLong();//len
								uint8_t *data = new uint8_t[len];
								rspdata.SerializeToArray(data, len);//data
								std::string const& typeName = rspdata.GetTypeName();//typename
								delete[] data;
								//转换json格式
								std::string jsonstr;
								PB2JSON::Pb2Json::PbMsg2JsonStr(rspdata, jsonstr, true);
								printf("\n--- *** %s\n", typeName.c_str());
								printf("%s\n\n", jsonstr.c_str());
							}
						}
					}
				}
			}
		} while ('q' != getchar());
	}

	//确定牌型/比牌测试
	//先让每个玩家确定手牌三墩牌型，手动摆牌或者从枚举几组最优解中任选一组作为手牌牌型与其他玩家比牌，
	//再玩家之间两两比牌，头墩与头墩比，中墩与中墩比，尾墩与尾墩比，
	//并计算输赢积分(输赢多少水，统计打枪/全垒打)
	void CGameLogic::TestCompareCards() {
		//游戏逻辑类
		S13S::CGameLogic				g;
		//所有玩家手牌
		uint8_t							handCards[GAME_PLAYER][MAX_COUNT];
		//手牌牌型分析结果
		S13S::CGameLogic::handinfo_t	handInfos[GAME_PLAYER];
		//最多枚举多少组最优墩，开元/德胜是3组
		int enum_group_sz = 3;
		//初始化
		g.InitCards();
		//洗牌
		g.ShuffleCards();
		do {
			{
				//给各个玩家发牌
			restart:
				assert(GAME_PLAYER <= 4);
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//余牌不够则重新洗牌，然后重新分发给各个玩家
						if (g.Remaining() < MAX_COUNT) {
							g.ShuffleCards();
							goto restart;
						}
						//发牌
						g.DealCards(MAX_COUNT, &(handCards[i])[0]);
					}
				}
			}
			{
				//各个玩家手牌分析
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//手牌排序
						S13S::CGameLogic::SortCards(&(handCards[i])[0], MAX_COUNT, true, true, true);
						printf("\n\n========================================================================\n");
						printf("--- *** chairID = [%d]\n", i);
						//一副手牌
						S13S::CGameLogic::PrintCardList(&(handCards[i])[0], MAX_COUNT);
						//手牌牌型分析
						int c = S13S::CGameLogic::AnalyseHandCards(&(handCards[i])[0], MAX_COUNT, enum_group_sz, handInfos[i]);
						//有特殊牌型时
						//if (handInfos[i].specialTy_ == S13S::TyNil) {
						//	goto restart;
						//}
						//查看所有枚举牌型
						//handInfos[i].rootEnumList->PrintEnumCards(false, S13S::Ty123sc);
						//查看手牌特殊牌型
						handInfos[i].PrintSpecCards();
						//查看手牌枚举三墩牌型
						handInfos[i].PrintEnumCards();
						//查看重复牌型和散牌
						//handInfos[i].classify.PrintCardList();
						printf("--- *** c = %d %s\n\n\n\n", c, handInfos[i].StringSpecialTy().c_str());
					}
				}
			}
			//手动摆牌测试
			{
				//遍历各个座椅玩家
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//服务器应答数据
						s13s::CMD_S_ManualCards rspdata;
						for (int k = 1; k <= 4; ++k) {
							//客户端请求数据
							s13s::CMD_C_ManualCards reqdata;
							//if (!reqdata.ParseFromArray(data, dataLen)) {
							//	return false;
							//}
							switch (k) {
							case 1: {
								//第一次点击手动摆牌按钮，获取初始所有枚举牌型，reqdata传空数据
								break;
							}
							case 2: {
								//第二次从13张手牌中随机抽5张放入尾墩
								int j = RandomBetween(0, MAX_COUNT - 5);
								reqdata.set_dt(S13S::DunLast);
								reqdata.set_cards(&(handCards[i])[j], 5);
								break;
							}
							case 3: {
								assert(rspdata.cpy().size() == 8);
								//第三次从余下手牌中随机抽5张放入中墩
								int j = RandomBetween(0, rspdata.cpy().size() - 5);
								reqdata.set_dt(S13S::DunSecond);
								reqdata.set_cards(rspdata.cpy().c_str() + j, 5);
								break;
							}
							case 4: {
								assert(rspdata.cpy().size() == 3);
								//第四次从余下手牌中随机抽3张放入头墩
								int j = RandomBetween(0, rspdata.cpy().size() - 3);
								assert(j == 0);
								reqdata.set_dt(S13S::DunFirst);
								reqdata.set_cards(rspdata.cpy().c_str() + j, 3);
								break;
							}
							}
							{
								//转换json格式
								std::string jsonstr;
								PB2JSON::Pb2Json::PbMsg2JsonStr(reqdata, jsonstr, true);
								std::string const& typeName = reqdata.GetTypeName();//typename
								printf("\n--- *** %s\n", typeName.c_str());
								printf("%s\n\n", jsonstr.c_str());
							}
							rspdata.Clear();
							//服务器应答数据
							//s13s::CMD_S_ManualCards rspdata;
							S13S::CGameLogic::EnumTree enumList;
							S13S::CGameLogic::EnumTree* rootEnumList = NULL;
							//客户端选择了哪一墩
							S13S::DunTy dt = (S13S::DunTy)(reqdata.dt());
							//客户端选择了哪些牌
							uint8_t * cards = (uint8_t *)reqdata.cards().c_str();
							//客户端选择了几张牌
							int len = reqdata.cards().size();
							if (len > 0) {
								S13S::CGameLogic::SortCards(cards, len, true, true, true);
								//S13S::CGameLogic::PrintCardList(cards, len);
								//对客户端选择的一组牌，进行单墩牌型判断
								S13S::HandTy ty = S13S::CGameLogic::GetDunCardHandTy(dt, cards, len);
								//手动摆牌，头墩要么三条/对子/乌龙，同花顺/同花/顺子都要改成乌龙
								if (dt == S13S::DunFirst && (ty == S13S::Ty123sc || ty == S13S::Tysc || ty == S13S::Ty123)) {
									ty = S13S::Tysp;
								}
								//判断手动摆牌是否倒水
								if (handInfos[i].IsInverted(dt, cards, len, ty)) {
									handInfos[i].ResetManual();
									continue;
								}
								//手动选牌组墩，给指定墩(头/中/尾墩)选择一组牌
								if (!handInfos[i].SelectAs(dt, cards, len, ty)) {
									continue;
								}
								//返回组墩后剩余牌
								uint8_t cpy[S13S::MaxSZ] = { 0 };
								int cpylen = 0;
								handInfos[i].GetLeftCards(&(handCards[i])[0], MAX_COUNT, cpy, cpylen);
								//3/8/10
								len = handInfos[i].GetManualC() < 10 ? 5 : 3;
								//从余牌中枚举所有牌型
								S13S::CGameLogic::classify_t classify = { 0 };
								S13S::CGameLogic::EnumCards(cpy, cpylen, len, classify, enumList, dt);
								//指向新的所有枚举牌型
								rootEnumList = &enumList;
								//客户端选择了哪一墩，标记0-头/1-中/2-尾
								rspdata.set_dt(dt);
								//墩对应牌型
								rspdata.set_ty(ty);
								//剩余牌
								rspdata.set_cpy(cpy, cpylen);
							}
							else {
								//重新摆牌重置手动摆牌
								handInfos[i].ResetManual();
								assert(handInfos[i].GetManualC() == 0);
								//指向初始所有枚举牌型
								rootEnumList = handInfos[i].rootEnumList;
							}
							//所有枚举牌型
							s13s::EnumCards* cards_enums = rspdata.mutable_enums();
							assert(rootEnumList != NULL);
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v123sc.begin();
								it != rootEnumList->v123sc.end(); ++it) {
								cards_enums->add_v123sc(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v40.begin();
								it != rootEnumList->v40.end(); ++it) {
								cards_enums->add_v40(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v32.begin();
								it != rootEnumList->v32.end(); ++it) {
								cards_enums->add_v32(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->vsc.begin();
								it != rootEnumList->vsc.end(); ++it) {
								cards_enums->add_vsc(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v123.begin();
								it != rootEnumList->v123.end(); ++it) {
								cards_enums->add_v123(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v30.begin();
								it != rootEnumList->v30.end(); ++it) {
								cards_enums->add_v30(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v22.begin();
								it != rootEnumList->v22.end(); ++it) {
								cards_enums->add_v22(&it->front(), it->size());
							}
							for (std::vector<S13S::CGameLogic::EnumTree::CardData>::const_iterator it = rootEnumList->v20.begin();
								it != rootEnumList->v20.end(); ++it) {
								cards_enums->add_v20(&it->front(), it->size());
							}
							{
								//序列化std::string
								//std::string data = rspdata.SerializeAsString();
								//序列化bytes
								int len = rspdata.ByteSizeLong();//len
								uint8_t *data = new uint8_t[len];
								rspdata.SerializeToArray(data, len);//data
								std::string const& typeName = rspdata.GetTypeName();//typename
								delete[] data;
								//转换json格式
								std::string jsonstr;
								PB2JSON::Pb2Json::PbMsg2JsonStr(rspdata, jsonstr, true);
								printf("\n--- *** %s\n", typeName.c_str());
								printf("%s\n\n", jsonstr.c_str());
							}
						}
					}
				}
			}
			//确定牌型/比牌测试
			{
				//确定牌型的玩家数
				int selectcount = 0;
				//遍历各个座椅玩家
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//客户端请求数据
						s13s::CMD_C_MakesureDunHandTy reqdata;
// 						if (!reqdata.ParseFromArray(data, dataLen)) {
// 							return false;
// 						}
						if (rand() % 2 == 0) {
							//手动摆牌构成的一组进行比牌
							reqdata.set_groupindex(-1);
						}
						else {
							//直接选择枚举中的一组进行比牌
							int groupindex = RandomBetween(0, handInfos[i].enum_groups.size() - 1);
							reqdata.set_groupindex(groupindex);
						}
						{
							//转换json格式
							std::string jsonstr;
							PB2JSON::Pb2Json::PbMsg2JsonStr(reqdata, jsonstr, true);
							std::string const& typeName = reqdata.GetTypeName();//typename
							printf("\n--- *** %s\n", typeName.c_str());
							printf("%s\n\n", jsonstr.c_str());
						}
						//玩家已经确认过牌型
						if (handInfos[i].HasSelected()) {
							assert(false);
							return;
						}
						//是否进行过手动任意摆牌
						if (reqdata.groupindex() == -1 && !handInfos[i].HasManualGroup()) {
							assert(handInfos[i].GetManualC() < MAX_COUNT);
							return;
						}
						//确认玩家手牌三墩牌型
						if (!handInfos[i].Select(reqdata.groupindex())) {
							assert(false);
							return;
						}
						//检查是否都确认牌型了
						if (++selectcount < GAME_PLAYER) {
						}
					}
				}
				//玩家之间两两比牌，头墩与头墩比，中墩与中墩比，尾墩与尾墩比
				s13s::CMD_S_CompareCards compareCards[GAME_PLAYER];
				//所有玩家牌型都确认了，比牌
				int c = 0;
				int chairIDs[GAME_PLAYER] = { 0 };
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//用于求两两组合
						chairIDs[c++] = i;
						//比牌玩家桌椅ID
						compareCards[i].mutable_player()->set_chairid(i);
						//获取座椅玩家确定的三墩牌型
						S13S::CGameLogic::groupdun_t const *player_select = handInfos[i].GetSelected();
						//桌椅玩家选择一组墩(含头墩/中墩/尾墩)
						s13s::GroupDunData* player_group = compareCards[i].mutable_player()->mutable_group();
						//从哪墩开始的
						player_group->set_start(S13S::DunFirst);
						//总体对应特殊牌型 ////////////
						player_group->set_specialty(player_select->specialTy);
						//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
						for (int d = S13S::DunFirst; d <= S13S::DunLast; ++d) {
							//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
							s13s::DunData* player_dun_i = player_group->add_duns();
							//标记0-头/1-中/2-尾
							player_dun_i->set_id(d);
							//墩对应普通牌型
							player_dun_i->set_ty(player_select->duns[d].ty_);
							//墩对应牌数c(3/5/5)
							player_dun_i->set_c(player_select->duns[d].c);
							//墩牌数据(头墩(3)/中墩(5)/尾墩(5))
							player_dun_i->set_cards(player_select->duns[d].cards, player_select->duns[d].c);
						}
					}
				}
				assert(c >= MIN_GAME_PLAYER);
				std::vector<std::vector<int>> vec;
				CFuncC fnC;
				fnC.FuncC(c, 2, vec);
				//CFuncC::Print(vec);
				for (std::vector<std::vector<int>>::const_iterator it = vec.begin();
					it != vec.end(); ++it) {
					assert(it->size() == 2);//两两比牌
					int src_chairid = chairIDs[(*it)[0]];//src椅子ID
					int dst_chairid = chairIDs[(*it)[1]];//dst椅子ID
					assert(src_chairid < GAME_PLAYER);
					assert(dst_chairid < GAME_PLAYER);
					//获取src确定的三墩牌型
					S13S::CGameLogic::groupdun_t const *src = handInfos[src_chairid].GetSelected();
					//获取dst确定的三墩牌型
					S13S::CGameLogic::groupdun_t const *dst = handInfos[dst_chairid].GetSelected();
					{
						//追加src比牌对象 ////////////
						s13s::ComparePlayer* src_peer = compareCards[src_chairid].add_peers();
						{
							//比牌对象桌椅ID
							src_peer->set_chairid(dst_chairid);
							//比牌对象选择一组墩
							s13s::GroupDunData* src_peer_select = src_peer->mutable_group();
							//从哪墩开始的
							src_peer_select->set_start(S13S::DunFirst);
							//总体对应特殊牌型 ////////////
							src_peer_select->set_specialty(dst->specialTy);
							//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
							for (int i = S13S::DunFirst; i <= S13S::DunLast; ++i) {
								//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
								s13s::DunData* src_peer_dun_i = src_peer_select->add_duns();
								//标记0-头/1-中/2-尾
								src_peer_dun_i->set_id(i);
								//墩对应普通牌型
								src_peer_dun_i->set_ty(dst->duns[i].ty_);
								//墩对应牌数c(3/5/5)
								src_peer_dun_i->set_c(dst->duns[i].c);
								//墩牌数据(头墩(3)/中墩(5)/尾墩(5))
								src_peer_dun_i->set_cards(dst->duns[i].cards, dst->duns[i].c);
							}
						}
						//追加src比牌结果 ////////////
						s13s::CompareResult* src_result = compareCards[src_chairid].add_results();
					}
					{
						//追加dst比牌对象 ////////////
						s13s::ComparePlayer* dst_peer = compareCards[dst_chairid].add_peers();
						{
							//比牌对象桌椅ID
							dst_peer->set_chairid(src_chairid);
							//比牌对象选择一组墩
							s13s::GroupDunData* dst_peer_select = dst_peer->mutable_group();
							//从哪墩开始的
							dst_peer_select->set_start(S13S::DunFirst);
							//总体对应特殊牌型 ////////////
							dst_peer_select->set_specialty(src->specialTy);
							//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
							for (int i = S13S::DunFirst; i <= S13S::DunLast; ++i) {
								//[0]头墩(3)/[1]中墩(5)/[2]尾墩(5)
								s13s::DunData* dst_peer_dun_i = dst_peer_select->add_duns();
								//标记0-头/1-中/2-尾
								dst_peer_dun_i->set_id(i);
								//墩对应普通牌型
								dst_peer_dun_i->set_ty(src->duns[i].ty_);
								//墩对应牌数c(3/5/5)
								dst_peer_dun_i->set_c(src->duns[i].c);
								//墩牌数据(头墩3张)
								dst_peer_dun_i->set_cards(src->duns[i].cards, src->duns[i].c);
							}
						}
						//追加dst比牌结果 ////////////
						s13s::CompareResult* dst_result = compareCards[dst_chairid].add_results();
					}
					//////////////////////////////////////////////////////////////
					//比较特殊牌型
					if (src->specialTy >= TyThreesc || dst->specialTy >= TyThreesc) {
						int winner = -1, loser = -1;
						if (src->specialTy != dst->specialTy) {
							//牌型不同比牌型
							if (src->specialTy > dst->specialTy) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (src->specialTy < dst->specialTy) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								assert(false);
							}
						}
						else {
							//牌型相同，和了
						}
						if (winner == -1) {
							//和了
							winner = src_chairid; loser = dst_chairid;
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//src比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//src输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//dst比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//dst输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//至尊青龙获胜赢32水
						else if(handInfos[winner].GetSelected()->specialTy == TyZZQDragon){
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(32);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-32);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//一条龙获胜赢30水
						else if (handInfos[winner].GetSelected()->specialTy == TyOneDragon) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(30);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-30);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//十二皇族获胜赢24水
						else if (handInfos[winner].GetSelected()->specialTy == Ty12Royal) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(24);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-24);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//三同花顺获胜赢20水
						else if (handInfos[winner].GetSelected()->specialTy == TyThree123sc) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(20);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-20);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//三分天下获胜赢20水
						else if (handInfos[winner].GetSelected()->specialTy == TyThree40) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(20);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-20);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//全大获胜赢10水
						else if (handInfos[winner].GetSelected()->specialTy == TyAllBig) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(10);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-10);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//全小获胜赢10水
						else if (handInfos[winner].GetSelected()->specialTy == TyAllSmall) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(10);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-10);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//凑一色获胜赢10水
						else if (handInfos[winner].GetSelected()->specialTy == TyAllOneColor) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(10);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-10);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//双怪冲三获胜赢8水
						else if (handInfos[winner].GetSelected()->specialTy == TyTwo3220) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(8);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-8);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//四套三条获胜赢6水
						else if (handInfos[winner].GetSelected()->specialTy == TyFour30) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(6);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-6);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//五对三条获胜赢5水
						else if (handInfos[winner].GetSelected()->specialTy == TyFive2030) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(5);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-5);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//六对半获胜赢4水
						else if (handInfos[winner].GetSelected()->specialTy == TySix20) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(4);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-4);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//三顺子获胜赢4水
						else if (handInfos[winner].GetSelected()->specialTy == TyThree123) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//赢家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(4);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-4);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						//三同花获胜赢3水
						else if (handInfos[winner].GetSelected()->specialTy == TyThreesc) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//头墩输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(3);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->specialTy);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//输家输赢信息
								s13s::CompareItem* item = result->mutable_specitem();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-3);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->specialTy);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->specialTy);
								}
							}
						}
						continue;
					}
					//////////////////////////////////////////////////////////////
					//比较头墩
					{
						//单墩比牌，头墩要么三条/对子/乌龙，同花顺/同花/顺子都要改成乌龙
						if (src->duns[S13S::DunFirst].ty_ == S13S::Ty123sc ||
							src->duns[S13S::DunFirst].ty_ == S13S::Tysc ||
							src->duns[S13S::DunFirst].ty_ == S13S::Ty123) {
							//assert(src->duns[S13S::DunFirst].ty_ == S13S::Tysp);
							const_cast<S13S::CGameLogic::groupdun_t*>(src)->duns[S13S::DunFirst].ty_ = S13S::Tysp;
						}
						//单墩比牌，头墩要么三条/对子/乌龙，同花顺/同花/顺子都要改成乌龙
						if (dst->duns[S13S::DunFirst].ty_ == S13S::Ty123sc ||
							dst->duns[S13S::DunFirst].ty_ == S13S::Tysc ||
							dst->duns[S13S::DunFirst].ty_ == S13S::Ty123) {
							//assert(dst->duns[S13S::DunFirst].ty_ == S13S::Tysp);
							const_cast<S13S::CGameLogic::groupdun_t*>(dst)->duns[S13S::DunFirst].ty_ = S13S::Tysp;
						}
						int winner = -1, loser = -1;
						int dt = S13S::DunFirst;
						if (src->duns[dt].ty_ != dst->duns[dt].ty_) {
							//牌型不同比牌型
							if (src->duns[dt].ty_ > dst->duns[dt].ty_) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (src->duns[dt].ty_ < dst->duns[dt].ty_) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								assert(false);
							}
						}
						else {
							//牌型相同比大小
							assert(src->duns[dt].GetC() == 3);
							assert(dst->duns[dt].GetC() == 3);
							int bv = CGameLogic::CompareCards(
								src->duns[dt].cards, dst->duns[dt].cards, dst->duns[dt].GetC(), false, dst->duns[dt].ty_);
							if (bv > 0) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (bv < 0) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								//头墩和
								//assert(false);
							}
						}
						if (winner == -1) {
							//头墩和
							winner = src_chairid; loser = dst_chairid;
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//src比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//头墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//dst比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//头墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						//乌龙/对子获胜赢1水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Tysp ||
								 handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty20) {
								{
									int index = compareCards[winner].results_size() - 1;
									assert(index >= 0);
									//赢家比牌结果 ////////////
									s13s::CompareResult* result = compareCards[winner].mutable_results(index);
									//头墩输赢信息
									s13s::CompareItem* item = result->add_items();
									{
										//赢了
										item->set_winlost(1);
										//赢分
										item->set_score(1);
										//赢的牌型
										item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
										//对方牌型
										item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
									}
								}
								{
									int index = compareCards[loser].results_size() - 1;
									assert(index >= 0);
									//输家比牌结果 ////////////
									s13s::CompareResult* result = compareCards[loser].mutable_results(index);
									//头墩输赢信息
									s13s::CompareItem* item = result->add_items();
									{
										//输了
										item->set_winlost(-1);
										//输分
										item->set_score(-1);
										//输的牌型
										item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
										//对方牌型
										item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
									}
								}
						}
						//三条摆头墩获胜赢3水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty30) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//头墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(3);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//头墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-3);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						else {
							assert(false);
						}
					}
					//////////////////////////////////////////////////////////////
					//比较中墩
					{
						int winner = -1, loser = -1;
						int dt = S13S::DunSecond;
						if (src->duns[dt].ty_ != dst->duns[dt].ty_) {
							//牌型不同比牌型
							if (src->duns[dt].ty_ > dst->duns[dt].ty_) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (src->duns[dt].ty_ < dst->duns[dt].ty_) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								assert(false);
							}
						}
						else {
							//牌型相同比大小
							assert(src->duns[dt].GetC() == 5);
							assert(dst->duns[dt].GetC() == 5);
							int bv = S13S::CGameLogic::CompareCards(
								src->duns[dt].cards, dst->duns[dt].cards, dst->duns[dt].GetC(), false, dst->duns[dt].ty_);
							if (bv > 0) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (bv < 0) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								//中墩和
								//assert(false);
							}
						}
						if (winner == -1) {
							//中墩和
							winner = src_chairid; loser = dst_chairid;
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//src比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//dst比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						//乌龙/对子/两对/三条/顺子/同花获胜赢1水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Tysp ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty20 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty22 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty30 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty123 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Tysc) {
								{
									int index = compareCards[winner].results_size() - 1;
									assert(index >= 0);
									//赢家比牌结果 ////////////
									s13s::CompareResult* result = compareCards[winner].mutable_results(index);
									//中墩输赢信息
									s13s::CompareItem* item = result->add_items();
									{
										//赢了
										item->set_winlost(1);
										//赢分
										item->set_score(1);
										//赢的牌型
										item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
										//对方牌型
										item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
									}
								}
								{
									int index = compareCards[loser].results_size() - 1;
									assert(index >= 0);
									//输家比牌结果 ////////////
									s13s::CompareResult* result = compareCards[loser].mutable_results(index);
									//中墩输赢信息
									s13s::CompareItem* item = result->add_items();
									{
										//输了
										item->set_winlost(-1);
										//输分
										item->set_score(-1);
										//输的牌型
										item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
										//对方牌型
										item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
									}
								}
						}
						//葫芦摆中墩获胜赢2水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty32) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(2);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-2);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						//铁支摆中墩获胜赢8水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty40) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(8);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-8);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						//同花顺摆中墩获胜赢10水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty123sc) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(10);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//中墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-10);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						else {
							assert(false);
						}
					}
					//////////////////////////////////////////////////////////////
					//比较尾墩
					{
						int winner = -1, loser = -1;
						int dt = S13S::DunLast;
						if (src->duns[dt].ty_ != dst->duns[dt].ty_) {
							//牌型不同比牌型
							if (src->duns[dt].ty_ > dst->duns[dt].ty_) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (src->duns[dt].ty_ < dst->duns[dt].ty_) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								assert(false);
							}
						}
						else {
							//牌型相同比大小
							assert(src->duns[dt].GetC() == 5);
							assert(dst->duns[dt].GetC() == 5);
							int bv = S13S::CGameLogic::CompareCards(
								src->duns[dt].cards, dst->duns[dt].cards, dst->duns[dt].GetC(), false, dst->duns[dt].ty_);
							if (bv > 0) {
								winner = src_chairid; loser = dst_chairid;
							}
							else if (bv < 0) {
								winner = dst_chairid; loser = src_chairid;
							}
							else {
								//尾墩和
								//assert(false);
							}
						}
						if (winner == -1) {
							//尾墩和
							winner = src_chairid; loser = dst_chairid;
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//src比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//尾墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//dst比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//尾墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//和了
									item->set_winlost(0);
									//和分
									item->set_score(0);
									//和的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						//乌龙/对子/两对/三条/顺子/同花/葫芦获胜赢1水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Tysp ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty20 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty22 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty30 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty123 ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Tysc ||
							handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty32) {
								{
									int index = compareCards[winner].results_size() - 1;
									assert(index >= 0);
									//赢家比牌结果 ////////////
									s13s::CompareResult* result = compareCards[winner].mutable_results(index);
									//尾墩输赢信息
									s13s::CompareItem* item = result->add_items();
									{
										//赢了
										item->set_winlost(1);
										//赢分
										item->set_score(1);
										//赢的牌型
										item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
										//对方牌型
										item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
									}
								}
								{
									int index = compareCards[loser].results_size() - 1;
									assert(index >= 0);
									//输家比牌结果 ////////////
									s13s::CompareResult* result = compareCards[loser].mutable_results(index);
									//尾墩输赢信息
									s13s::CompareItem* item = result->add_items();
									{
										//输了
										item->set_winlost(-1);
										//输分
										item->set_score(-1);
										//输的牌型
										item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
										//对方牌型
										item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
									}
								}
						}
						//铁支摆尾墩获胜赢4水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty40) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//尾墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(4);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//尾墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-4);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						//同花顺摆尾墩获胜赢5水
						else if (handInfos[winner].GetSelected()->duns[dt].ty_ == S13S::Ty123sc) {
							{
								int index = compareCards[winner].results_size() - 1;
								assert(index >= 0);
								//赢家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[winner].mutable_results(index);
								//尾墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//赢了
									item->set_winlost(1);
									//赢分
									item->set_score(5);
									//赢的牌型
									item->set_ty(handInfos[winner].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[loser].GetSelected()->duns[dt].ty_);
								}
							}
							{
								int index = compareCards[loser].results_size() - 1;
								assert(index >= 0);
								//输家比牌结果 ////////////
								s13s::CompareResult* result = compareCards[loser].mutable_results(index);
								//尾墩输赢信息
								s13s::CompareItem* item = result->add_items();
								{
									//输了
									item->set_winlost(-1);
									//输分
									item->set_score(-5);
									//输的牌型
									item->set_ty(handInfos[loser].GetSelected()->duns[dt].ty_);
									//对方牌型
									item->set_peerty(handInfos[winner].GetSelected()->duns[dt].ty_);
								}
							}
						}
						else {
							{
								assert(false);
							}
						}
					}
				}
				//玩家对其它玩家打枪
				std::map<uint8_t, std::vector<uint8_t>> shootIds;
				//统计判断打枪/全垒打
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//判断是否全垒打
						int shootc = 0;
						//当前比牌玩家
						s13s::PlayerItem const& player = compareCards[i].player();
						//遍历玩家所有比牌对象
						for (int j = 0; j < compareCards[i].peers_size(); ++j) {
							s13s::ComparePlayer const& peer = compareCards[i].peers(j);
							s13s::CompareResult const& result = compareCards[i].results(j);
							int winc = 0, lostc = 0, sumscore = 0;
							//自己特殊牌型或对方特殊牌型
							assert(result.items_size() == (
								player.group().specialty() >= S13S::TyThreesc ||
								peer.group().specialty() >= S13S::TyThreesc) ? 0 : 3);
							//玩家与当前比牌对象比头/中/尾三墩输赢得水总分，不考虑打枪
							for (int d = 0; d < result.items_size(); ++d) {
								if (result.items(d).winlost() == 1) {
									++winc;
								}
								else if (result.items(d).winlost() == -1) {
									++lostc;
								}
								sumscore += result.items(d).score();
							}
							//三墩不考虑打枪输赢得水总分 赢分+/和分0/输分-
							const_cast<s13s::CompareResult&>(result).set_score(sumscore);
							//自己特殊牌型或对方特殊牌型不计算打枪
							if (player.group().specialty() >= S13S::TyThreesc ||
								peer.group().specialty() >= S13S::TyThreesc) {
								const_cast<s13s::CompareResult&>(result).set_shoot(0);//-1被打枪/0不打枪/1打枪
								continue;
							}
							if (winc == result.items_size()) {
								//玩家三墩全部胜过比牌对象，则玩家对比牌对象打枪，中枪者付给打枪者2倍的水
								const_cast<s13s::CompareResult&>(result).set_shoot(1);//-1被打枪/0不打枪/1打枪
								//统计当前玩家打枪次数
								++shootc;
								//玩家对比牌对象打枪
								shootIds[i].push_back(peer.chairid());
							}
							else if (lostc == result.items_size()) {
								//比牌对象三墩全部胜过玩家，则比牌对象对玩家打枪，中枪者付给打枪者2倍的水
								const_cast<s13s::CompareResult&>(result).set_shoot(-1);//-1被打枪/0不打枪/1打枪
							}
							else {
								const_cast<s13s::CompareResult&>(result).set_shoot(0);//-1被打枪/0不打枪/1打枪
							}
						}
						if (shootc == compareCards[i].peers_size() && compareCards[i].peers_size() > 1) {
							//全垒打，玩家三墩全部胜过其它玩家，且至少打2枪，中枪者付给打枪者4倍的水
							compareCards[i].set_allshoot(1);//-1被全垒打/0无全垒打/1全垒打
							//其它比牌对象都是被全垒打
							for (int k = 0; k < GAME_PLAYER; ++k) {
								if (true) {
									if (k != i) {
										//-1被全垒打/0无全垒打/1全垒打
										compareCards[k].set_allshoot(-1);
										//allshoot=-1被全垒打，记下全垒打桌椅ID
										compareCards[k].set_fromchairid(i);
									}
								}
							}
						}
					}
				}
				//其它玩家之间打枪
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//当前比牌玩家
						s13s::PlayerItem const& player = compareCards[i].player();
						//遍历玩家所有比牌对象
						for (int j = 0; j < compareCards[i].peers_size(); ++j) {
							s13s::ComparePlayer const& peer = compareCards[i].peers(j);
							//s13s::CompareResult const& result = compareCards[i].results(j);
							std::map<uint8_t, std::vector<uint8_t>>::const_iterator it = shootIds.find(peer.chairid());
							if (it != shootIds.end()) {
								for (std::vector<uint8_t>::const_iterator ir = it->second.begin();
									ir != it->second.end(); ++ir) {
									//排除当前玩家
									if (*ir != i) {
										assert(player.chairid() == i);
										const_cast<s13s::ComparePlayer&>(peer).add_shootids(*ir);
									}
								}
							}
						}
					}
				}
				//计算包括打枪/全垒打在内输赢得水总分
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//玩家输赢得水总分，不含特殊牌型输赢分
						int deltascore = 0;
						//遍历玩家所有比牌对象
						for (int j = 0; j < compareCards[i].peers_size(); ++j) {
							s13s::ComparePlayer const& peer = compareCards[i].peers(j);
							s13s::CompareResult const& result = compareCards[i].results(j);
							//1打枪(对当前比牌对象打枪)
							if (result.shoot() == 1) {
								//1全垒打
								if (compareCards[i].allshoot() == 1) {
									//-1被全垒打/0无全垒打/1全垒打
									deltascore += 4 * result.score();
								}
								else {
									//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
									if (compareCards[i].allshoot() == -1) {
									}
									else {
									}
									//-1被打枪/0不打枪/1打枪
									deltascore += 2 * result.score();
								}
							}
							//-1被打枪(被当前比牌对象打枪)
							else if(result.shoot() == -1) {
								//-1被全垒打
								if (compareCards[i].allshoot() == -1) {
									//被当前比牌对象全垒打
									if (peer.chairid() == compareCards[i].fromchairid()) {
										//-1被全垒打/0无全垒打/1全垒打
										deltascore += 4 * result.score();
									}
									//被另外比牌对象全垒打
									else {
										//-1被打枪/0不打枪/1打枪
										deltascore += 2 * result.score();
									}
								}
								else {
									//-1被打枪/0不打枪/1打枪
									deltascore += 2 * result.score();
								}
							}
							//0不打枪(与当前比牌对象互不打枪)
							else {
								//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
								if (compareCards[i].allshoot() == -1) {
								}
								else {
									//一定不是全垒打，-1被打枪/0不打枪/1打枪
									assert(compareCards[i].allshoot() == 0);
								}
								assert(result.shoot() == 0);
								deltascore += result.score();
							}
						}
						//玩家输赢得水总分，不含特殊牌型输赢分
						compareCards[i].set_deltascore(deltascore);
					}
				}
				//座椅玩家与其余玩家按墩比输赢分
				//不含打枪/全垒打与打枪/全垒打分开计算
				//计算特殊牌型输赢分
				//统计含打枪/全垒打/特殊牌型输赢得水总分
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						//三墩打枪/全垒打输赢算水
						int allshootscore = 0;
						//当前比牌玩家
						s13s::PlayerItem const& player = compareCards[i].player();
						//遍历各墩(头墩/中墩/尾墩)
						for (int d = S13S::DunFirst; d <= S13S::DunLast; ++d) {
							//按墩比输赢分
							int sumscore = 0;
							//按墩计算打枪/全垒打输赢分
							int sumshootscore = 0;
							//按墩比输赢分，含打枪/全垒打
							{
								//自己特殊牌型，不按墩比牌，输赢水数在specitem中
								if (player.group().specialty() >= S13S::TyThreesc) {
								}
								else {
									//遍历比牌对象
									for (int j = 0; j < compareCards[i].peers_size(); ++j) {
										s13s::ComparePlayer const& peer = compareCards[i].peers(j);
										s13s::CompareResult const& result = compareCards[i].results(j);
										//对方特殊牌型，不按墩比牌，输赢水数在specitem中
										if (peer.group().specialty() >= S13S::TyThreesc) {
											continue;
										}
										//累加指定墩输赢得水积分，不含打枪/全垒打
										sumscore += result.items(d).score();
										//1打枪(对当前比牌对象打枪)
										if (result.shoot() == 1) {
											//1全垒打
											if (compareCards[i].allshoot() == 1) {
												//-1被全垒打/0无全垒打/1全垒打
												sumshootscore += 3/*4*/ * result.items(d).score();
											}
											else {
												//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
												if (compareCards[i].allshoot() == -1) {
												}
												else {
												}
												//-1被打枪/0不打枪/1打枪
												sumshootscore += 1/*2*/ * result.items(d).score();
											}
										}
										//-1被打枪(被当前比牌对象打枪)
										else if (result.shoot() == -1) {
											//-1被全垒打
											if (compareCards[i].allshoot() == -1) {
												//被当前比牌对象全垒打
												if (peer.chairid() == compareCards[i].fromchairid()) {
													//-1被全垒打/0无全垒打/1全垒打
													sumshootscore += 3/*4*/ * result.items(d).score();
												}
												//被另外比牌对象全垒打
												else {
													//-1被打枪/0不打枪/1打枪
													sumshootscore += 1/*2*/ * result.items(d).score();
												}
											}
											else {
												//-1被打枪/0不打枪/1打枪
												sumshootscore += 1/*2*/ * result.items(d).score();
											}
										}
										//0不打枪(与当前比牌对象互不打枪)
										else {
											//-1被全垒打(被另外比牌对象打枪，并且该比牌对象是全垒打)
											if (compareCards[i].allshoot() == -1) {
											}
											else {
												//一定不是全垒打，-1被打枪/0不打枪/1打枪
												assert(compareCards[i].allshoot() == 0);
											}
											assert(result.shoot() == 0);
											//sumshootscore += 0/*1*/ * result.items(d).score();
										}
									}
								}
							}
							//座椅玩家输赢得水积分(头/中/尾/特殊)
							compareCards[i].add_itemscores(sumscore);
							//按墩计算打枪/全垒打输赢分
							compareCards[i].add_itemscorepure(sumshootscore);
							//三墩打枪/全垒打输赢算水
							allshootscore += sumshootscore;
						}
						{
							//特殊牌型输赢算水(无打枪/全垒打)
							int sumspecscore = 0;
							{
								//遍历比牌对象
								for (int j = 0; j < compareCards[i].peers_size(); ++j) {
									s13s::ComparePlayer const& peer = compareCards[i].peers(j);
									s13s::CompareResult const& result = compareCards[i].results(j);
									//自己普通牌型，对方特殊牌型
									//自己特殊牌型，对方普通牌型
									//自己特殊牌型，对方特殊牌型
									if (result.has_specitem()) {
										//累加特殊比牌算水
										sumspecscore += result.specitem().score();
									}
								}
							}
							//座椅玩家输赢得水积分(头/中/尾/特殊)
							compareCards[i].add_itemscores(sumspecscore);
							//三墩打枪/全垒打输赢算水 + 特殊牌型输赢算水(无打枪/全垒打)
							compareCards[i].add_itemscorepure(allshootscore + sumspecscore);
							//玩家输赢得水总分，含打枪/全垒打，含特殊牌型输赢分
							int32_t deltascore = compareCards[i].deltascore();
							compareCards[i].set_deltascore(deltascore + sumspecscore);
						}
					}
				}
				//比牌对方输赢得水总分
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						for (int j = 0; j < compareCards[i].peers_size(); ++j) {
							s13s::ComparePlayer const& peer = compareCards[i].peers(j);
							int32_t deltascore = compareCards[peer.chairid()].deltascore();
							const_cast<s13s::ComparePlayer&>(peer).set_deltascore(deltascore);
						}
					}
				}
				//json格式在线view工具：http://www.bejson.com/jsonviewernew/
				//玩家之间两两比牌输赢得水总分明细，包括打枪/全垒打
				for (int i = 0; i < GAME_PLAYER; ++i) {
					if (true) {
						{
							//序列化std::string
							//std::string data = compareCards[i].SerializeAsString();
							//序列化bytes
							int len = compareCards[i].ByteSizeLong();//len
							uint8_t *data = new uint8_t[len];
							compareCards[i].SerializeToArray(data, len);//data
							std::string const& typeName = compareCards[i].GetTypeName();//typename
							delete[] data;
							//转换json格式
							std::string jsonstr;
							PB2JSON::Pb2Json::PbMsg2JsonStr(compareCards[i], jsonstr, true);
							printf("\n--- *** {\"%s\":\n", typeName.c_str());
							printf("%s}\n\n", jsonstr.c_str());
						}
					}
				}
			}
		} while ('q' != getchar());
	}
#if 0
	//从src中抽取连续n张牌到dst中
	//src uint8_t* 牌源
	//ty FetchTy 抽取类型
	//n int 抽取n张，dst长度
	//dst uint8_t* 符合要求的单个目标牌型
	//cpy uint8_t* 抽取后不符合要求的余牌
	bool CGameLogic::FetchCardsFrom(uint8_t *src, int len, FetchTy Ty, uint8_t *dst, int n, uint8_t *cpy, int& cpylen)
	{
		int i = 0, j = 0, k = 0;
		bool succ = false;
		cpylen = 0;
		switch (Ty)
		{
			//同花色非顺子
		case SameColor:
		{
			if (len <= 0)
				break;
			int s = i++, c = 0;
			uint8_t c_src_pre = GetCardColor(src[s]);
			for (; i < len; ++i) {
				//src中当前的花色
				uint8_t c_src_cur = GetCardColor(src[i]);
				//两张花色相同的牌
				if (c_src_pre == c_src_cur) {
					//收集到n张牌后返回
					if (++c + 1 == n) {
						succ = true;
						break;
					}
				}
				else {
					//缓存不合要求的副本
					memcpy(&cpy[k], &src[s], i - s);
					k += (i - s);
					//赋值新的
					c = 0;
					s = i;
					c_src_pre = GetCardColor(src[s]);
				}
			}
			if (succ) {
				//缓存符合要求的牌型
				assert(n == i - s + 1);
				memcpy(dst, &src[s], n);
				//缓存不合要求的副本
				if (len - (i + 1) > 0) {
					memcpy(&cpy[k], &src[i + 1], len - (i + 1));
					cpylen = k;
					cpylen += len - (i + 1);
				}
				else {
					cpylen = k;
				}
			}
			break;
		}
		//顺子非同花色
		case Consecutive:
		{
			if (len <= 0)
				break;
			dst[j++] = src[i++];
			for (; i < len;) {
				//dst中之前的牌值
				uint8_t v_dst_pre = GetCardValue(dst[j - 1]);
				//src中当前的牌值
				uint8_t v_src_cur = GetCardValue(src[i]);
				//dst中之前的花色
				//uint8_t c_dst_pre = GetCardColor(dst[j - 1]);
				//src中当前的花色
				//uint8_t c_src_cur = GetCardColor(src[i]);
				//dst[j - 1]与src[i]为连续牌♠A [♣2 ♠2]
				if (v_dst_pre + 1 == v_src_cur) {
					//同花色
					//if (c_dst_pre == c_src_cur) {
					dst[j++] = src[i++];
					//收集到n张牌后返回
					if (j == n) {
						succ = true;
						break;
					}
					continue;
					//}
					//花色不同♠A [♣2 ♠2]
					//cpy[k++] = src[i++];
					//continue;
				}
				else if (v_dst_pre == v_src_cur) {
					//连续两张相同牌[♠A ♣2] ♠2
					cpy[k++] = src[i++];
					continue;
				}
				else /*if (v_dst_pre + 1 < v_src_cur)*/ {
					//不连续了
					cpy[k++] = src[i++];
					break;
				}
			}
			//收集到n张牌后拷贝剩余的牌
			if (j == n && i < len) {
				memcpy(&cpy[k], &src[i], len - i);
				k += (len - i);
				cpylen = k;
			}
			break;
		}
		//同花色顺子
		case SameColorConsecutive:
		{
			if (len <= 0)
				break;
			dst[j++] = src[i++];
			for (; i < len;) {
				//dst中之前的牌值
				uint8_t v_dst_pre = GetCardValue(dst[j - 1]);
				//src中当前的牌值
				uint8_t v_src_cur = GetCardValue(src[i]);
				//dst中之前的花色
				uint8_t c_dst_pre = GetCardColor(dst[j - 1]);
				//src中当前的花色
				uint8_t c_src_cur = GetCardColor(src[i]);
				//dst[j - 1]与src[i]为连续牌♠A [♣2 ♠2]
				if (v_dst_pre + 1 == v_src_cur) {
					//同花色
					if (c_dst_pre == c_src_cur) {
						dst[j++] = src[i++];
						//收集到n张牌后返回
						if (j == n) {
							succ = true;
							break;
						}
						continue;
					}
					//花色不同♠A [♣2 ♠2]
					cpy[k++] = src[i++];
					continue;
				}
				else if (v_dst_pre == v_src_cur) {
					//连续两张相同牌[♠A ♣2] ♠2
					cpy[k++] = src[i++];
					continue;
				}
				else /*if (v_dst_pre + 1 < v_src_cur)*/ {
					//不连续了
					cpy[k++] = src[i++];
					break;
				}
			}
			//收集到n张牌后拷贝剩余的牌
			if (j == n && i < len) {
				memcpy(&cpy[k], &src[i], len - i);
				k += (len - i);
				cpylen = k;
			}
			break;
		}
		}//{{switch}}
		return succ;
	}
#endif

};

//////////////////////////////////////////////////////////////////////////////////
