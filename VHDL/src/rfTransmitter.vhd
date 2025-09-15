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
		clk0	: in std_logic;
		clk1   	: in  std_logic;
		clk2  	: in  std_logic;
		clk3  	: in  std_logic;
		rst   	: in  std_logic;
		xin_i  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xin_q  	: in  std_logic_vector(XWIDTH-1 downto 0);
		
		xout_i_stage1	: out std_logic_vector(3 downto 0);
		xout_q_stage1	: out std_logic_vector(3 downto 0);
		xout_i_stage2	: out std_logic;
		xout_q_stage2	: out std_logic;
		xout_stage3		: out std_logic
	);
end entity;

architecture rtl of rfTransmitter is

	constant KERNEL_ID						: integer := 7;
	signal xout_i_delay, xout_q_delay 		: std_logic_vector(XWIDTH-1 downto 0) := (others => '0');

	signal xin_i_stage1_s, xin_q_stage1_s 	: std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout_i_stage1_s, xout_q_stage1_s : std_logic_vector(3 downto 0) := (others => '0');
	signal stage1_strobe 					: std_logic := '0';
	
	signal xin_i_stage2_s, xin_q_stage2_s	: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_i_stage2_s, xout_q_stage2_s : std_logic := '0';
	signal stage2_strobe 					: std_logic := '0';
	
	signal xin_i_stage3_s, xin_q_stage3_s	: std_logic := '0';
	signal xout_stage3_s					: std_logic := '0';
	signal stage3_strobe 					: std_logic := '0';

begin

	stage0_gen : entity work.stage0
		generic map (
			KERNEL_ID	=> KERNEL_ID,
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk   	=> clk0,
			rst   	=> rst,
			xin_i  	=> xin_i,
			xin_q  	=> xin_q,
			xout_i	=> xout_i_delay,
			xout_q	=> xout_q_delay
		);

	cdc01 : entity work.cdc
		port map (
			rst 		=> rst,
			clk_slow	=> clk0,
		    clk_fast	=> clk1,
			strobe		=> stage1_strobe
		);
	
	process(clk1)
	begin
		if rising_edge(clk1) then
			if rst = '1' then
				xin_i_stage1_s <= (others => '0');
				xin_q_stage1_s <= (others => '0');
			elsif stage1_strobe = '1' then
				xin_i_stage1_s <= xout_i_delay;
				xin_q_stage1_s <= xout_q_delay;
			end if;
		end if;
	end process;
	

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
			strobe 	=> stage1_strobe,
			xin_i  	=> xin_i_stage1_s,
			xin_q  	=> xin_q_stage1_s,
			xout_i	=> xout_i_stage1_s,
			xout_q	=> xout_q_stage1_s
		);
		
	process(clk1)
	begin
		if rising_edge(clk1) then
			if rst = '1' then
				xout_i_stage1 <= (others => '0');
				xout_q_stage1 <= (others => '0');
			else
				xout_i_stage1 <= xout_i_stage1_s;
				xout_q_stage1 <= xout_q_stage1_s;
			end if;
		end if;
	end process;
		
	cdc12 : entity work.cdc
		port map (
			rst 		=> rst,
			clk_slow	=> clk1,
		    clk_fast	=> clk2,
			strobe		=> stage2_strobe
		);
		
	process(clk2)
	begin
		if rising_edge(clk2) then
			if rst = '1' then
				xin_i_stage2_s <= (others => '0');
				xin_q_stage2_s <= (others => '0');
			elsif stage2_strobe = '1' then
				xin_i_stage2_s <= xout_i_stage1_s;
				xin_q_stage2_s <= xout_q_stage1_s;
			end if;
		end if;
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
			xin_i  	=> xin_i_stage2_s,
			xin_q  	=> xin_q_stage2_s,
			xout_i	=> xout_i_stage2_s,
			xout_q	=> xout_q_stage2_s
		);
		
	process(clk2)
	begin
		if rising_edge(clk2) then
			if rst = '1' then
				xout_i_stage2 <= '0';
				xout_q_stage2 <= '0';
			else
				xout_i_stage2 <= xout_i_stage2_s;
				xout_q_stage2 <= xout_q_stage2_s;
			end if;
		end if;
	end process;
	
	cdc23 : entity work.cdc
		port map (
			rst 		=> rst,
			clk_slow	=> clk2,
		    clk_fast	=> clk3,
			strobe		=> stage3_strobe
		);
		
	process(clk3)
	begin
		if rising_edge(clk3) then
			if rst = '1' then
				xin_i_stage3_s <= '0';
				xin_q_stage3_s <= '0';
			elsif stage3_strobe = '1' then
				xin_i_stage3_s <= xout_i_stage2_s;
				xin_q_stage3_s <= xout_q_stage2_s;
			end if;
		end if;
	end process;
	
	stage3_gen : entity work.stage3
		port map (
			clk   	=> clk3,
			rst   	=> rst,
			strobe	=> stage3_strobe,
			xin_i  	=> xin_i_stage3_s,
			xin_q  	=> xin_q_stage3_s,
			xout_iq	=> xout_stage3_s 
		);

	process(clk3)
	begin
		if rising_edge(clk3) then
			if rst = '1' then
				xout_stage3 <= '0';
			else
				xout_stage3 <= xout_stage3_s;
			end if;
		end if;
	end process;
	
end architecture;