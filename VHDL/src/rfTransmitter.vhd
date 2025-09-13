library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity rfTransmitter is
	generic(
		LUT_ID		: integer := 3;
		COEF_L		: integer := 15;
		XWIDTH		: integer := 12;
		INT  		: integer := 1;
		FRAC 		: integer := 26
	);
	port(
		clk1   	: in  std_logic;
		clk2  	: in  std_logic;
		rst   	: in  std_logic;
		xin_i  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xin_q  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xout_i	: out std_logic;
		xout_q	: out std_logic
	);
end entity;

architecture rtl of rfTransmitter is

	signal xout_i_stage1 : std_logic_vector(3 downto 0) := (others => '0');
	signal xout_q_stage1 : std_logic_vector(3 downto 0) := (others => '0');
	
	signal xout_i_stage1_reg 	: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_q_stage1_reg 	: std_logic_vector(3 downto 0) := (others => '0');
	signal toggle12				: std_logic := '0';
	
	signal xout_i_stage2 	: std_logic := '0';
	signal xout_q_stage2 	: std_logic := '0';
	signal stage2_strobe 	: std_logic := '0';

begin

	stage1_gen : entity work.stage1
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk   	=> clk1,
			rst   	=> rst,
			xin_i  	=> xin_i,
			xin_q  	=> xin_q,
			xout_i	=> xout_i_stage1,
			xout_q	=> xout_q_stage1
		);
		
	sync12_slow : process(clk1)
	begin
		if rising_edge(clk1) then
			xout_i_stage1_reg 	<= xout_i_stage1;
			xout_q_stage1_reg 	<= xout_q_stage1;
			toggle12 			<= not toggle12; 
		end if;
	end process;
	
	sync12_fast : process(clk2)
		variable ff1, ff2 : std_logic := '0';
	begin
		if rising_edge(clk2) then
			ff1 := toggle12;
			ff2 := ff1;
		end if;
		stage2_strobe <= ff2;
	end process;
		
	stage2_gen : entity work.stage2
		generic map (
			LUT_ID		=> LUT_ID,
			XWIDTH		=> 4
		)
		port map (
			clk   	=> clk2,
			rst   	=> rst,
			strobe 	=> stage2_strobe,
			xin_i  	=> xout_i_stage1_reg,
			xin_q  	=> xout_q_stage1_reg,
			xout_i	=> xout_i_stage2,
			xout_q	=> xout_q_stage2
		);
		
	process(clk2)
	begin
		if rising_edge(clk2) then
			if rst = '1' then
				xout_i <= '0';
				xout_q <= '0';
			else
				xout_i <= xout_i_stage2;
				xout_q <= xout_q_stage2;
			end if;
		end if;
	end process;

end architecture;