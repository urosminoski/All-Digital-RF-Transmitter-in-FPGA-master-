library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_lut_serializer is
end entity;

architecture tb of tb_lut_serializer is
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;
	
	constant LUT_ID		: integer := 3;
	constant XWIDTH		: integer := 4;
	
	constant N : integer := 32;

	signal clk   		: std_logic := '0';
	signal rst      	: std_logic := '1';
	signal enable 		: std_logic := '0';
	signal xin        	: std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout       	: std_logic := '0';
	signal sym_tick		: std_logic := '0';
	
	signal tb_cnt 		: integer := 0;
	signal out_ready 	: std_logic := '0';

	file input_file  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\lut_serializer_test\xin_test.txt";
	file output_file  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\lut_serializer_test\xout_test.txt";

begin
	uut: entity work.lut_serializer
		generic map (
			LUT_ID		=> LUT_ID,
			XWIDTH		=> XWIDTH
		)
		port map (
			clk   		=> clk,
			rst   		=> rst,
			enable		=> enable,
			xin  		=> xin,
			xout		=> xout,
			sym_tick	=> sym_tick		
		);
	
	clk <= not clk after C_CLK_PERIOD/2;
	
	rst <= '0' after 6*C_CLK_PERIOD;
	
	process(clk)
	begin
		if rising_edge(clk) then
			if rst='1' then
				tb_cnt <= 0;
			else
				if tb_cnt = N-1 then
					tb_cnt <= 0;
				else
					tb_cnt <= tb_cnt + 1;
				end if;
			end if;
		end if;
	end process;
		
	read_files : process(clk)
		variable L : line;
		variable r : integer;
		variable s : sfixed(XWIDTH-1 downto 0);
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xin   		<= (others => '0');
				out_ready 	<= '0';
				enable		<= '0';
			elsif tb_cnt = N-1 then
				-- Čitamo paralelno: zaustavi kad ijedan fajl dođe do kraja
				if (not endfile(input_file)) then--and (not endfile(input_file_q)) then
					-- I kanal
					readline(input_file, L);
					read(L, r);
					s := to_sfixed(r, s'high, s'low);
					
					xin <= to_slv(s);

					out_ready	<= '1';
					enable		<= '1';
				else
					out_ready 	<= '0';
					enable		<= '0';
					report "Kraj jednog od fajlova - simulacija se zaustavlja." severity note;
					std.env.stop;  -- VHDL-2008
				end if;
			else
				out_ready 	<= '0';
				enable 		<= '0';
			end if;
		end if;
	end process;

	write_files : process(clk)
		variable L : line;
	begin
		if rising_edge(clk) then
			if out_ready = '1' then
				-- upis I izlaza
				write(L, xout);
				writeline(output_file, L);
			end if;
		end if;
	end process;


end architecture;
