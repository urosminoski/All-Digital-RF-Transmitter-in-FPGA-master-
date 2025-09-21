library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity osr8 is
	generic(
		COEF_L		: integer := 15;
		XWIDTH		: integer := 12;
		INT  		: integer := 1;
		GUARD_BITS	: integer := 4
	);
	port(
		clk   	: in  std_logic;
		rst   	: in  std_logic;
		strobe	: in  std_logic;
		xin  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xout	: out std_logic_vector(XWIDTH-1 downto 0);
		vout 	: out std_logic
	);
end entity;

architecture rtl of osr8 is

	constant INT_IN		: integer := INT + GUARD_BITS;
	constant FRAC_IN 	: integer := COEF_L + XWIDTH - INT;

	constant S0_OFF0 : integer := 0;
	constant S0_OFF1 : integer := 4;
	
	constant S1_OFF0 : integer := 0;
	constant S1_OFF1 : integer := 2;
	
	constant S2_OFF0 : integer := 0;
	constant S2_OFF1 : integer := 1;
	
	constant NUM_TAPS0 : integer := FIR0_N;
	constant NUM_TAPS1 : integer := FIR1_N;
	constant NUM_TAPS2 : integer := FIR2_N;
	
	-- constant XWIDTH_HB2 : integer := INT+FRAC+1;

	signal frame_cnt : std_logic_vector(2 downto 0) := (others => '0');
	
	-- signal x0, x1, x2 	: std_logic_vector(XWIDTH_HB2-1 downto 0);
	signal x0, x1, x2 	: sfixed(INT_IN downto -FRAC_IN);
	signal v0, v1, v2 	: std_logic;
	signal seed 		: std_logic;
	
	-- signal xin_hb2 	: std_logic_vector(XWIDTH_HB2-1 downto 0);
	signal xin_hb2 	: sfixed(INT_IN downto -FRAC_IN);

begin

	process(clk)
		variable cnt : unsigned(frame_cnt'high downto 0) := (others => '0');
	begin
		if rising_edge(clk) then
			if rst = '1' then
				cnt := (others => '0');
			else
				if cnt = "111" then
					cnt := (others => '0');
				else
					cnt := cnt + 1;
				end if;
			end if;
		end if;
		frame_cnt <= std_logic_vector(cnt);
	end process;
	
	seed <= '1' when (frame_cnt = "000") else '0';
	
	xin_hb2 <= resize(
			to_sfixed(xin, 0, -(XWIDTH-1)),
			INT_IN, -FRAC_IN
		);
	
	-- Stage 0
	u_s0: entity work.hb2_sched
		generic map(
			STAGE_ID   	=> 0,
			COEF_L		=> COEF_L,
			INT    		=> INT_IN,
			FRAC   		=> FRAC_IN,
			NUM_TAPS   	=> NUM_TAPS0,
			OFF0       	=> S0_OFF0,
			OFF1       	=> S0_OFF1
		)
		port map(
			clk        	=> clk,
			rst        	=> rst,
			seed       	=> seed,
			frame_cnt  	=> frame_cnt,
			xin        	=> xin_hb2,
			xout       	=> x0,
			vout       	=> v0
		);
		
	-- Stage 1
	u_s1: entity work.hb2_sched
		generic map(
			STAGE_ID   	=> 1,
			COEF_L		=> COEF_L,
			INT    		=> INT_IN,
			FRAC   		=> FRAC_IN,
			NUM_TAPS   	=> NUM_TAPS1,
			OFF0       	=> S1_OFF0,
			OFF1       	=> S1_OFF1
		)
		port map(
			clk        	=> clk,
			rst        	=> rst,
			seed       	=> v0,
			frame_cnt  	=> frame_cnt,
			xin        	=> x0,
			xout       	=> x1,
			vout       	=> v1
		);
		
	-- Stage 2
	u_s2: entity work.hb2_sched
		generic map(
			STAGE_ID   	=> 2,
			COEF_L		=> COEF_L,
			INT    		=> INT_IN,
			FRAC   		=> FRAC_IN,
			NUM_TAPS   	=> NUM_TAPS2,
			OFF0       	=> S2_OFF0,
			OFF1       	=> S2_OFF1
		)
		port map(
			clk        	=> clk,
			rst        	=> rst,
			seed       	=> v1,
			frame_cnt  	=> frame_cnt,
			xin        	=> x1,
			xout       	=> x2,
			vout       	=> v2
		);
		
	vout <= v2;
	xout <= to_slv(resize(x2, INT, -(XWIDTH-1-INT)));

end architecture;