
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
	-- constant C_CLK_FREQ   	: integer := 150_000_000;
	constant C_CLK0_PERIOD 	: time    := 10240 ps;
	constant C_CLK1_PERIOD 	: time    := 1280 ps;
	constant C_CLK2_PERIOD 	: time    := 40 ps;
	constant C_CLK3_PERIOD 	: time    := 10	ps;
	
	constant LUT_ID			: integer := 3;
	constant KERNEL_ID		: integer := 7;
	constant DS_WIDTH		: integer := 16;
	constant OSR_WIDTH		: integer := 16;
	constant OSR_COEFF		: integer := 15;
	constant OSR_GUARD_BITS	: integer := 4;

	signal clk0   			: std_logic := '1';
	signal clk1   			: std_logic := '1';
	signal clk2   			: std_logic := '1';
	signal clk3  			: std_logic := '1';
	signal rst      		: std_logic := '1';
	signal xin_i        	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xin_q        	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_i_stage1 	: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_q_stage1 	: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_i_stage2	: std_logic := '0';
	signal xout_q_stage2	: std_logic := '0';
	signal xout_stage3		: std_logic := '0';

	file input_file_i  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xin_i_test.txt";
	file input_file_q 	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xin_q_test.txt";
	
	file output_file_i_stage1  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xout_i_stage1.txt";
	file output_file_q_stage1  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xout_q_stage1.txt";
	
	file output_file_i_stage2  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xout_i_stage2.txt";
	file output_file_q_stage2  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xout_q_stage2.txt";
	
	file output_file_stage3  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\RFTransmitter\VHDL\data\rfTransmitter_test\xout_stage3.txt";
	
begin

	clk0 <= not clk0 after C_CLK0_PERIOD/2;
	clk1 <= not clk1 after C_CLK1_PERIOD/2;
	clk2 <= not clk2 after C_CLK2_PERIOD/2;
	clk3 <= not clk3 after C_CLK3_PERIOD/2;
	rst <= '0' after 6*C_CLK0_PERIOD;
	
	uut_i: entity work.rfTransmitter
		generic map (
			LUT_ID 			=> LUT_ID,
			KERNEL_ID		=> KERNEL_ID,
			DS_WIDTH		=> DS_WIDTH,
			OSR_WIDTH		=> OSR_WIDTH,
			OSR_COEFF  		=> OSR_COEFF,
			OSR_GUARD_BITS 	=> OSR_GUARD_BITS
		)
		port map (
			clk0 			=> clk0,
			clk1 			=> clk1,
			clk2 			=> clk2,
			clk3 			=> clk3,
			rst 			=> rst,
			xin_i   		=> xin_i,
			xin_q   		=> xin_q,
			xout_i_stage1 	=> xout_i_stage1,
			xout_q_stage1 	=> xout_q_stage1,
			xout_i_stage2 	=> xout_i_stage2,
			xout_q_stage2 	=> xout_q_stage2,
			xout_stage3 	=> xout_stage3
		);
	
	read_files : process(clk0)
		variable L_i, L_q : line;
		variable r_i, r_q : real;
		variable s_i, s_q : sfixed(0 downto -(OSR_WIDTH-1));
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				xin_i   	<= (others => '0');
				xin_q   	<= (others => '0');
			else
				if (not endfile(input_file_i)) and (not endfile(input_file_q)) then
					readline(input_file_i, L_i);
					read(L_i, r_i);
					s_i := to_sfixed(r_i, s_i'high, s_i'low);

					readline(input_file_q, L_q);
					read(L_q, r_q);
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
	
	write_stage1 : process(clk1)
		variable L_i, L_q : line;
		variable L_i_test, L_q_test : line;
	begin
		if falling_edge(clk1) then
			if rst = '0' then
				write(L_i, to_integer(signed(xout_i_stage1)));
				write(L_q, to_integer(signed(xout_q_stage1)));
				writeline(output_file_i_stage1, L_i);
				writeline(output_file_q_stage1, L_q);
			end if;
		end if;
	end process;

	write_stage2 : process(clk2)
		variable L_i, L_q : line;
	begin
		if falling_edge(clk2) then
			if rst = '0' then
				write(L_i, xout_i_stage2);
				write(L_q, xout_q_stage2);
				writeline(output_file_i_stage2, L_i);
				writeline(output_file_q_stage2, L_q);
			end if;
		end if;
	end process;
	
	write_stage3 : process(clk3)
		variable L : line;
	begin
		if falling_edge(clk3) then
			if rst = '0' then
				write(L, xout_stage3);
				writeline(output_file_stage3, L);
			end if;
		end if;
	end process;

end architecture;


	write_stage2 : process(clk2)
		variable L_i, L_q : line;
	begin
		if falling_edge(clk2) then
			if rst = '0' then
				write(L_i, xout_i_stage2);
				write(L_q, xout_q_stage2);
				writeline(output_file_i_stage2, L_i);
				writeline(output_file_q_stage2, L_q);
			end if;
		end if;
	end process;
	
	write_stage3 : process(clk3)
		variable L : line;
	begin
		if falling_edge(clk3) then
			if rst = '0' then
				write(L, xout_stage3);
				writeline(output_file_stage3, L);
			end if;
		end if;
	end process;

end architecture;