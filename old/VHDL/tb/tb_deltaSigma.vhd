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
	signal xi         : std_logic_vector(11 downto 0) := (others => '0');
	signal yi         : std_logic_vector(3 downto 0)  := (others => '0');
	signal xq         : std_logic_vector(11 downto 0) := (others => '0');
	signal yq         : std_logic_vector(3 downto 0)  := (others => '0');
	-- signal x : sfixed(3 downto -8);
	-- signal y : sfixed(3 downto 0);

	-- Lokalni “handshake” u TB:
	signal out_ready : std_logic := '0';

	file input_file_i  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\deltaSigma_test\xin_i_test.txt";
	file input_file_q  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\deltaSigma_test\xin_q_test.txt";
	file output_file_i 	: text open write_mode is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\deltaSigma_test\xout_i_test.txt";
	file output_file_q 	: text open write_mode is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\deltaSigma_test\xout_q_test.txt";

begin
	uut_i: entity work.deltaSigma
		port map (
			clk => clk,
			rst => rst,
			x   => xi,
			y   => yi
		);
	
	uut_q: entity work.deltaSigma
		port map (
			clk => clk,
			rst => rst,
			x   => xq,
			y   => yq
		);

	clk <= not clk after C_CLK_PERIOD/2;
	rst <= '0' after 6*C_CLK_PERIOD;
	
	read_files : process(clk)
		variable L_i, L_q : line;
		variable r_i, r_q : real;
		variable s_i, s_q : sfixed(3 downto -8);
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xi        <= (others => '0');
				xq        <= (others => '0');
				out_ready <= '0';
			else
				-- Čitamo paralelno: zaustavi kad ijedan fajl dođe do kraja
				if (not endfile(input_file_i)) and (not endfile(input_file_q)) then
					-- I kanal
					readline(input_file_i, L_i);
					read(L_i, r_i);
					s_i := to_sfixed(r_i, s_i'high, s_i'low);

					-- Q kanal
					readline(input_file_q, L_q);
					read(L_q, r_q);
					s_q := to_sfixed(r_q, s_q'high, s_q'low);

					-- Izlazi (ako su xi/xq tipa std_logic_vector)
					xi <= to_slv(s_i);
					xq <= to_slv(s_q);

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


	write_files : process(clk)
		variable L_i, L_q : line;
	begin
		if falling_edge(clk) then
			if out_ready = '1' then
				-- upis I izlaza
				write(L_i, to_integer(signed(yi)));
				writeline(output_file_i, L_i);

				-- upis Q izlaza
				write(L_q, to_integer(signed(yq)));
				writeline(output_file_q, L_q);
			end if;
		end if;
	end process;


end architecture;
