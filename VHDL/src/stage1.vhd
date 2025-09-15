library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity stage1 is
	generic(
		COEF_L		: integer := 15;
		XWIDTH		: integer := 12;
		INT  		: integer := 1;
		FRAC 		: integer := 26
	);
	port(
		clk   	: in  std_logic;
		rst   	: in  std_logic;
		strobe	: in  std_logic;
		xin_i  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xin_q  	: in  std_logic_vector(XWIDTH-1 downto 0);
		
		xout_i_osr8_test	: out std_logic_vector(XWIDTH-1 downto 0);
		xout_q_osr8_test	: out std_logic_vector(XWIDTH-1 downto 0);
		
		xout_i	: out std_logic_vector(3 downto 0);
		xout_q	: out std_logic_vector(3 downto 0)
	);
end entity;

architecture rtl of stage1 is

	constant LUT_ID : integer := 3;
	
	constant DELTA 		: real := 0.00390625;
	constant DELTA_I 	: real := -DELTA;
	constant DELTA_Q 	: real := DELTA;

	signal factor	: sfixed(4 downto -(XWIDTH-5));
	signal factor1, factor2	: sfixed(4 downto -(XWIDTH-5));
	signal xi_1, xq_1 		: sfixed(3 downto -(XWIDTH-4));
	signal xi_2, xq_2 		: sfixed(3 downto -(XWIDTH-4));
	signal xi_2_ds, xq_2_ds : sfixed(3 downto -8);
	
	signal xin_i_delay, xin_q_delay 	: std_logic_vector(XWIDTH-1 downto 0);
	signal xout_i_delay, xout_q_delay 	: std_logic_vector(XWIDTH-1 downto 0);
	
	signal xout_i_osr8, xout_q_osr8 : std_logic_vector(XWIDTH-1 downto 0);
	signal vout_i, vout_q 	: std_logic := '0';
	
	signal xin_i_ds, xin_q_ds 	: std_logic_vector(11 downto 0);
	signal xout_i_ds, xout_q_ds	: std_logic_vector(3 downto 0);
	
	signal cnt 		: unsigned(2 downto 0) := (others => '0');
	signal delay_en : std_logic := '0';

