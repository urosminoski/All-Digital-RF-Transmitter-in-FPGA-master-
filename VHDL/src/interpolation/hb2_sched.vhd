library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity hb2_sched is
	generic(
		STAGE_ID   	: integer := 0;   -- 0,1,2
		COEF_L		: integer := 15;
		INT			: integer := 1;
		FRAC		: integer := 26;
		XWIDTH		: integer := 12;
		NUM_TAPS   	: integer := 42;  -- ef. dužina u ovoj instanci (padujemo nulama ako treba)
		OFF0       	: integer := 0;   -- seed+OFF0 -> ph0 (even)
		OFF1       	: integer := 4    -- seed+OFF1 -> ph1 (odd)
	);
	port(
		clk        : in  std_logic;
		rst        : in  std_logic;
		seed       : in  std_logic;                  -- 1 clk: novi ulaz za stejdž
		frame_cnt  : in  std_logic_vector(2 downto 0); -- 0..7 @ 8*fs
		xin        : in  std_logic_vector(XWIDTH-1 downto 0);
		xout       : out std_logic_vector(XWIDTH-1 downto 0);
		vout       : out std_logic                   -- 1 clk kada je xout valid
	);
end entity;

architecture rtl of hb2_sched is

	subtype coef_t is sfixed(0 downto -COEF_L);
	subtype acc_t  is sfixed(INT downto -FRAC);

	type coef_vec_t  is array (0 to NUM_TAPS-1) of coef_t;
	type mul_vec_t   is array (0 to NUM_TAPS-1) of acc_t;
	type shift_vec_t is array (0 to NUM_TAPS-1) of acc_t;

	signal frame_u     : unsigned(2 downto 0);
	signal seed_cnt    : unsigned(2 downto 0) := (others => '0');

	signal xin_sf      : sfixed(0 downto -(XWIDTH-1)) := (others => '0');

	signal coef_ph0    : coef_vec_t;
	signal coef_ph1    : coef_vec_t;

	signal mul_ph0     : mul_vec_t;
	signal mul_ph1     : mul_vec_t;

	signal sft_ph0     : shift_vec_t := (others => (others => '0'));
	signal sft_ph1     : shift_vec_t := (others => (others => '0'));

	signal acc_ph0     : acc_t;
	signal acc_ph1     : acc_t;

	signal y_ph0_buf   : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal y_ph1_buf   : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');

	signal fire0       : std_logic;
	signal fire1       : std_logic;
	
	signal tmp1, tmp2 : unsigned(2 downto 0);

	function add_mod8(a : unsigned(2 downto 0); b : natural) return unsigned is
		variable tmp : natural := to_integer(a) + b;
	begin
		return to_unsigned(tmp mod 8, 3);
	end function;
begin
	frame_u <= unsigned(frame_cnt);

	-- Koefovi: popuni sfixed nizove iz paketa; ako je NUM_TAPS duži, paduj nulama
	gen_s0: if STAGE_ID = 0 generate
		gen_s0_i: for i in 0 to NUM_TAPS-1 generate
		begin
			coef_ph0(i) <= to_sfixed(FIR0_PH0_R(i), 0, -COEF_L);
			coef_ph1(i) <= to_sfixed(FIR0_PH1_R(i), 0, -COEF_L);
		end generate;
	end generate;

	gen_s1: if STAGE_ID = 1 generate
		gen_s1_i: for i in 0 to NUM_TAPS-1 generate
		begin
			coef_ph0(i) <= to_sfixed(FIR1_PH0_R(i), 0, -COEF_L);
			coef_ph1(i) <= to_sfixed(FIR1_PH1_R(i), 0, -COEF_L);
		end generate;
	end generate;

	gen_s2: if STAGE_ID = 2 generate
		gen_s2_i: for i in 0 to NUM_TAPS-1 generate
		begin
			coef_ph0(i) <= to_sfixed(FIR2_PH0_R(i), 0, -COEF_L);
			coef_ph1(i) <= to_sfixed(FIR2_PH1_R(i), 0, -COEF_L);
		end generate;
	end generate;

	-- latch ulaza + seeding
	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				xin_sf   <= (others => '0');
				seed_cnt <= (others => '0');
			elsif seed='1' then
				xin_sf   <= to_sfixed(xin, xin_sf'high, xin_sf'low);
				seed_cnt <= add_mod8(frame_u, 1);
			end if;
		end if;
	end process;

	-- množenja (transposed: trenutni x * svi koefovi)
	gen_mul: for i in 0 to NUM_TAPS-1 generate
	begin
		mul_ph0(i) <= resize(xin_sf * coef_ph0(NUM_TAPS-1-i), mul_ph0(i)'high, mul_ph0(i)'low);
		mul_ph1(i) <= resize(xin_sf * coef_ph1(NUM_TAPS-1-i), mul_ph1(i)'high, mul_ph1(i)'low);
	end generate;

	-- akumulatori (osvežavaj samo na seed)
	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				sft_ph0 <= (others => (others => '0'));
			elsif seed='1' then
				if NUM_TAPS > 2 then
					for i in 0 to NUM_TAPS-3 loop
						sft_ph0(i) <= resize(sft_ph0(i+1) + mul_ph0(i+1), sft_ph0(i)'high, sft_ph0(i)'low);
					end loop;
				end if;
				sft_ph0(NUM_TAPS-2) <= mul_ph0(NUM_TAPS-1);
			end if;
		end if;
	end process;

	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				sft_ph1 <= (others => (others => '0'));
			elsif seed='1' then
				if NUM_TAPS > 2 then
					for i in 0 to NUM_TAPS-3 loop
						sft_ph1(i) <= resize(sft_ph1(i+1) + mul_ph1(i+1), sft_ph1(i)'high, sft_ph1(i)'low);
					end loop;
				end if;
				sft_ph1(NUM_TAPS-2) <= mul_ph1(NUM_TAPS-1);
			end if;
		end if;
	end process;

	acc_ph0 <= resize(sft_ph0(0) + mul_ph0(0), acc_ph0'high, acc_ph0'low);
	acc_ph1 <= resize(sft_ph1(0) + mul_ph1(0), acc_ph1'high, acc_ph1'low);

	-- snimi oba rezultata (ph0/ph1) na seed
	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				y_ph0_buf <= (others => '0');
				y_ph1_buf <= (others => '0');
			elsif seed='1' then
				y_ph0_buf <= to_slv(resize(acc_ph0, 0, -(XWIDTH-1)));
				y_ph1_buf <= to_slv(resize(acc_ph1, 0, -(XWIDTH-1)));
			end if;
		end if;
	end process;
	
	tmp1 <= add_mod8(seed_cnt, OFF0);
	tmp2 <= add_mod8(seed_cnt, OFF1);
	
	-- fire = seed+OFF (mod 8) i izbacivanje
	fire0 <= '1' when frame_u = add_mod8(seed_cnt, OFF0) else '0';
	fire1 <= '1' when frame_u = add_mod8(seed_cnt, OFF1) else '0';

	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				xout <= (others => '0');
				vout <= '0';
			else
				vout <= '0';
				if fire0='1' then
					xout <= y_ph0_buf;  -- ph0
					vout <= '1';
				elsif fire1='1' then
					xout <= y_ph1_buf;  -- ph1
					vout <= '1';
				end if;
			end if;
		end if;
	end process;
end architecture;
