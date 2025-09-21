library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

entity deltaSigma is
	port(
		clk, rst 	: in  std_logic;
		xi, xq 		: in  std_logic_vector(11 downto 0);
		yi, yq   	: out std_logic_vector(3 downto 0)
	);
end entity;

architecture rti of deltaSigma is
	
  
begin

	deltaSigma_i: entity work.deltaSigma
		port map (
			clk		=> clk,
			rst 	=> rst,
			x 		=> xi,
			y		=> yi
		);
		
	deltaSigma_q: entity work.deltaSigma
		port map (
			clk		=> clk,
			rst 	=> rst,
			x 		=> xq,
			y		=> yq
		);
	
	
end architecture;
