//
// Created by andy_ro@qq.com
// 			11/19/2021
//
#ifndef GAME_LOGIC_SUOHA_H
#define GAME_LOGIC_SUOHA_H

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

#define MAX_CARD_TOTAL	SUOHA::MaxCardTotal	//牌总个数
#define GAME_PLAYER		SUOHA::MaxPlayer	//最多5人局
#define MIN_GAME_PLAYER SUOHA::MinPlayer	//至少2人局
#define MAX_COUNT		SUOHA::MaxCount		//每人5张牌
#define MAX_ROUND       SUOHA::MaxRound		//最大局数

//梭哈
namespace SUOHA {
	const int MaxCardTotal = 52;	//牌总个数，除去大小王，52张牌
	const int MaxPlayer = 5;		//最多5人局
	const int MinPlayer = 2;		//至少2人局
	const int MaxCount = 5;			//每人5张牌
	const int MaxRound = 4;			//最多4轮

	//牌值：A<2<3<4<5<6<7<8<9<10<J<Q<K
	//点数：2<3<4<5<6<7<8<9<10<J<Q<K<A
	//花色：黑>红>梅>方
	//
	//普通牌型
	//同花顺>四条>葫芦>同花>顺子>三条>两对>对子>散牌
	//
	//手牌类型：从小到大
	enum HandTy {
		//TyNil		  = 0,
		Tysp          = 0,	//散牌：一墩牌不组成任何牌型
		Ty20          = 1,	//对子(一对)：除了两张值相同的牌外没有其它牌型
		Ty22          = 2,	//两对：两个对子加上一张单牌
		Ty30          = 3,	//三条：除了三张值相同的牌外没有其它牌型
		Ty123         = 4,	//顺子：花色不同的连续五张牌(A2345仅小于10JQKA)
		Tysc          = 5,	//同花：花色相同的五张牌，非顺子
		Ty32          = 6,	//葫芦：一组三条加上一组对子
		Ty40          = 7,	//四条(铁支)：除了四张值相同的牌外没有其它牌型
		Ty123sc       = 8,	//同花顺：花色相同的连续五张牌(A2345仅小于10JQKA)
		Ty123scRoyal  = 9,	//皇家同花顺：同一种花色最大的顺子(10JQKA)
		TyAll,
	};

	//花色：黑>红>梅>方
	enum CardColor {
		Diamond = 0x10,	//方块(♦)
		Club	= 0x20,	//梅花(♣)
		Heart	= 0x30,	//红心(♥)
		Spade	= 0x40,	//黑桃(♠)
	};

	//牌值：A<2<3<4<5<6<7<8<9<10<J<Q<K
	enum CardValue {
		A = 0x01,
		T = 0x0A,
		J = 0x0B,
		Q = 0x0C,
		K = 0x0D,
	};

	//手牌占位条码mark大小 14
	//牌值对应占位->A,2,,,K,0 牌点对应占位->0,2,,,K,A
	int const MaxSZ = K + 1;

