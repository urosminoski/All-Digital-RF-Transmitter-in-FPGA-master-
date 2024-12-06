#include "deltaSigma.hpp"

void deltaSigma(std::vector<double>& x, std::vector<double>& y)
{
    y.clear();  // Make sure output vector is empty

    // H0 Coeffitients
    ac_fixed<12,8,true,AC_RND_CONV,AC_SAT> b00 = 7.3765809;
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> a01 = 0.3466036;

    // H1 Coeffitients
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> b10 = 0.424071040;
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> a11 = 0.66591402;
    ac_fixed<12,9,true,AC_RND_CONV,AC_SAT> b11 = 2.782608716;
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> a12 = 0.16260264;

    // H2 Coeffitients
    ac_fixed<12,8,true,AC_RND_CONV,AC_SAT> b20 = 4.606822182;
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> a21 = 0.62380242;
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> b21 = 0.023331537;
    ac_fixed<12,11,true,AC_RND_CONV,AC_SAT> a22 = 0.4509869;

    typedef ac_fixed<24,19,true,AC_RND_CONV,AC_SAT> fx_ss;

    fx_ss y_iir = 0, e = 0, y_i = 0;
    fx_ss x0 = 0, x0d = 0;
    fx_ss x1 = 0, w1 = 0, w1d = 0, w1dd = 0;
    fx_ss x2 = 0, w2 = 0, w2d = 0, w2dd = 0;
    ac_fixed<4,0,true,AC_RND_CONV,AC_SAT> v = 0;
    ac_fixed<12,8,true,AC_RND_CONV,AC_SAT> xin = 0;

    for(size_t i = 0; i < x.size(); i++)
    {
        xin = x[i];
        v = x[i];
        y.push_back(v.to_double());
        // std::cout << y << '\n' << std::endl;
    }
}