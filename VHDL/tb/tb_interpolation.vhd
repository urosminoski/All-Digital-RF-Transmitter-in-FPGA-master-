library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_interpolation is
end entity;

architecture tb of tb_interpolation is
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;
	
	constant XWIDTH		: integer := 12;
	constant COEF_L		: integer := 15;
	constant INT 		: integer := 0;
	constant FRAC 		: integer := XWIDTH + COEF_L;

	signal clk   		: std_logic := '0';
	signal rst      	: std_logic := '1';
	signal xin        	: std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout       	: std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	
	signal tb_cnt 		: unsigned(2 downto 0) := (others => '0');

	signal out_ready : std_logic := '0';

	file input_file  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xin_test.txt";
	file output_file  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xout_test.txt";

begin
	uut: entity work.osr8
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			xin   	=> xin,
			xout   	=> xout
		);

	clk <= not clk after C_CLK_PERIOD/2;
	
	rst <= '0' after 6*C_CLK_PERIOD;
	
	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				tb_cnt <= (others => '0');
			else
				tb_cnt <= tb_cnt + 1;
			end if;
		end if;
	end process;
	
	read_files : process(clk)
		variable L : line;
		variable r : real;
		variable s : sfixed(0 downto -(XWIDTH-1));
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xin        	<= (others => '0');
				out_ready 	<= '0';
			elsif tb_cnt = "000" then
				-- Čitamo paralelno: zaustavi kad ijedan fajl dođe do kraja
				if (not endfile(input_file))then
					-- I kanal
					readline(input_file, L);
					read(L, r);
					s := to_sfixed(r, s'high, s'low);
					xin <= to_slv(s);
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
		variable L : line;
	begin
		if falling_edge(clk) then
			if out_ready = '1' then
				-- upis I izlaza
				write(L, to_integer(signed(xout)));
				writeline(output_file, L);
			end if;
		end if;
	end process;


end architecture;
