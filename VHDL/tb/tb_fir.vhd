library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_fir is
end entity;

architecture tb of tb_fir is
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;
	
	constant WIDTH_X : integer := 12;
	constant WIDTH_Y : integer := 16;

	signal clk0, clk1, clk2   	: std_logic := '0';
	signal rst       			: std_logic := '1';
	signal x         			: std_logic_vector(WIDTH_X-1 downto 0) := (others => '0');
	signal y, y0, y1         	: std_logic_vector(WIDTH_Y-1 downto 0) := (others => '0');
	-- signal x : sfixed(3 downto -8);
	-- signal y : sfixed(3 downto 0);

	-- Lokalni “handshake” u TB:
	signal out_ready : std_logic := '0';

	file input_file  		: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xin_test.txt";
	file output_file  		: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xout_test.txt";
	file output_file_0  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xout0_test.txt";
	file output_file_1  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xout1_test.txt";

begin
	uut: entity work.fir0
		generic map (
			WIDTH_X => WIDTH_X,
			WIDTH_Y => WIDTH_Y
		)
		port map (
			clk0 => clk0,
			clk1 => clk1,
			clk2 => clk2,
			rst => rst,
			x   => x,
			y 	=> y,
			y0   => y0,
			y1   => y1
		);

	clk0 <= not clk0 after C_CLK_PERIOD/2;
	clk1 <= not clk1 after C_CLK_PERIOD/4;
	rst <= '0' after 6*C_CLK_PERIOD;
	
	read_files : process(clk0)
		variable L : line;
		variable r : real;
		variable s : sfixed(0 downto -WIDTH_X+1);
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				x        <= (others => '0');
				out_ready <= '0';
			else
				-- Čitamo paralelno: zaustavi kad ijedan fajl dođe do kraja
				if (not endfile(input_file))then
					-- I kanal
					readline(input_file, L);
					read(L, r);
					s := to_sfixed(r, s'high, s'low);

					-- Izlazi (ako su xi/xq tipa std_logic_vector)
					x <= to_slv(s);

					-- Ako su xi/xq sfixed tipa, umesto prethodne dve linije uradi:
					-- xi <= s_i; 
					-- xq <= s_q;

					out_ready <= '1';
				else
					out_ready <= '0';
					report "Kraj jednog od fajlova - simulacija se zaustavlja." severity note;
					std.env.stop;  -- VHDL-2008
				end if;
			end if;
		end if;
	end process;


	write_files : process(clk0)
		variable L, L_0, L_1 : line;
	begin
		if falling_edge(clk0) then
			if out_ready = '1' then
				-- upis I izlaza
				write(L, to_integer(signed(y)));
				writeline(output_file, L);
				
				write(L_1, to_integer(signed(y0)));
				writeline(output_file_0, L_1);
				
				write(L_0, to_integer(signed(y1)));
				writeline(output_file_1, L_0);
			end if;
		end if;
	end process;


end architecture;
