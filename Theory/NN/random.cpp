/*=============================================================================================================
                                                random.cpp
===============================================================================================================

 Routines for the generation of pseudo-random numbers
 
 C++-code accompanying:
		 
		(ms. in prep).

 Written by:
        G. Sander van Doorn
       	Centre for Ecological and Evolutionary Studies - Theoretical Biology Group
        University of Groningen
        the Netherlands
 
 Modified by:
        Vivek Hari Sridhar
        Department of Collective Behaviour
        Max Planck Institute of Ornithology
        Germany

 Program version
		xx/xx/xxxx	:

=============================================================================================================*/

#include <chrono>
#include <sstream>
#include <iomanip>
#include "random.h"

namespace rnd {
    std::mt19937 rng;
	const double BINOMIALCOST = 7.0;	//time cost of drawing a binomial() deviate relative to drawing a uniform() deviate

    void set_seed(const unsigned &seed)
    {
        std::ostringstream oss;
        oss << "random seed set to " << seed;
        echo(oss.str());
        rng.seed(seed);
    }
    
	void set_seed()
    {
        unsigned seed = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        set_seed(seed);
    }
	
    int integer(const int &n)
    {
        typedef std::uniform_int_distribution<int> distribution;
        static int k = n;
        static distribution distr(0, k - 1);
        if (k != n) {
            k = n;
            distr = distribution(0, k - 1);
        }
		return distr(rng);
    }
    
	bool bernoulli(const double &p)
    {
        typedef std::bernoulli_distribution distribution;
        static double z = p;
        static distribution distr(z);
        if(p > 0.0 && p < 1.0) {
            if (z != p) {
                z = p;
                distr = distribution(z);
            }
        }
        else if(p == 0.0) return false;
        else if(p == 1.0) return true;
        else error("argument out of range", CURRENT_FUNCTION);
        return distr(rng);
	}

	int binomial(const int &n, const double &p)
    {
        typedef std::binomial_distribution<int> distribution;
        static int k = n;
        static double z = p;
        static distribution distr(k, z);
        if(p > 0.0 && p < 1.0) {
            if (k != n || z != p) {
                k = n;
                z = p;
                distr = distribution(k, z);
            }
        }
        else if(p == 0.0) return 0;
        else if(p == 1.0) return n;
        else error("argument out of range", CURRENT_FUNCTION);
        return distr(rng);
	}

	int poisson(const double &r)
	{
        typedef std::poisson_distribution<int> distribution;
        static double z = r;
        static distribution distr(z);
        if(r > 0.0) {
            if (z != r) {
                z = r;
                distr = distribution(z);
            }
        }
        else if (r == 0.0) return 0;
        else error("argument out of range", CURRENT_FUNCTION);
        return distr(rng);
	}

	double uniform()
	{
        static std::uniform_real_distribution<double> distr(0.0, 1.0);
        return distr(rng);
	}
    
    double normal(const double &mean, const double &stdev)
	{
        typedef std::normal_distribution<double> distribution;
        static double z1 = mean, z2 = stdev;
        static distribution distr(z1, z2);
        if(stdev > 0.0) {
            if (z1 != mean || z2 != stdev) {
                z1 = mean;
                z2 = stdev;
                distr = distribution(z1, z2);
            }
        }
        else if (stdev == 0.0) return mean;
        else error("argument out of range", CURRENT_FUNCTION);
        return distr(rng);
	}
    
    double gamma(const double &shape, const double &scale)
    {
        typedef std::gamma_distribution<double> distribution;
        static double z1 = shape, z2 = scale;
        static distribution distr (z1, z2);
        if (shape > 0.0 && scale > 0.0) {
            if (z1 != shape || z2 != scale) {
                z1 = shape;
                z2 = scale;
                distr = distribution(z1, z2);
            }
        }
        else error("argument out of range", CURRENT_FUNCTION);
        return distr(rng);
    }
    
    double exponential(const double &r)
	{
        typedef std::exponential_distribution<double> distribution;
        static double z = r;
        static distribution distr(z);
        if(r > 0.0) {
            if (z != r) {
            z = r;
            distr = distribution(z);
            }
        }
        else if (r == 0.0) return 0.0;
        else error("argument out of range", CURRENT_FUNCTION);
        return distr(rng);
	}
    
    discrete_distribution::discrete_distribution(const int &sz) :
    n(sz), pdf(std::vector<double>(sz, 0.0)), cdf(std::vector<double>(sz))
    {
        is_accumulated = false;
    }
    
    void discrete_distribution::accumulate()
    {
        double sum = 0.0;
		for (std::vector<double>::iterator itp = pdf.begin(), itc = cdf.begin(); itp != pdf.end(); ++itp, ++itc)
            *itc = sum += *itp;
        is_accumulated = true;
    }
    
	int discrete_distribution::sample()
	//samples from the cdf using bisection
    {
        if (!is_accumulated) accumulate();
		int jmin = -1, jmax = n - 1;
        const double f = uniform() * cdf.back();
		while(jmax - jmin > 1)
        {
            const int jmid = (jmax + jmin) / 2;
            if(f > cdf[jmid]) jmin = jmid;
            else jmax = jmid;
        }
        return jmax;
    }
    
    std::vector<int> discrete_distribution::sample(int k)
	//samples k times with replacement
	{
        if (!is_accumulated) accumulate();
        std::vector<double> pdf_sav(pdf);
        std::vector<int> out(n, 0);
        
        //binomial sampling
        bool restore_pdf = false;
        double rsum = cdf.back();
		for (int i = 0; k > 0 && i < n; ++i) {
			double pi  = pdf[i] / rsum;
            clip_range(pi, 0.0, 1.0);
			if (pi * k > BINOMIALCOST) {
                k -= out[i] = binomial(k, pi);
				rsum -= pdf[i];
				pdf[i] = 0.0;
                restore_pdf = true;
			}
		}
        
        bool restore_cdf = false;
        if(k && restore_pdf) {
            accumulate();
            restore_cdf = true;
        }
        
        //direct sampling
        while (k) {
            ++out[sample()];
            --k;
        }
        
        if (restore_pdf) {
            pdf = pdf_sav;
            if (restore_cdf) is_accumulated = false;
        }
        return out;
	}
}
