library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity osr8 is
	generic(
		XIN_WIDTH  : integer := 12;
		XOUT_WIDTH : integer := 28
	);
	port(
		clk        : in  std_logic;
		rst        : in  std_logic;
		xin        : in  std_logic_vector(XIN_WIDTH-1 downto 0);
		xout       : out std_logic_vector(XOUT_WIDTH-1 downto 0)
	);
end entity;

architecture rtl of osr8 is

begin

	process(clk)
	begin
	
	end process;

end process;