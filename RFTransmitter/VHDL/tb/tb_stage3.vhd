library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_stage3 is
end entity;

architecture tb of tb_stage3 is
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;

	signal clk   		: std_logic := '0';
	signal rst      	: std_logic := '1';
	signal xin_i        : std_logic := '0';
	signal xin_q        : std_logic := '0';
	signal xout       	: std_logic := '0';
	
	constant N			: integer := 4;
	signal tb_cnt 		: integer := 0;
	signal out_ready 	: std_logic := '0';
	
	signal strobe : std_logic := '0';

	file input_file_i  			: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xout_i_stage2.txt";
	file input_file_q  			: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xout_q_stage2.txt";
	file output_file  			: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xout_stage3.txt";

begin
	stage3_gen : entity work.stage3
		port map (
			clk   	=> clk,
			rst   	=> rst,
			strobe	=> strobe,
			xin_i  	=> xin_i,
			xin_q  	=> xin_q,
			xout_iq	=> xout 
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
	
	strobe <= '1' when (tb_cnt = 0) else '0';
	
	read_files : process(clk)
		variable L_i, L_q : line;
		variable r_i, r_q : std_logic := '0';
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xin_i   	<= '0';
				xin_q   	<= '0';
				out_ready 	<= '0';
			elsif tb_cnt = 0 then
				-- Čitamo paralelno: zaustavi kad ijedan fajl dođe do kraja
				if (not endfile(input_file_i)) and (not endfile(input_file_q)) then
					-- I kanal
					readline(input_file_i, L_i);
					read(L_i, r_i);

					-- Q kanal
					readline(input_file_q, L_q);
					read(L_q, r_q);


					-- Izlazi (ako su xi/xq tipa std_logic_vector)
					xin_i <= r_i;
					xin_q <= r_q;

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
		variable L : line;
	begin
		if rising_edge(clk) then
			if out_ready = '1' then

				-- upis Q izlaza
				write(L, xout);
				writeline(output_file, L);
			end if;
		end if;
	end process;


end architecture;
