library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity rfTransmitter is
	generic(
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
		xout_i	: out std_logic_vector(3 downto 0);
		xout_q	: out std_logic_vector(3 downto 0)
	);
end entity;

architecture rtl of rfTransmitter is

	signal factor	: sfixed(4 downto -(XWIDTH-5));
	signal xi, xq 	: sfixed(3 downto -(XWIDTH-4));
	
	signal xi_osr8, xq_osr8 : std_logic_vector(XWIDTH-1 downto 0);
	signal vout_i, vout_q 	: std_logic := '0';
	
	signal xin_i_ds, xin_q_ds 	: std_logic_vector(XWIDTH-1 downto 0);
	signal xout_i_ds, xout_q_ds	: std_logic_vector(3 downto 0);

begin

	factor <= to_sfixed(30, factor'high, factor'low);

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
			xin   	=> xin_i,
			xout   	=> xi_osr8,
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
			xin   	=> xin_q,
			xout   	=> xq_osr8,
			vout 	=> vout_q
		);
		
	xi <= resize(to_sfixed(xi_osr8, 0, -(XWIDTH-1)) * factor, xi'high, xi'low);
	xq <= resize(to_sfixed(xq_osr8, 0, -(XWIDTH-1)) * factor, xq'high, xq'low);
	
	xin_i_ds <= to_slv(xi);
	xin_q_ds <= to_slv(xq);
	
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

	xout_i <= xout_i_ds;
	xout_q <= xout_q_ds;

end architecture;