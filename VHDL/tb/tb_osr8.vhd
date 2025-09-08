library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_osr8 is
end entity;

architecture tb of tb_osr8 is
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;
	
	constant XWIDTH		: integer := 12;
	constant COEF_L		: integer := 15;
	constant INT 		: integer := 0;
	constant FRAC 		: integer := XWIDTH + COEF_L;

	signal clk   		: std_logic := '0';
	signal rst      	: std_logic := '1';
	signal xin_i        : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xin_q        : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout_i       : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout_q       : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal vout_i 		: std_logic := '0';
	signal vout_q 		: std_logic := '0';
	
	signal out_ready : std_logic := '0';

	file input_file_i  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xin_i_test.txt";
	file input_file_q  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xin_q_test.txt";
	file output_file_i  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xout_i_test.txt";
	file output_file_q  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\interpolation_test\xout_q_test.txt";

begin
	uut_i: entity work.osr8
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			xin   	=> xin_i,
			xout   	=> xout_i,
			vout 	=> vout_i
		);
		
	uut_q: entity work.osr8
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			xin   	=> xin_q,
			xout   	=> xout_q,
			vout 	=> vout_q
		);

	clk <= not clk after C_CLK_PERIOD/2;
	
	rst <= '0' after 6*C_CLK_PERIOD;
	
	read_files : process(clk)
		variable L_i, L_q : line;
		variable r_i, r_q : real;
		variable s_i, s_q : sfixed(0 downto -(XWIDTH-1));
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xin_i   	<= (others => '0');
				xin_q   	<= (others => '0');
				out_ready 	<= '0';
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
					xin_i <= to_slv(s_i);
					xin_q <= to_slv(s_q);

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
		if rising_edge(clk) then
			if out_ready = '1' then
				-- upis I izlaza
				write(L_i, to_integer(signed(xout_i)));
				writeline(output_file_i, L_i);

				-- upis Q izlaza
				write(L_q, to_integer(signed(xout_q)));
				writeline(output_file_q, L_q);
			end if;
		end if;
	end process;


end architecture;
