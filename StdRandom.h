#ifndef STD_RANDOM_H
#define STD_RANDOM_H

#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <utility>
#include <random>

#ifndef MAX_WEIGHT
#define MAX_WEIGHT 10
#endif

//#define DEBUG_PRINT

//标准库的伪随机函数
namespace STD {
	//测试用例
	static void test001();
	static void test002(char const* filename);

	//////////////////////////////////////
	//Generator
	class Generator {
	public:
		Generator()
			: mt_({ rd_() })
			, re_({ rd_() }) {
		}
		static Generator& instance() {
			static Generator gen_;
			return gen_;
		}
		std::mt19937& get_mt() {
			return mt_;
		}
		std::default_random_engine& get_re() {
			return re_;
		}
	private:
		std::random_device rd_;
		std::mt19937 mt_;
		std::default_random_engine re_;
	};
	
	//////////////////////////////////////
	//Random
	using RangeInt = std::uniform_int_distribution<>::param_type;
	using RangeFloat = std::uniform_real_distribution<float>::param_type;
	class Random {
	public:
		explicit Random() {}
		//整数范围
		explicit Random(int a, int b) :i_(a, b) {
		}
		//浮点范围
		explicit Random(float a, float b) :f_(a, b) {
		}
	public:
		Random& betweenInt(int a, int b) {
			i_.param(RangeInt{ a, b });
			return *this;
		}
		Random& betweenFloat(float a, float b) {
			i_.param(RangeInt{ a, b });
			return *this;
		}
	public:
		//mt生成器随机整数
		int randInt_mt() {
			return i_(Generator::instance().get_mt());
		}
		//re生成器随机整数
		int randInt_re() {
			return i_(Generator::instance().get_re());
		}
		//mt生成器随机浮点数
		float randFloat_mt() {
			return f_(Generator::instance().get_mt());
		}
		//re生成器随机浮点数
		float randFloat_re() {
			return f_(Generator::instance().get_re());
		}
	private:
		std::uniform_real_distribution<float> f_;
		std::uniform_int_distribution<> i_;
	};

	//////////////////////////////////////
	//Weight
	class Weight {
	public:
		//权重池
		Weight() {
			sum_ = 0;
			len_ = 0;
			memset(indxId_, 0, sizeof(int)*MAX_WEIGHT);
			memset(weight_, 0, sizeof(int)*MAX_WEIGHT);
		}
		//初始化权重集合
		void init(int weight[], int len)
		{
			if (len > MAX_WEIGHT) {
				printf("CWeight::init ERR: len:%d > MAX_WEIGHT:%d\n", len, MAX_WEIGHT);
				return;
			}
			sum_ = 0;
			for (int i = 0; i < len; ++i) {
				weight_[i] = weight[i];
				indxId_[i] = i;
				sum_ += weight[i];
			}
			if (sum_ <= 1) {
				return;
			}
			len_ = len;
			//随机数范围
			rand_.betweenInt(1, sum_);
		}
		//权重随机重排
		void shuffle() {
			for (int i = len_ - 1; i > 0; --i) {
				std::uniform_int_distribution<decltype(i)> d(0, i);
				int j = d(STD::Generator::instance().get_mt());
				std::swap(weight_[i], weight_[j]);
				std::swap(indxId_[i], indxId_[j]);
			}
		}
		//按权值来随机，返回索引
		int getResult() {
			if (sum_ <= 1) {
				return indxId_[0];
			}
#ifdef DEBUG_PRINT
			for (int i = 0; i < len_; ++i) {
				printf("w[%d]=%d\n", indxId_[i], weight_[i]);
			}
#endif
			int r = rand_.randInt_mt(), c = r;
			for (int i = 0; i < len_; ++i) {
				c -= weight_[i];
				if (c <= 0) {
#ifdef DEBUG_PRINT
					printf("sum=%d r=%d i=%d\n", sum_, r, indxId_[i]);
					printf("-------------------------\n\n\n");
#endif
					return indxId_[i];
				}
			}
		}
	public:
		STD::Random rand_;		//随机数值
		int sum_;				//权值之和
		int len_;				//统计个数
		int weight_[MAX_WEIGHT];//权重集合
		int indxId_[MAX_WEIGHT];//对应索引
	};

	//测试用例
	static void test001() {

		//随机数[5,10]
		STD::Random r1(5, 10);
		r1.randInt_mt();

		//随机数[5,10]
		STD::Random r2;
		r2.betweenInt(5, 10).randInt_mt();

		//随机浮点数[0.1,0.9]
		STD::Random r3(0.1f, 0.9f);
		r3.randFloat_mt();

		//随机浮点数[0.1,0.9]
		STD::Random r4;
		r4.betweenFloat(0.1f, 0.9f).randFloat_mt();

		//概率分别为30, 20, 50
		int weight[3] = { 30,20,50 };
		STD::Weight w;
		//初始化
		w.init(weight, 3);
		//随机10次
		for (int i = 0; i < 10; ++i) {
			//权值随机重排，可以不调用
			w.shuffle();
			//返回随机索引
			w.getResult();
		}
	}

	//测试按权重随机概率结果
	//写入文件再导入Excel并插入图表查看概率正态分布情况
	//filename char const* 要写入的文件 如/home/testweight.txt
	static void test002(char const* filename) {
		while (1) {
			if ('q' == getchar()) {
				break;
			}
			remove(filename);
			FILE* fp = fopen(filename, "a");
			if (fp == NULL) {
				return;
			}
			int c = 1000;			//循环总次数
			int scale = 10;			//放大倍数
			int ratioExC = 25;		//换牌概率
			int exC = 0, noExC = 0; //换牌/不换牌分别统计次数
			//概率分别为ratioExC, 100 - ratioExC
			int weight[2] = { ratioExC*scale,(100 - ratioExC)*scale };
			STD::Weight w;
			//初始化
			w.init(weight, 2);
			//随机c次
			for (int i = 0; i < c; ++i) {
				//权值随机重排，可以不调用
				w.shuffle();
				//返回随机索引
				int index = w.getResult();
				if (index == 0) {
					++exC;
				}
				else if (index == 1) {
					++noExC;
				}
				//写入文件再导入Excel并插入图表查看概率正态分布情况
				char ch[10] = { 0 };
				sprintf(ch, "%d\t", index == 0 ? 1 : -1);
				fwrite(ch, strlen(ch), 1, fp);
			}
			fflush(fp);
			fclose(fp);
			printf("c:%d:%d scale:%d ratioExC:%d exC:%d:ratio:%.02f\n",
				c, exC + noExC, scale, ratioExC,
				exC, ((float)exC) / (float)(exC + noExC));
		}
	}
}

#endif