begin

	-- process(clk)
	-- begin
		-- if rising_edge(clk) then
			-- if rst = '1' then
				-- cnt <= (others => '0');
			-- else
				-- if cnt = "111" then
					-- cnt <= (others => '0');
				-- else
					-- cnt <= cnt + 1;
				-- end if;
			-- end if;
		-- end if;
	-- end process;
	
	-- delay_en <= '1' when (cnt = "000") else '0';

	-- delay_i: entity work.delay
		-- generic map (
			-- KERNEL_ID   => 7,
			-- COEF_L		=> COEF_L,
			-- XWIDTH		=> XWIDTH,
			-- INT  		=> INT,
			-- FRAC 		=> FRAC,
			-- NUM_TAPS   	=> 7,
			-- DELTA		=> DELTA_I
		-- )
		-- port map (
			-- clk		=> clk,
			-- rst		=> rst, 
			-- en		=> delay_en, 		
			-- xin		=> xin_i,
			-- xout	=> xout_i_delay      
		-- );
		
	-- delay_q: entity work.delay
		-- generic map (
			-- KERNEL_ID   => 7,
			-- COEF_L		=> COEF_L,
			-- XWIDTH		=> XWIDTH,
			-- INT  		=> INT,
			-- FRAC 		=> FRAC,
			-- NUM_TAPS   	=> 7,
			-- DELTA		=> DELTA_Q
		-- )
		-- port map (
			-- clk		=> clk,
			-- rst		=> rst, 
			-- en		=> delay_en, 		
			-- xin		=> xin_q,
			-- xout	=> xout_q_delay      
		-- );

	osr8_i: entity work.osr8
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			strobe 	=> strobe,
			xin   	=> xin_i, --xout_i_delay,
			xout   	=> xout_i_osr8,
			vout 	=> vout_i
		);
		
	osr8_q: entity work.osr8
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			strobe 	=> strobe,
			xin   	=> xin_q, --xout_q_delay,
			xout   	=> xout_q_osr8,
			vout 	=> vout_q
		);
		
	xout_i_osr8_test	<= xout_i_osr8;
	xout_q_osr8_test	<= xout_q_osr8;
	
	-- factor1 <= to_sfixed(7.5, factor1'high, factor1'low);
	-- factor2 <= to_sfixed(2, factor2'high, factor2'low);
		
	-- xi_1 <= resize(to_sfixed(xout_i_osr8, 0, -(XWIDTH-1)) * factor1, xi_1'high, xi_1'low);
	-- xq_1 <= resize(to_sfixed(xout_q_osr8, 0, -(XWIDTH-1)) * factor1, xq_1'high, xq_1'low);
	
	-- xin_i_delay <= to_slv(xi_1);
	-- xin_q_delay <= to_slv(xq_1);
	
	-- delay_i: entity work.delay
		-- generic map (
			-- KERNEL_ID   => 7,
			-- COEF_L		=> COEF_L,
			-- XWIDTH		=> XWIDTH,
			-- INT  		=> INT,
			-- FRAC 		=> FRAC,
			-- NUM_TAPS   	=> 7,
			-- DELTA		=> DELTA_I
		-- )
		-- port map (
			-- clk		=> clk,
			-- rst		=> rst, 
			-- en		=> '1', 		
			-- xin		=> xin_i_delay,
			-- xout	=> xout_i_delay      
		-- );
		
	-- delay_q: entity work.delay
		-- generic map (
			-- KERNEL_ID   => 7,
			-- COEF_L		=> COEF_L,
			-- XWIDTH		=> XWIDTH,
			-- INT  		=> INT,
			-- FRAC 		=> FRAC,
			-- NUM_TAPS   	=> 7,
			-- DELTA		=> DELTA_Q
		-- )
		-- port map (
			-- clk		=> clk,
			-- rst		=> rst, 
			-- en		=> '1', 		
			-- xin		=> xin_q_delay,
			-- xout	=> xout_q_delay      
		-- );
		
	-- xi_2 <= resize(to_sfixed(xout_i_delay, 0, -(XWIDTH-1)) * factor2, xi_2'high, xi_2'low);
	-- xq_2 <= resize(to_sfixed(xout_q_delay, 0, -(XWIDTH-1)) * factor2, xq_2'high, xq_2'low);
	
	-- xi_1 <= resize(to_sfixed(xout_i_osr8, 0, -(XWIDTH-1)) * factor1, xi_1'high, xi_1'low);
	-- xq_1 <= resize(to_sfixed(xout_q_osr8, 0, -(XWIDTH-1)) * factor1, xq_1'high, xq_1'low);
	
	-- xi_2 <= resize(xi_1 * factor2, xi_2'high, xi_2'low);
	-- xq_2 <= resize(xq_1 * factor2, xq_2'high, xq_2'low);
	
	factor <= to_sfixed(50, factor'high, factor'low);
	 
	xi_2 <= resize(to_sfixed(xout_i_osr8, INT, -(XWIDTH-1-INT)) * factor, xi_2'high, xi_2'low);
	xq_2 <= resize(to_sfixed(xout_q_osr8, INT, -(XWIDTH-1-INT)) * factor, xq_2'high, xq_2'low);
	
	xin_i_delay <= to_slv(xi_2);
	xin_q_delay <= to_slv(xq_2);
	
	
	delay_i: entity work.delay
		generic map (
			KERNEL_ID   => 7,
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> 3,
			FRAC 		=> XWIDTH-4,
			NUM_TAPS   	=> 7,
			DELTA		=> DELTA_I
		)
		port map (
			clk		=> clk,
			rst		=> rst, 
			en		=> '1', 		
			xin		=> xin_i_delay,
			xout	=> xout_i_delay      
		);
		
	delay_q: entity work.delay
		generic map (
			KERNEL_ID   => 7,
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> 3,
			FRAC 		=> XWIDTH-4,
			NUM_TAPS   	=> 7,
			DELTA		=> DELTA_Q
		)
		port map (
			clk		=> clk,
			rst		=> rst, 
			en		=> '1', 		
			xin		=> xin_q_delay,
			xout	=> xout_q_delay      
		);
	
	xi_2_ds <= resize(to_sfixed(xout_i_delay, INT, -(XWIDTH-1-INT)), xi_2_ds'high, xi_2_ds'low);
	xq_2_ds <= resize(to_sfixed(xout_q_delay, INT, -(XWIDTH-1-INT)), xq_2_ds'high, xq_2_ds'low);
	
	-- xin_i_ds <= to_slv(xi_2);
	-- xin_q_ds <= to_slv(xq_2);
	
	xin_i_ds <= to_slv(xi_2_ds);--xin_i_delay;--xout_i_delay;
	xin_q_ds <= to_slv(xq_2_ds);--xin_q_delay;--xout_q_delay;
	
	deltaSigma_i: entity work.deltaSigma
		port map (
			clk		=> clk,
			rst 	=> rst,
			x 		=> xin_i_ds,
			y		=> xout_i_ds
		);
		
	deltaSigma_q: entity work.deltaSigma
		port map (
			clk		=> clk,
			rst 	=> rst,
			x 		=> xin_q_ds,
			y		=> xout_q_ds
		);

	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xout_i 	<= (others => '0');
				xout_q 	<= (others => '0');
			else	
				xout_i 	<= xout_i_ds;
				xout_q 	<= xout_q_ds;
			end if;
		end if;
	end process;

end architecture;