	//游戏逻辑类
	class CGameLogic
	{
	public:
		CGameLogic();
		virtual ~CGameLogic();
	public:
		//初始化扑克牌数据
		void InitCards();
		//debug打印
		void DebugListCards();
		//剩余牌数
		int8_t Remaining();
		//洗牌
		void ShuffleCards();
		//发牌，生成n张玩家手牌
		void DealCards(int8_t n, uint8_t *cards);
	public:
		//花色：黑>红>梅>方
		static uint8_t GetCardColor(uint8_t card);
		//牌值：A<2<3<4<5<6<7<8<9<10<J<Q<K
		static uint8_t GetCardValue(uint8_t card);
		//点数：2<3<4<5<6<7<8<9<10<J<Q<K<A
		static uint8_t GetCardPoint(uint8_t card);
		//花色和牌值构造单牌
		static uint8_t MakeCardWith(uint8_t color, uint8_t value);
		//手牌排序(默认按牌点降序排列)，先比牌值/点数，再比花色
		//byValue bool false->按牌点 true->按牌值
		//ascend bool false->降序排列(即从大到小排列) true->升序排列(即从小到大排列)
		//clrAscend bool false->花色降序(即黑桃到方块) true->花色升序(即从方块到黑桃)
		static void SortCards(uint8_t *cards, int n, bool byValue, bool ascend, bool clrAscend);
		//手牌排序(默认按牌点降序排列)，先比花色，再比牌值/点数
		//clrAscend bool false->花色降序(即黑桃到方块) true->花色升序(即从方块到黑桃)
		//byValue bool false->按牌点 true->按牌值
		//ascend bool false->降序排列(即从大到小排列) true->升序排列(即从小到大排列)
		static void SortCardsColor(uint8_t *cards, int n, bool clrAscend, bool byValue, bool ascend);
		//牌值字符串 
		static std::string StringCardValue(uint8_t value);
		//花色字符串 
		static std::string StringCardColor(uint8_t color);
		//单牌字符串
		static std::string StringCard(uint8_t card);
		//牌型字符串
		static std::string StringHandTy(HandTy ty);
		//手牌字符串
		static std::string StringCards(uint8_t const* cards, int n);
		//打印n张牌
		static void PrintCardList(uint8_t const* cards, int n, bool hide = true);
		//获取牌有效列数
		//cards uint8_t const* 相同牌值n张牌(n<=4)
		//n uint8_t 黑/红/梅/方4张牌
		static uint8_t get_card_c(uint8_t const* cards, int n);
		//返回指定花色牌列号
		//cards uint8_t const* 相同牌值n张牌(n<=4)
		//n uint8_t 黑/红/梅/方4张牌
		//clr CardColor 指定花色
		static uint8_t get_card_colorcol(uint8_t const* cards, int n, CardColor clr);
	private:
		//拆分字符串"♦A ♦3 ♥3 ♥4 ♦5 ♣5 ♥5 ♥6 ♣7 ♥7 ♣9 ♣10 ♣J"
		static void CardsBy(std::string const& strcards, std::vector<std::string>& vec);
		//字串构造牌"♦A"->0x11
		static uint8_t MakeCardBy(std::string const& name);
		//生成n张牌<-"♦A ♦3 ♥3 ♥4 ♦5 ♣5 ♥5 ♥6 ♣7 ♥7 ♣9 ♣10 ♣J"
		static void MakeCardList(std::vector<std::string> const& vec, uint8_t *cards, int size);
	public:
		//生成n张牌<-"♦A ♦3 ♥3 ♥4 ♦5 ♣5 ♥5 ♥6 ♣7 ♥7 ♣9 ♣10 ♣J"
		static int MakeCardList(std::string const& strcards, uint8_t *cards, int size);
		//手牌点数最大牌
		static uint8_t MaxCard(uint8_t const* cards, size_t size);
		static uint8_t MaxCard(std::vector<uint8_t> const& cards);
		//手牌点数最小牌
		static uint8_t MinCard(uint8_t const* cards, size_t size);
		static uint8_t MinCard(std::vector<uint8_t> const& cards);
	private:
		int8_t index_;
		uint8_t cardsData_[MaxCardTotal];
	public:
		//////////////////////////////////////////////////////////////
		//EnumTree 枚举一墩牌型的所有可能，多叉树结构
		class EnumTree {
		public:
			//枚举牌，枚举一墩牌，5/3张
			typedef std::vector<uint8_t>                CardData;
			//枚举项，pair<牌型，一墩牌>
			typedef std::pair<HandTy, CardData const*>  EnumItem;
			//树节点，pair<枚举项，子枚举项列表>
			typedef std::pair<EnumItem, EnumTree*>      TreeNode;
			//树节点，pair<树节点指针，对应树枚举项>
			typedef std::pair<EnumTree*, int>   TraverseTreeNode;
		public:
			EnumTree() {
				Reset();
			}
			~EnumTree() {
				Reset();
			}
			void Reset();
			//打印指定枚举牌型
			void PrintEnumCards(bool reverse/* = false*/, HandTy ty/* = TyAll*/);
			//打印指定枚举牌型
			void PrintEnumCards(std::string const& name, HandTy ty, std::vector<std::vector<uint8_t>> const& src, bool reverse);
		public:
			//所有同花色五张/三张连续牌(五张/三张同花顺)
			std::vector<CardData> v123sc;
			//所有铁支(四张)
			std::vector<CardData> v40;
			//所有葫芦(一组三条加上一组对子)
			std::vector<CardData> v32;
			//所有同花五张/三张非连续牌(五张/三张同花)
			std::vector<CardData> vsc;
			//所有非同花五张/三张连续牌(五张/三张顺子)
			std::vector<CardData> v123;
			//所有三条(三张)
			std::vector<CardData> v30;
			//所有两对(两个对子)
			std::vector<CardData> v22;
			//所有对子(一对)
			std::vector<CardData> v20;
		};
	public:
		//////////////////////////////////////////////////////////////
		//classify_t 分类牌型
		struct classify_t {
			void copy(classify_t const& ref) {
				c4 = ref.c4;
				c3 = ref.c3;
				c2 = ref.c2;
				cpylen = ref.cpylen;
				memcpy(dst4, ref.dst4, sizeof(dst4));
				memcpy(dst3, ref.dst3, sizeof(dst3));
				memcpy(dst2, ref.dst2, sizeof(dst2));
				memcpy(cpy, ref.cpy, sizeof(cpy));
			}
			int c4, c3, c2;
			//所有重复四张牌型
			static int const size4 = 3;
			uint8_t dst4[size4][4];
			//所有重复三张牌型
			static int const size3 = 4;
			uint8_t dst3[size3][4];
			//所有重复二张牌型
			static int const size2 = 6;
			uint8_t dst2[size2][4];
			//去重后的余牌/散牌
			uint8_t cpy[MaxSZ];
			int cpylen;
			void PrintCardList();
		};
		//////////////////////////////////////////////////////////////
		//handinfo_t 一副手牌信息
		class handinfo_t {
			friend class CGameLogic;
		public:
			handinfo_t() {
				Reset();
			}
			void Reset() {
				ty_ = Tysp;
				cards_.clear();
			}
			handinfo_t(handinfo_t const& ref) {
				Reset();
				ty_ = ref.ty_;
				std::copy(ref.cards_.begin(), ref.cards_.end(), std::back_inserter(cards_));
			}
			handinfo_t& operator=(handinfo_t const& ref) {
				Reset();
				ty_ = ref.ty_;
				std::copy(ref.cards_.begin(), ref.cards_.end(), std::back_inserter(cards_));
				return *this;
			}
		public:
			//牌型
			HandTy ty_;
			//构成牌型的牌
			std::vector<uint8_t> cards_;
			uint32_t chairID;
		};
	public:
		//枚举牌型测试
		static void TestEnumCards(int size);
		//枚举牌型测试
		//filename char const* 文件读取手牌 cardsList.ini
		static void TestEnumCards(char const* filename);
	public:
		//牌型判断，算法入口 /////////
		//src uint8_t const* 牌源
		//hand handinfo_t& 保存手牌信息
		static HandTy AnalyseCards(uint8_t const* src, int len, handinfo_t& hand, bool bv = false);
		//牌比大小
		static int CompareCards(uint8_t* src, uint8_t* dst, int len);
		static int CompareCards(handinfo_t const& src, handinfo_t const& dst);
		//按照尾墩5张/中墩5张/头墩3张依次抽取枚举普通牌型
		//src uint8_t const* 手牌余牌(13/8/3)，初始13张，按5/5/3依次抽，余牌依次为13/8/3
		//n int 抽取n张(5/5/3) 第一次抽5张余8张，第二次抽5张余3张，第三次取余下3张抽完
		//classify classify_t& 存放分类信息(所有重复四张/三张/二张/散牌/余牌)
		//enumList EnumTree& 存放枚举墩牌型列表数据 dt DunTy 指定为第几墩
		static void EnumCards(uint8_t const* src, int len,
			int n, classify_t& classify, EnumTree& enumList);
	public:
		static int CompareCards(
			uint8_t const* src, int srcLen,
			uint8_t const* dst, int dstLen, HandTy ty);
		static int CompareCardPointBy(
			uint8_t const* src, int srcLen,
			uint8_t const* dst, int dstLen);
	private:
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
		static void EnumCards(uint8_t const* src, int len, int n,
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
			std::vector<std::vector<uint8_t>>& v20);
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
		static void ClassifyCards(uint8_t const* src, int len, int n,
			uint8_t(**const pdst)[4],
			uint8_t(*dst4)[4], int& c4,
			uint8_t(*dst3)[4], int& c3,
			uint8_t(*dst2)[4], int& c2,
			uint8_t *cpy, int& cpylen,
			std::vector<std::vector<uint8_t>>& dst0,
			std::vector<std::vector<uint8_t>>& dst1,
			std::vector<std::vector<uint8_t>>& dstc);
		//返回去重后的余牌/散牌
		//src uint8_t const* 牌源
		//pdst uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//dst4 uint8_t(*)[4] 存放所有四张牌型，c4 四张牌型数
		//dst3 uint8_t(*)[4] 存放所有三张牌型，c3 三张牌型数
		//dst2 uint8_t(*)[4] 存放所有对子牌型，c2 对子牌型数
		//cpy uint8_t* 去重后的余牌/散牌(除去四张/三张/对子)///////
		static int RemoveRepeatCards(uint8_t const* src, int len,
			uint8_t(**const pdst)[4],
			uint8_t(*dst4)[4], int& c4, uint8_t(*dst3)[4], int& c3,
			uint8_t(*dst2)[4], int& c2, uint8_t *cpy, int& cpylen);
		//枚举所有不区分花色n张连续牌型(同花顺/顺子)，先去重再补位，最后遍历查找
		//去重后的余牌/散牌与单张组合牌补位合并//////
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//psrc uint8_t(**const)[4] 从所有四张/三张/对子中各取一张合成单张组合牌
		//cpy uint8_t const* 去重后的余牌/散牌(除去四张/三张/对子)牌源///////
		//n int 抽取n张(3/5/13)
		//ctx std::vector<std::vector<short>>& 标记psrc的ctx信息
		//dst std::vector<std::vector<uint8_t>>& 存放所有连续牌型
		//clr std::vector<bool>& 对应dst是否同花
		static void EnumConsecCards(
			uint8_t(**const psrc)[4], int const psrclen,
			uint8_t const* cpy, int const cpylen, int n,
			std::vector<std::vector<short>>& ctx,
			std::vector<std::vector<uint8_t>>& dst,
			std::vector<bool>& clr);
		//枚举所有n张相同花色不连续牌型(同花)，先去重再补位，最后遍历查找
		//去重后的余牌/散牌与单张组合牌补位合并//////
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//psrc uint8_t(**const)[4] 从所有四张/三张/对子中各取一张合成单张组合牌
		//cpy uint8_t const* 去重后的余牌/散牌(除去四张/三张/对子)牌源///////
		//n int 抽取n张(3/5/13)
		//ctx std::vector<std::vector<short>>& 标记psrc的ctx信息
		//dst std::vector<std::vector<uint8_t>>& 存放所有同花牌型
		static void EnumSameColorCards(
			uint8_t(**const psrc)[4], int const psrclen,
			uint8_t const* cpy, int const cpylen, int n,
			std::vector<std::vector<short>>& ctx,
			std::vector<std::vector<uint8_t>>& dst);
		//枚举所有同花顺/顺子(区分花色五张连续牌)
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//psrc uint8_t(**const)[4] 从所有四张/三张/对子中各取一张合成单张组合牌
		//ctx std::vector<std::vector<short>>& 标记psrc的ctx信息
		//src std::vector<std::vector<uint8_t>> const& 所有单张组合牌源///////
		//clr std::vector<bool> const& 对应src是否同花
		//dst1 std::vector<std::vector<uint8_t>>& 存放所有同花顺
		//dst2 std::vector<std::vector<uint8_t>>& 存放所有顺子
		static void EnumConsecCardsByColor(
			uint8_t(**const psrc)[4], int const psrclen,
			std::vector<std::vector<short>> const& ctx,
			std::vector<std::vector<uint8_t>> const& src,
			std::vector<bool> const& clr,
			std::vector<std::vector<uint8_t>>& dst1,
			std::vector<std::vector<uint8_t>>& dst2);
		//枚举所有葫芦(一组三条加上一组对子)
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
		//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
		//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
		//dst std::vector<std::vector<uint8_t>>& 存放所有葫芦牌型
		static int EnumCards32(
			uint8_t(**const psrc)[4],
			uint8_t(*const src4)[4], int const c4,
			uint8_t(*const src3)[4], int const c3,
			uint8_t(*const src2)[4], int const c2,
			std::vector<std::vector<uint8_t>>& dst);
		//枚举所有三条(三张值相同的牌)
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
		//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
		//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
		//dst std::vector<std::vector<uint8_t>>& 存放所有三条牌型
		static int EnumCards30(
			uint8_t(**const psrc)[4],
			uint8_t(*const src4)[4], int const c4,
			uint8_t(*const src3)[4], int const c3,
			uint8_t(*const src2)[4], int const c2,
			std::vector<std::vector<uint8_t>>& dst);
		//枚举所有两对(两个对子加上一张单牌)
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
		//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
		//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
		//dst std::vector<std::vector<uint8_t>>& 存放所有两对牌型
		static int EnumCards22(
			uint8_t(**const psrc)[4],
			uint8_t(*const src4)[4], int const c4,
			uint8_t(*const src3)[4], int const c3,
			uint8_t(*const src2)[4], int const c2,
			std::vector<std::vector<uint8_t>>& dst);
		//枚举所有对子(一对)
		//psrc uint8_t(**const)[4] 衔接dst4/dst3/dst2
		//src4 uint8_t(*const)[4] 所有四张牌型牌源，c4 四张牌型数
		//src3 uint8_t(*const)[4] 所有三张牌型牌源，c3 三张牌型数
		//src2 uint8_t(*const)[4] 所有对子牌型牌源，c2 对子牌型数
		//dst std::vector<std::vector<uint8_t>>& 存放所有对子牌型
		static int EnumCards20(
			uint8_t(**const psrc)[4],
			uint8_t(*const src4)[4], int const c4,
			uint8_t(*const src3)[4], int const c3,
			uint8_t(*const src2)[4], int const c2,
			std::vector<std::vector<uint8_t>>& dst);
	private:
		//枚举所有指定重复牌型(四张/三张/二张)
		//src uint8_t const* 牌源
		//n int 抽取n张(4/3/2)
		//dst uint8_t(*)[4] 存放指定重复牌型
		//cpy uint8_t* 抽取后不符合要求的余牌
		static int EnumRepeatCards(uint8_t const* src, int len, int n,
			uint8_t(*dst)[4], int size, uint8_t *cpy, int& cpylen);
		//填充卡牌条码标记位
		//src uint8_t* 用A2345678910JQK来占位 size = MaxSZ
		//dst uint8_t const* 由单张组成的余牌或枚举组合牌
		static void FillCardsMarkLoc(uint8_t *src, int size, uint8_t const* dst, int dstlen, bool reset);
		//填充卡牌条码标记位
		//src uint8_t* 用2345678910JQKA来占位 size = MaxSZ
		//dst uint8_t const* 由单张组成的余牌或枚举组合牌
		static void FillCardsMarkLocByPoint(uint8_t *src, int size, uint8_t const* dst, int dstlen, bool reset);
		//从补位合并后的单张组合牌中枚举所有不区分花色n张连续牌型///////
		//src uint8_t const* 单张组合牌源，去重后的余牌/散牌与从psrc每组中各抽一张牌补位合并
		//n int 抽取n张(3/5/13)
		//start int const 检索src/mark的起始下标
		//psrcctx short const* 标记psrc的ctx信息
		//dst std::vector<std::vector<uint8_t>>& 存放所有连续牌型(不区分花色)
		//clr std::vector<bool>& 对应dst是否同花
		static int EnumConsecCardsMarkLoc(uint8_t const* src, int len, int n,
			int const start, short const* psrcctx,
			std::vector<std::vector<short>>& ctx,
			std::vector<std::vector<uint8_t>>& dst,
			std::vector<bool>& clr);
		//从补位合并后的单张组合牌中枚举所有n张指定花色牌型///////
		//src uint8_t const* 单张组合牌源，去重后的余牌/散牌与从psrc每组中各抽一张牌补位合并
		//n int 抽取n张(3/5/13)
		//clr CardColor 指定花色
		//psrcctx short const* 标记psrc的ctx信息
		//dst std::vector<std::vector<uint8_t>>& 存放所有同花牌型(非顺子)
		//consec bool false->不保留同花顺 true->保留同花顺 
		//dst2 std::vector<std::vector<uint8_t>>& 存放所有同花顺牌型
		static int EnumSameColorCardsMarkLoc(uint8_t const* src, int len, int n, CardColor clr,
			short const* psrcctx,
			std::vector<std::vector<short>>& ctx,
			std::vector<std::vector<uint8_t>>& dst,
			bool consec,
			std::vector<std::vector<uint8_t>>& dst2);
	private:
		//求组合C(n,1)*C(n,1)...*C(n,1)
		//f(k)=C(n,1)
		//Multi(k)=f(1)*f(2)...*f(k)
		//n int 访问广度
		//k int 访问深度
		//深度优先遍历，由浅到深，广度遍历，由里向外
		static int DepthVisit(int n,
			int k,
			uint8_t(*const*const psrc)[4],
			int const* colc,
			std::vector<uint8_t> const& vec,
			std::vector<std::vector<uint8_t>>& dst0,
			std::vector<std::vector<uint8_t>>& dst1);
		//递归求组合C(n,1)*C(n,1)...*C(n,1)
		//f(k)=C(n,1)
		//Multi(k)=f(1)*f(2)...*f(k)
		//n int 访问广度
		//k int 访问深度
		//深度优先遍历，由浅到深，广度遍历，由里向外
		static int DepthC(int n,
			int k, int *e, int& c,
			uint8_t(*const*const psrc)[4],
			int const* colc,
			std::vector<uint8_t> const& vec,
			std::vector<std::vector<uint8_t>>& dst0,
			std::vector<std::vector<uint8_t>>& dst1);
		//求组合C(n,k)
		//n int 访问广度
		//k int 访问深度
		//深度优先遍历，由浅到深，广度遍历，由里向外
		static int FuncC(int n, int k,
			uint8_t(*const*const psrc)[4], int const r,
			std::vector<std::vector<uint8_t>>& dst);
		//递归求组合C(n,k)
		//n int 访问广度
		//k int 访问深度
		//深度优先遍历，由浅到深，广度遍历，由里向外
		static int FuncC(int n, int k, int *e, int& c,
			uint8_t(*const*const psrc)[4], int const r,
			std::vector<std::vector<uint8_t>>& dst);
	};
};

#endif