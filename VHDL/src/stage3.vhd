library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity stage3 is
	port(
		clk   	: in  std_logic;
		rst   	: in  std_logic;
		strobe	: in  std_logic;
		xin_i  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xin_q  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xout_iq	: out std_logic
	);
end entity;

architecture rtl of stage2 is
	
	signal xout_i_lut : std_logic := '0';
	signal xout_q_lut : std_logic := '0';

begin

	

end architecture;