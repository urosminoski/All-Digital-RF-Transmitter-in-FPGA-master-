library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_deltaSigma is
end entity;

architecture tb of tb_deltaSigma is
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;

	signal clk       : std_logic := '0';
	signal rst       : std_logic := '1';
	signal x         : std_logic_vector(11 downto 0) := (others => '0');
	signal y         : std_logic_vector(3 downto 0)  := (others => '0');

	-- Lokalni “handshake” u TB:
	signal out_ready : std_logic := '0';
	signal xout_en   : std_logic := '1';  -- ako nemaš en iz UUT, samo drži '1'

	file input_file  : text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\xin_test.txt";
	file output_file : text open write_mode is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\xout_test.txt";

begin
	uut: entity work.deltaSigma
		port map (
			clk => clk,
			rst => rst,
			x   => x,
			y   => y
		);

	clk <= not clk after C_CLK_PERIOD/2;
	rst <= '0' after 6*C_CLK_PERIOD;

	-- Čitanje ulaza, sa EOF zaštitom
	read_file : process(clk)
		variable L : line;
		variable v : integer;
	begin
		if rising_edge(clk) then
			if rst = '1' then
				x        <= (others => '0');
				out_ready <= '0';
			else
				if not endfile(input_file) then
					readline(input_file, L);
					read(L, v);
					x        <= std_logic_vector(to_signed(v, 12));
					out_ready <= '1';
				else
					out_ready <= '0';
				end if;
			end if;
		end if;
	end process;

	-- Upis izlaza
	write_file : process(clk)
		variable L : line;
	begin
		if falling_edge(clk) then
			if out_ready = '1' and xout_en = '1' then
				write(L, to_integer(signed(y)));
				writeline(output_file, L);
			end if;
		end if;
	end process;

end architecture;
