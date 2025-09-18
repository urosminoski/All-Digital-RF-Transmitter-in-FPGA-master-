library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_stage1 is
end entity;

architecture tb of tb_stage1 is

	constant C_CLK0_PERIOD 	: time    := 10240 ps;
	constant C_CLK1_PERIOD 	: time    := 1280 ps;
	
	constant DS_WIDTH		: integer := 12;
	constant OSR_WIDTH		: integer := 16;
	constant OSR_COEFF		: integer := 15;
	constant OSR_GUARD_BITS : integer := 4;

	signal clk0   				: std_logic := '1';
	signal clk1   				: std_logic := '1';
	signal stage1_strobe   		: std_logic := '1';
	signal rst      			: std_logic := '1';
	signal xin_i        		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xin_q        		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_i_osr8_test		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_q_osr8_test		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_i_delay_test	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_q_delay_test	: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_i				: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_q				: std_logic_vector(3 downto 0) := (others => '0');
	
	signal xin_i_stage1		: std_logic_vector(xin_i'high downto xin_i'low);
	signal xin_q_stage1		: std_logic_vector(xin_q'high downto xin_q'low);
		   
	constant Ncnt 		: integer := 8;
	signal tb_cnt 		: integer := 0;

	file input_file_i  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xin_i_test.txt";
	file input_file_q  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xin_q_test.txt";
	
	file output_file_i_osr8  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xout_i_osr8.txt";
	file output_file_q_osr8  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xout_q_osr8.txt";
	
	file output_file_i_delay  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xout_i_delay.txt";
	file output_file_q_delay  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xout_q_delay.txt";
	
	file output_file_i  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xout_i.txt";
	file output_file_q  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\stage1_test\xout_q.txt";

begin

	clk0 <= not clk0 after C_CLK0_PERIOD/2;
	clk1 <= not clk1 after C_CLK1_PERIOD/2;
	rst <= '0' after 6*C_CLK0_PERIOD;

	cdc01 : entity work.cdc
		port map (
			rst 		=> rst,
			clk_slow	=> clk0,
		    clk_fast	=> clk1,
			strobe		=> stage1_strobe
		);
	
	process(clk1)
	begin
		if rising_edge(clk1) then
			if rst = '1' then
				xin_i_stage1 	<= (others => '0');
				xin_q_stage1 	<= (others => '0');
			elsif stage1_strobe = '1' then
				xin_i_stage1 	<= xin_i;
				xin_q_stage1 	<= xin_q;
			end if;
		end if;
	end process;
	
	uut_i: entity work.stage1
		generic map (
			DS_WIDTH		=> DS_WIDTH,
			OSR_WIDTH		=> OSR_WIDTH,
			OSR_COEFF		=> OSR_COEFF,
			OSR_GUARD_BITS 	=> OSR_GUARD_BITS
		)
		port map (
			clk 				=> clk1,
			rst 				=> rst,
			strobe				=> stage1_strobe,
			xin_i   			=> xin_i_stage1,
			xin_q   			=> xin_q_stage1,
			xout_i_osr8_test 	=> xout_i_osr8_test,
			xout_q_osr8_test 	=> xout_q_osr8_test,
			xout_i_delay_test	=> xout_i_delay_test,
			xout_q_delay_test	=> xout_q_delay_test,
			xout_i 				=> xout_i,
			xout_q 				=> xout_q
		);
	
	-- process(clk)
	-- begin
		-- if rising_edge(clk) then
			-- if rst = '1' then
				-- tb_cnt <= 0;
			-- else
				-- if tb_cnt = 7 then
					-- tb_cnt <= 0;
				-- else
					-- tb_cnt <= tb_cnt + 1;
				-- end if;
			-- end if;
		-- end if;
	-- end process;
	
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
				write(L_i, to_integer(signed(xout_i_osr8_test)));
				write(L_q, to_integer(signed(xout_q_osr8_test)));
				writeline(output_file_i_osr8, L_i);
				writeline(output_file_q_osr8, L_q);
				
				write(L_i, to_integer(signed(xout_i_delay_test)));
				write(L_q, to_integer(signed(xout_i_delay_test)));
				writeline(output_file_i_delay, L_i);
				writeline(output_file_q_delay, L_q);
				
				write(L_i, to_integer(signed(xout_i)));
				write(L_q, to_integer(signed(xout_q)));
				writeline(output_file_i, L_i);
				writeline(output_file_q, L_q);
			end if;
		end if;
	end process;

end architecture;
