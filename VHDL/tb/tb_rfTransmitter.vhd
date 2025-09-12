library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_rfTransmitter is
end entity;

architecture tb of tb_rfTransmitter is
	constant C_CLK_FREQ   	: integer := 150_000_000;
	constant C_CLK1_PERIOD 	: time    := 1 sec / C_CLK_FREQ;
	constant C_CLK2_PERIOD 	: time    := C_CLK1_PERIOD/32;
	
	constant XWIDTH		: integer := 12;
	constant COEF_L		: integer := 15;
	constant INT 		: integer := 0;
	constant FRAC 		: integer := XWIDTH + COEF_L;

	signal clk1   		: std_logic := '0';
	signal clk2   		: std_logic := '0';
	signal rst      	: std_logic := '1';
	signal xin_i        : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xin_q        : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	-- signal xout_i       : std_logic_vector(3 downto 0) := (others => '0');
	-- signal xout_q       : std_logic_vector(3 downto 0) := (others => '0');
	signal xout_i		: std_logic := '0';
	signal xout_q		: std_logic := '0';
	
	signal tb_cnt 		: unsigned(2 downto 0) := (others => '0');
	signal out_ready 	: std_logic := '0';

	file input_file_i  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xin_i_test.txt";
	file input_file_q  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xin_q_test.txt";
	file output_file_i  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xout_i_test.txt";
	file output_file_q  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\rfTransmitter_test\xout_q_test.txt";

begin
	uut_i: entity work.rfTransmitter
		generic map (
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC
		)
		port map (
			clk1 	=> clk1,
			clk2 	=> clk2,
			rst 	=> rst,
			xin_i   => xin_i,
			xin_q   => xin_q,
			xout_i	=> xout_i,
			xout_q	=> xout_q
		);

	clk1 <= not clk1 after C_CLK1_PERIOD/2;
	clk2 <= not clk2 after C_CLK2_PERIOD/2;
	
	
	rst <= '0' after 6*C_CLK1_PERIOD;
	
	process(clk1)
	begin
		if rising_edge(clk1) then
			if rst='1' then
				tb_cnt <= (others => '0');
			else
				tb_cnt <= tb_cnt + 1;
			end if;
		end if;
	end process;
	
	read_files : process(clk1)
		variable L_i, L_q : line;
		variable r_i, r_q : real;
		variable s_i, s_q : sfixed(0 downto -(XWIDTH-1));
	begin
		if rising_edge(clk1) then
			if rst = '1' then
				xin_i   	<= (others => '0');
				xin_q   	<= (others => '0');
				out_ready 	<= '0';
			elsif tb_cnt = "000" then
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

					out_ready <= '1';
				else
					out_ready <= '0';
					report "Kraj jednog od fajlova - simulacija se zaustavlja." severity note;
					std.env.stop;  -- VHDL-2008
				end if;
			end if;
		end if;
	end process;

	write_files : process(clk2)
		variable L_i, L_q : line;
	begin
		if rising_edge(clk2) then
			if rst = '0' then
				-- upis I izlaza
				-- write(L_i, to_integer(signed(xout_i)));
				write(L_i, xout_i);
				writeline(output_file_i, L_i);

				-- upis Q izlaza
				-- write(L_q, to_integer(signed(xout_q)));
				write(L_q, xout_q);
				writeline(output_file_q, L_q);
			end if;
		end if;
	end process;


end architecture;
