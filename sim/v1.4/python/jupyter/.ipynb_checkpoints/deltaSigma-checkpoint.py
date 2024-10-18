from fxpmath import Fxp
import numpy as np

def deltaSigma(x, n_word, n_frac, overflow='saturate', rounding='around'):
    # Coefficients of H0.
    b00 = Fxp(7.3765809, n_word=12, n_frac=8, overflow='saturate', rounding='around')
    a01 = Fxp(0.3466036, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    # Coefficients of H1.
    b10 = Fxp(0.424071040, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    b11 = Fxp(2.782608716, n_word=12, n_frac=9, overflow='saturate', rounding='around')
    a11 = Fxp(0.66591402, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    a12 = Fxp(0.16260264, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    # Coefficients of H2.
    b20 = Fxp(4.606822182, n_word=12, n_frac=8, overflow='saturate', rounding='around')
    b21 = Fxp(0.023331537, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    a21 = Fxp(0.62380242, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    a22 = Fxp(0.4509869, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    
    y_iir = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    e = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    y_i = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x0 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x0d = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x1 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w1 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w1d = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w1dd = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x2 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w2 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w2d = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w2dd = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    v = Fxp(0, n_word=4, n_frac=0, overflow='saturate', rounding='around')
    
    y = np.zeros(len(x))
    
    for i in range(len(x)):
        y_i( (x[i]+y_iir)() )
        v( y_i() )
        # if v() % 2 == 0:
        #     v( (v+1)() )
        y[i] = v()
        e( (y_i-v)() )
    
        x0( (b00*e+a01*x0d)() )
        w1( (e+a11*w1d-a12*w1dd)() )
        x1( (b10*w1-b11*w1d)() )
        w2( (e+a21*w2d-a22*w2dd)() )
        x2( (b21*w2d-b20*w2)() )
        y_iir( (x0+x1+x2)() )
    
        x0d( x0() )
        w1dd( w1d() )
        w1d( w1() )
        w2dd( w2d() )
        w2d( w2() )
    return y