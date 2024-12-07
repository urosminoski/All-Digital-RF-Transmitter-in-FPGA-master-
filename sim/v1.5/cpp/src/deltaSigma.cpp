#include "deltaSigma.hpp"

void deltaSigma(std::vector<double>& x, std::vector<double>& y)
{
    y.clear();  // Make sure output vector is empty

    // H0 Coeffitients
    ac_fixed<12,4,true,AC_RND,AC_SAT> b00 = 7.3765809;
    ac_fixed<12,1,true,AC_RND,AC_SAT> a01 = 0.3466036;

    // H1 Coeffitients
    ac_fixed<12,1,true,AC_RND,AC_SAT> b10 = 0.424071040;
    ac_fixed<12,1,true,AC_RND,AC_SAT> a11 = 0.66591402;
    ac_fixed<12,3,true,AC_RND,AC_SAT> b11 = 2.782608716;
    ac_fixed<12,1,true,AC_RND,AC_SAT> a12 = 0.16260264;

    // H2 Coeffitients
    ac_fixed<12,4,true,AC_RND,AC_SAT> b20 = 4.606822182;
    ac_fixed<12,1,true,AC_RND,AC_SAT> a21 = 0.62380242;
    ac_fixed<12,1,true,AC_RND,AC_SAT> b21 = 0.023331537;
    ac_fixed<12,1,true,AC_RND,AC_SAT> a22 = 0.4509869;

    typedef ac_fixed<24,5,true,AC_RND,AC_SAT> fx_ss;

    fx_ss y_iir = 0, e = 0, y_i = 0;
    fx_ss x0 = 0, x0d = 0;
    fx_ss x1 = 0, w1 = 0, w1d = 0, w1dd = 0;
    fx_ss x2 = 0, w2 = 0, w2d = 0, w2dd = 0;
    ac_fixed<4,4,true,AC_RND,AC_SAT> v = 0;
    ac_fixed<12,4,true,AC_RND,AC_SAT> xin = 0;

    for(size_t i = 0; i < x.size(); i++)
    {
        xin = x[i];
        y_i = xin + y_iir;

        std::cout << "xin = " << xin.to_double() << ", y_i = " << y_i << std::endl;

        v = y_i;
        y.push_back(v.to_int());

        e = y_i - v;

        x0 = b00*e + a01*x0d;

        w1 = e + a11*w1d - a12*w1dd;
        x1 = b10*w1 - b11*w1d;

        w2 = e + a21*w2d - a22*w2dd;
        x2 = b21*w2d - b20*w2;

        y_iir = x0 + x1 + x2;

        x0d = x0;
        w1dd = w1d;
        w1d = w1;
        w2dd = w2d;
        w2d = w2;
    }
}