----------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.deltaSigma_pkg.all;

library ieee_proposed;
use ieee_proposed.math_utility_pkg.all;
use ieee_proposed.fixed_pkg.all;
----------------------------------------------------------------------

entity deltaSigma is
	port(
		clk, rst 	: in std_logic;
		x 			: in sfixed(C_X_UPPER downto C_X_LOWER);
		y 			: out sfixed(C_Y_UPPER downto C_Y_LOWER)
	);
end entity;
----------------------------------------------------------------------
architecture rti of deltaSigma is

	-- Coefficients of H0.
	constant b00 : sfixed(3 downto -8)  := to_sfixed(7.3765809,	3, -8);
	constant a01 : sfixed(0 downto -11) := to_sfixed(0.3466036, 0, -11);
	-- Coefficients of H1.
	constant b10 : sfixed(0 downto -11) := to_sfixed(0.424071040,0, -11);
	constant b11 : sfixed(2 downto -9)  := to_sfixed(2.782608716, 2, -9);
	constant a11 : sfixed(0 downto -11) := to_sfixed(0.66591402, 0, -11);
	constant a12 : sfixed(0 downto -11) := to_sfixed(0.16260264, 0, -11);
	-- Coefficients of H2.
	constant b20 : sfixed(3 downto -8) 	:= to_sfixed(4.606822182, 3, -8);
	constant b21 : sfixed(0 downto -11) := to_sfixed(0.023331537, 0, -11);
	constant a21 : sfixed(0 downto -11) := to_sfixed(0.62380242, 0, -11);
	constant a22 : sfixed(0 downto -11) := to_sfixed(0.4509869, 0, -11);
	
begin

	process(clk, rst)
		-- IIR's output, quantizier input and quantization error.
		variable y_iir, e : sfixed(4 downto -19);
		variable y_i : sfixed(4 downto -19);
		-- H0.
		variable x0, x0d : sfixed(4 downto -19);
		-- H1.
		variable x1, w1, w1d, w1dd : sfixed(4 downto -19);
		-- H2.
		variable x2, w2, w2d, w2dd : sfixed(4 downto -19);
		-- Output.
		variable v : sfixed(C_Y_UPPER downto C_Y_LOWER);
	begin
		if(rst = '1') then
			y_iir 	:= to_sfixed(0, y_iir);
			y_i 	:= to_sfixed(0, y_i);
			e 		:= to_sfixed(0, e);
			x0 		:= to_sfixed(0, x0);
			x0d 	:= to_sfixed(0, x0d);
			x1 		:= to_sfixed(0, x1);
			w1 		:= to_sfixed(0, w1);
			w1d 	:= to_sfixed(0, w1d);
			w1dd 	:= to_sfixed(0, w1dd);
			x2 		:= to_sfixed(0, x2);
			w2 		:= to_sfixed(0, w2);
			w2d 	:= to_sfixed(0, w2d);
			w2dd 	:= to_sfixed(0, w2dd);
			v 		:= to_sfixed(0, v);
		elsif rising_edge(clk) then
			y_i := resize(x + y_iir, y_i'high, y_i'low);
			v 	:= resize(y_i, v'high, v'low);
			if(v(v'right) = '0') then
				v := resize(v + to_sfixed(1, v), v'high, v'low);
			end if;
			
			e 		:= resize(y_i - v, e'high, e'low);
			x0 		:= resize(b00*e + a01*x0d, x0'high, x0'low);
			w1 		:= resize(e + a11*w1d - a12*w1dd, w1'high, w1'low);
			x1 		:= resize(b10*w1 - b11*w1d, x1'high, x1'low);
			w2 		:= resize(e + a21*w2d - a22*w2dd, w2'high, w2'low);
			x2 		:= resize(b21*w2d - b20*w2, x2'high, x2'low);
			y_iir 	:= resize(x0 + x1 + x2, y_iir'high, y_iir'low);
			
			x0d 	:= x0;
			w1dd 	:= w1d;
			w1d 	:= w1;
			w2dd 	:= w2d;
			w2d 	:= w2;
		end if;
		y <= v;
	end process;
	
end architecture;
