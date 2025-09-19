library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_delay is
end entity;

architecture tb of tb_delay is

	constant C_CLK1_PERIOD 	: time    := 1280 ps;
	
	constant DELTA 		: real := 0.5;--0.00390625;
	constant DELTA_I 	: real := -DELTA;
	constant DELTA_Q 	: real := DELTA;
	constant factor		: real := 1.0/(1.0+DELTA);
	
	constant DS_WIDTH		: integer := 32;
	constant OSR_WIDTH		: integer := 32;
	constant OSR_COEFF		: integer := 31;
	constant OSR_GUARD_BITS : integer := 4;
	constant LUT_ID 		: integer := 3;
	constant KERNEL_ID		: integer := 7;

	signal clk1   				: std_logic := '1';
	signal rst      			: std_logic := '1';
	signal xin_i        		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xin_q        		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	
	signal xout_i_delay_test	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_q_delay_test	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	
	signal xin_i_delay	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xin_q_delay	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_i_delay	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_q_delay	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');

	file input_file_i  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xin_i_test.txt";
	file input_file_q  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xin_q_test.txt";
	
	file output_file_i_delay  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_i_delay.txt";
	file output_file_q_delay  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_q_delay.txt";

begin

	clk1 <= not clk1 after C_CLK1_PERIOD/2;
	rst <= '0' after 6*C_CLK1_PERIOD;
	
	xin_i_delay <= to_slv(
		resize(
			to_sfixed(factor, 1, -10) * to_sfixed(xin_i, 0, -(OSR_WIDTH-1)),
			0,
			-(OSR_WIDTH-1)
		)
	);
	
	xin_q_delay <= to_slv(
		resize(
			to_sfixed(factor, 1, -10) * to_sfixed(xin_q, 0, -(OSR_WIDTH-1)),
			0,
			-(OSR_WIDTH-1)
		)
	);
	
	delay_i: entity work.delay
		generic map (
			KERNEL_ID   => KERNEL_ID,
			COEF_L		=> OSR_COEFF,
			XWIDTH		=> OSR_WIDTH,
			INT  		=> 3,
			GUARD_BITS	=> 4,
			DELTA		=> DELTA_I
		)
		port map (
			clk		=> clk1,
			rst		=> rst, 
			en		=> '1', 		
			xin		=> xin_i_delay,
			xout	=> xout_i_delay      
		);
		
	delay_q: entity work.delay
		generic map (
			KERNEL_ID   => KERNEL_ID,
			COEF_L		=> OSR_COEFF,
			XWIDTH		=> OSR_WIDTH,
			INT  		=> 3,
			GUARD_BITS	=> 4,
			DELTA		=> DELTA_Q
		)
		port map (
			clk		=> clk1,
			rst		=> rst, 
			en		=> '1', 		
			xin		=> xin_q_delay,
			xout	=> xout_q_delay      
		);
		
	xout_i_delay_test <= xout_i_delay;
	xout_q_delay_test <= xout_q_delay;
	
	-------------------------------------------------------------------------------
	-- Read input Data
	-------------------------------------------------------------------------------
	
	read_files : process(clk1)
		variable L_i, L_q : line;
		variable r_i, r_q : real;
		variable s_i, s_q : sfixed(0 downto -(OSR_WIDTH-1));
	begin
		if rising_edge(clk1) then
			if rst = '1' then
				xin_i   	<= (others => '0');
				xin_q   	<= (others => '0');
			else
				if (not endfile(input_file_i)) and (not endfile(input_file_q)) then
					readline(input_file_i, L_i);
					readline(input_file_q, L_q);
					read(L_i, r_i);
					read(L_q, r_q);
					s_i := to_sfixed(r_i, s_i'high, s_i'low);
					s_q := to_sfixed(r_q, s_q'high, s_q'low);

					xin_i <= to_slv(s_i);
					xin_q <= to_slv(s_q);
				else
					report "Kraj jednog od fajlova - simulacija se zaustavlja." severity note;
					std.env.stop;  -- VHDL-2008
				end if;
			end if;
		end if;
	end process;
	
	-------------------------------------------------------------------------------
	-- Write Output Data
	-------------------------------------------------------------------------------
	
	write_delay : process(clk1)
		variable L_i, L_q : line;
		variable L_i_test, L_q_test : line;
	begin
		if falling_edge(clk1) then
			if rst = '0' then
				write(L_i, to_integer(signed(xout_i_delay_test)));
				write(L_q, to_integer(signed(xout_q_delay_test)));
				writeline(output_file_i_delay, L_i);
				writeline(output_file_q_delay, L_q);
			end if;
		end if;
	end process;
	
end architecture;




				

