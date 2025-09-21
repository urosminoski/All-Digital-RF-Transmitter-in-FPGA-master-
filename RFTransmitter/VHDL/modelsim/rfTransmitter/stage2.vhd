library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity stage2 is
	generic(
		LUT_ID	: integer := 3;
		XWIDTH	: integer := 4
	);
	port(
		clk   	: in  std_logic;
		rst   	: in  std_logic;
		strobe	: in  std_logic;
		xin_i  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xin_q  	: in  std_logic_vector(XWIDTH-1 downto 0);
		xout_i	: out std_logic;
		xout_q	: out std_logic
	);
end entity;

architecture rtl of stage2 is
	
	signal xout_i_lut : std_logic := '0';
	signal xout_q_lut : std_logic := '0';

begin

	lut_ser_i : entity work.lut_serializer
		generic map (
			LUT_ID  => LUT_ID,
			XWIDTH  => 4
		)
		port map (
			clk      => clk,
			rst      => rst,
			enable   => strobe,
			xin      => xin_i,
			xout     => xout_i_lut	
		);
		
	lut_ser_q : entity work.lut_serializer
		generic map (
			LUT_ID  => LUT_ID,
			XWIDTH  => 4
		)
		port map (
			clk      => clk,
			rst      => rst,
			enable   => strobe,
			xin      => xin_q,
			xout     => xout_q_lut
		);

	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xout_i <= '0';
				xout_q <= '0';
			else	
				xout_i <= xout_i_lut;
				xout_q <= xout_q_lut;
			end if;
		end if;
	end process;

end architecture;