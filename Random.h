/*
	CRandom::Instance().Random_Int(0,100);
	CRandom::Instance().Random_Float(0,1.0f);
*/

#ifndef __RANDOM_HEADER__
#define __RANDOM_HEADER__

#include <math.h>
#include <sys/time.h>

class CRandom
{
public:
    CRandom()
    {
        timeval tv;
        gettimeofday(&tv,NULL);
        m_nSeed=(((tv.tv_sec*1000)+(tv.tv_usec/1000)));
        Random_Seed(m_nSeed);
    }

public:
    static CRandom& Instance()
    {
        static CRandom* sp = 0;
        if(!sp)
        {
             sp = new CRandom();
        }
//Cleanup:
        return (*sp);
    }

public:
    void Random_Seed(unsigned int seed=0)
    {
        timeval tv;
        if(!seed)
        {
            gettimeofday(&tv,NULL);
            m_nSeed=(((tv.tv_sec*1000)+(tv.tv_usec/1000)));
        }else
        {
            m_nSeed=seed;
        }
    }
    
    int Random_Int(int min, int max)
    {
        GetNextSeed();
        return min+(m_nSeed ^ m_nSeed>>15)%(max-min+1);
    }

    float Random_Float(float min, float max)
    {
        GetNextSeed();
        //return min+m_nSeed*(1.0f/4294967295.0f)*(max-min);
        return min+(m_nSeed>>16)*(1.0f/65535.0f)*(max-min);
    }

    int Range(int min, int max)
    {
            GetNextSeed();
            return min + (m_nSeed ^ m_nSeed >> 15) % (max - min + 1);
    }

    float Range(float min, float max)
    {
            GetNextSeed();
            return min + (m_nSeed >> 16) * (1.0f / 65535.0f) * (max - min);
    }

    unsigned int GetNextSeed()
    {
            return m_nSeed = 214013 * m_nSeed + 2531011;
    }

    unsigned int GetCurrentSeed()
    {
            return m_nSeed;
    }

protected:
    unsigned int m_nSeed;
};

#define Random (*CRandom::Instance())
#endif//__RANDOM_HEADER__
