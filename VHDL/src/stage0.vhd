library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity stage0 is
	generic(
		KERNEL_ID	: integer := 7;
		COEF_L		: integer := 15;
		XWIDTH		: integer := 12;
		INT  		: integer := 1;
		FRAC 		: integer := 26
	);
	port(
		clk   	: in  std_logic;
		rst   	: in  std_logic;
		xin_i  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xin_q  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xout_i  : out std_logic_vector(XWIDTH-1 downto 0);
		xout_q  : out std_logic_vector(XWIDTH-1 downto 0)
	);
end entity;

architecture rtl of stage0 is
	
	constant DELTA 		: real := 0.00048828125;
	constant DELTA_I 	: real := -DELTA;
	constant DELTA_Q 	: real := DELTA;
	
	constant NUM_TAPS : integer := KERNEL_ID;
	
	signal enable 		: std_logic := '1';
	signal xout_i_delay : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout_q_delay : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');

begin

	enable <= '1';

	delay_i: entity work.delay
		generic map (
			KERNEL_ID   => KERNEL_ID,
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC,
			NUM_TAPS   	=> NUM_TAPS,
			DELTA		=> DELTA_I
		)
		port map (
			clk		=> clk,
			rst		=> rst, 
			en		=> enable, 		
			xin		=> xin_i,
			xout	=> xout_i_delay      
		);
		
	delay_q: entity work.delay
		generic map (
			KERNEL_ID   => KERNEL_ID,
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC,
			NUM_TAPS   	=> NUM_TAPS,
			DELTA		=> DELTA_Q
		)
		port map (
			clk		=> clk,
			rst		=> rst, 
			en		=> enable, 		
			xin		=> xin_q,
			xout	=> xout_q_delay      
		);

	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xout_i 	<= (others => '0');
				xout_q 	<= (others => '0');
			else	
				xout_i 	<= xout_i_delay;
				xout_q 	<= xout_q_delay;
			end if;
		end if;
	end process;

end architecture;