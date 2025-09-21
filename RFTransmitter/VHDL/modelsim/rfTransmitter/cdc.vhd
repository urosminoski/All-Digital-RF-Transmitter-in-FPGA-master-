library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity cdc is
	port (
		rst 		: in  std_logic;
		clk_slow	: in  std_logic;
		clk_fast	: in  std_logic;
		strobe		: out std_logic
	);
end entity;

architecture behavioral of cdc is

	signal ff1, ff2, ff2_d 		: std_logic := '0';
	signal toggle				: std_logic := '0';

begin

	sync_slow : process(clk_slow)
	begin
		if rising_edge(clk_slow) then
			toggle <= not toggle;
		end if;
	end process;
	
	sync_fast : process(clk_fast)
	begin
		if rising_edge(clk_fast) then
			if rst = '1' then
				strobe <= '0';
			else
				ff1 	<= toggle;
				ff2 	<= ff1;
				ff2_d	<= ff2;
				
				strobe <= ff2 xor ff2_d;
			end if;
		end if;
	end process;

end architecture;