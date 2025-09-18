library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.delay_kernel_pkg.all;

entity delay is
	generic(
		KERNEL_ID   : integer := 7;   -- 3,5,7
		COEF_L		: integer := 15;
		XWIDTH		: integer := 12;
		INT  		: integer := 1;
		GUARD_BITS	: integer := 1;
		DELTA		: real := 0.5
	);
	port(
		clk, rst 	: in  std_logic;
		en 			: in  std_logic;
		xin   		: in  std_logic_vector(XWIDTH-1 downto 0);
		xout        : out std_logic_vector(XWIDTH-1 downto 0)
	);
end entity;

architecture rtl of delay is

	constant INT_IN : integer := INT+GUARD_BITS;

	constant FRAC 		: integer := COEF_L + XWIDTH - INT;
	constant NUM_TAPS	: integer := KERNEL_ID;
	constant D 			: integer := NUM_TAPS;--(NUM_TAPS - 1) / 2;

	subtype xin_t 	is sfixed(INT downto -(XWIDTH-INT-1));
	subtype coef_t 	is sfixed(0 downto -COEF_L);
	subtype acc_t  	is sfixed(INT_IN downto -FRAC);

	type xin_vec_t  	is array (0 to D-1) 		of xin_t;
	type coef_vec_t  	is array (0 to NUM_TAPS-1) 	of coef_t;
	type mul_vec_t   	is array (0 to NUM_TAPS-1)	of acc_t;
	type shift_vec_t 	is array (0 to NUM_TAPS-1)	of acc_t;

	signal xin_sf 	: xin_t := (others => '0');
	signal xin_d	: xin_t := (others => '0');
	signal xout_d 	: acc_t := (others => '0');
	
	signal sft_xin	: xin_vec_t 	:= (others => (others => '0'));
	signal coef		: coef_vec_t	:= (others => (others => '0'));
	signal mul 		: mul_vec_t		:= (others => (others => '0'));
	signal sft		: shift_vec_t 	:= (others => (others => '0'));
	signal acc		: acc_t			:= (others => '0');
	
	signal f	: sfixed(0 downto -(INT_IN+FRAC)) := to_sfixed(DELTA, 0, -(INT_IN+FRAC));

begin

	gen_k3: if KERNEL_ID = 3 generate
		gen_k3_i: for i in 0 to NUM_TAPS-1 generate
		begin
			coef(i) <= to_sfixed(K3_R(i), 0, -COEF_L);
		end generate;
	end generate;
	
	gen_k5: if KERNEL_ID = 5 generate
		gen_s5_i: for i in 0 to NUM_TAPS-1 generate
		begin
			coef(i) <= to_sfixed(K5_R(i), 0, -COEF_L);
		end generate;
	end generate;
	
	gen_k7: if KERNEL_ID = 7 generate
		gen_s7_i: for i in 0 to NUM_TAPS-1 generate
		begin
			coef(i) <= to_sfixed(K7_R(i), 0, -COEF_L);
		end generate;
	end generate;
	
	-- latch ulaza
	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xin_sf   <= (others => '0');
			elsif en = '1' then
				xin_sf   <= to_sfixed(xin, xin_sf'high, xin_sf'low);
			end if;
		end if;
	end process;
	
	-- Kasnjenje ulaznog signala, da se poravna sa filtriranim signalom
	-- y = x[n-D] + delta * x_fir[n], D = (N-1)/2
	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				sft_xin <= (others => (others => '0'));
			else
				sft_xin(0) <= xin_sf;
				
				for i in D-1 downto 1 loop
					sft_xin(i) <= sft_xin(i-1);
				end loop;
			end if;
		end if;
	end process;
	
	xin_d <= sft_xin(D-1);

	-- množenja (transposed: trenutni x * svi koefovi)
	gen_mul: for i in 0 to NUM_TAPS-1 generate
	begin
		mul(i) <= resize(xin_sf * coef(NUM_TAPS-1-i), mul(i)'high, mul(i)'low);
	end generate;

	-- akumulatori (osvežavaj samo na seed)
	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				sft <= (others => (others => '0'));
			elsif en = '1' then
				if NUM_TAPS > 2 then
					for i in 0 to NUM_TAPS-3 loop
						sft(i) <= resize(sft(i+1) + mul(i+1), sft(i)'high, sft(i)'low);
					end loop;
				end if;
				sft(NUM_TAPS-2) <= mul(NUM_TAPS-1);
			end if;
		end if;
	end process;
	
	acc <= resize(sft(0) + mul(0), acc'high, acc'low);
	
	xout_d <= resize(acc*f + xin_d, xout_d'high, xout_d'low);
	
	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xout <= (others => '0');
			elsif en = '1' then
				xout <= to_slv(resize(xout_d, INT, -(XWIDTH-INT-1)));
			end if;
		end if;
	end process;

end architecture;