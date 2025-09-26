library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

entity deltaSigma is
	generic (
		DTYPE	: integer := 0; 	-- 0 - mid-tread, 1 - mid-rise
		XWIDTH	: integer := 12
	);
	port (
		clk	: in  std_logic;
		rst : in  std_logic;
		x 	: in  std_logic_vector(XWIDTH-1 downto 0);
		y 	: out std_logic_vector(3 downto 0)
		-- x : in sfixed(3 downto -8);
		-- y : out sfixed(3 downto 0)
	);
end entity;

architecture rti of deltaSigma is
	-- Koeficijenti (real → sfixed):
	constant b00 : sfixed(3 downto -(XWIDTH-4))	:= to_sfixed(7.3765809,  3, -(XWIDTH-4));
	constant a01 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.3466036,  0, -(XWIDTH-1));
	constant b10 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.424071040,0, -(XWIDTH-1));
	constant b11 : sfixed(2 downto -(XWIDTH-3))	:= to_sfixed(2.782608716,2, -(XWIDTH-3));
	constant a11 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.66591402, 0, -(XWIDTH-1));
	constant a12 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.16260264, 0, -(XWIDTH-1));
	constant b20 : sfixed(3 downto -(XWIDTH-4))	:= to_sfixed(4.606822182,3, -(XWIDTH-4));
	constant b21 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.023331537,0, -(XWIDTH-1));
	constant a21 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.62380242, 0, -(XWIDTH-1));
	constant a22 : sfixed(0 downto -(XWIDTH-1)) := to_sfixed(0.4509869,  0, -(XWIDTH-1));

	signal x_sfixed : sfixed(3 downto -(XWIDTH-4));
	signal y_sfixed : sfixed(3 downto 0);
  
	-- signal y_iir, e            : sfixed(3 downto -20);
    -- signal y_i                 : sfixed(3 downto -20);
    -- signal x0, x0d             : sfixed(3 downto -20);
    -- signal x1, w1, w1d, w1dd   : sfixed(3 downto -20);
    -- signal x2, w2, w2d, w2dd   : sfixed(3 downto -20);
    -- signal v                   : sfixed(3 downto 0);
  
begin
	process(clk, rst)
		variable y_iir, e            : sfixed(3 downto -(2*XWIDTH-4));
		variable y_i                 : sfixed(3 downto -(2*XWIDTH-4));
		variable x0, x0d             : sfixed(3 downto -(2*XWIDTH-4));
		variable x1, w1, w1d, w1dd   : sfixed(3 downto -(2*XWIDTH-4));
		variable x2, w2, w2d, w2dd   : sfixed(3 downto -(2*XWIDTH-4));
		variable v                   : sfixed(3 downto 0);
		
		variable k_code 	: sfixed(3 downto 0);
		variable k_reexp 	: sfixed(y_i'high downto y_i'low);
		variable v_fb 		: sfixed(3 downto -1);
	begin
		if rst = '1' then
			y_iir := to_sfixed(0, y_iir);  y_i := to_sfixed(0, y_i);  e := to_sfixed(0, e);
			x0 := to_sfixed(0, x0); x0d := to_sfixed(0, x0d);
			x1 := to_sfixed(0, x1); w1 := to_sfixed(0, w1); w1d := to_sfixed(0, w1d); w1dd := to_sfixed(0, w1dd);
			x2 := to_sfixed(0, x2); w2 := to_sfixed(0, w2); w2d := to_sfixed(0, w2d); w2dd := to_sfixed(0, w2dd);
			v  := to_sfixed(0, v);
			x_sfixed <= to_sfixed(0, x_sfixed);
			y_sfixed <= to_sfixed(0, y_sfixed);
			
			k_code 	:= (others => '0');
			k_reexp := (others => '0');
			v_fb 	:= (others => '0');
			
		elsif rising_edge(clk) then
			x_sfixed <= to_sfixed(x, x_sfixed'high, x_sfixed'low);
			y_i      := resize(x_sfixed + y_iir, y_i'high, y_i'low);
			-- y_i      := resize(x + y_iir, y_i'high, y_i'low);
			if DTYPE = 0 then
				v := resize(y_i, v'high, v'low);
				e := resize(y_i - v, e'high, e'low);
			else
				k_code := resize(y_i, k_code'high, k_code'low);
				k_reexp := resize(k_code, k_reexp'high, k_reexp'low);
				if (y_i < 0) and (y_i /= k_reexp) then
				  k_code := k_code - to_sfixed(1, k_code);
				end if;
				v_fb   := resize(k_code, v_fb'high, v_fb'low) + to_sfixed(0.5, v_fb);
				e      := resize(y_i - v_fb, e'high, e'low);
			end if;
			-- if v(v'right) = '0' then
				-- v := resize(v + to_sfixed(1, v), v'high, v'low);
			-- end if;

			x0  := resize(b00*e + a01*x0d,            	x0'high, x0'low);
			w1  := resize(e + a11*w1d - a12*w1dd,     	w1'high, w1'low);
			x1  := resize(b10*w1 - b11*w1d,           	x1'high, x1'low);
			w2  := resize(e + a21*w2d - a22*w2dd,     	w2'high, w2'low);
			x2  := resize(b21*w2d - b20*w2,           	x2'high, x2'low);
			y_iir := resize(x0 + x1 + x2,             	y_iir'high, y_iir'low);
			x0d := x0;  w1dd := w1d;  w1d := w1;  w2dd := w2d;  w2d := w2;
		end if;
		
		if DTYPE = 0 then
			y_sfixed <= v;
		else
			y_sfixed <= k_code;
		end if;
		-- y <= v;
	end process;

	y <= to_slv(y_sfixed);                                   -- sfixed → std_logic_vector:contentReference[oaicite:2]{index=2}
end architecture;
