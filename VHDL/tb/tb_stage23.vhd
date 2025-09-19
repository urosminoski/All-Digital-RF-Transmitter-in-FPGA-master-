library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

library std;
use std.textio.all;

entity tb_stage23 is
end entity;

architecture tb of tb_stage23 is

	constant C_CLK1_PERIOD 	: time    := 1280 ps;
	constant C_CLK2_PERIOD 	: time    := 40 ps;
	constant C_CLK3_PERIOD 	: time    := 10	ps;
	
	constant DS_WIDTH		: integer := 24;
	constant OSR_WIDTH		: integer := 24;
	constant OSR_COEFF		: integer := 17;
	constant OSR_GUARD_BITS : integer := 4;
	constant LUT_ID 		: integer := 3;

	signal clk1   				: std_logic := '1';
	signal clk2   				: std_logic := '1';
	signal clk3   				: std_logic := '1';
	signal rst      			: std_logic := '1';
	signal xin_i        		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xin_q        		: std_logic_vector(OSR_WIDTH-1 downto 0) := (others => '0');
	signal xout_i_ds_test		: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_q_ds_test		: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_i_stage2_test	: std_logic := '0';
	signal xout_q_stage2_test	: std_logic := '0';
	signal xout_iq_test			: std_logic := '0';
	
	signal xin_i_ds 		: std_logic_vector(DS_WIDTH-1 downto 0) := (others => '0');
	signal xin_q_ds 		: std_logic_vector(DS_WIDTH-1 downto 0) := (others => '0');
	signal xout_i_ds 		: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_q_ds 		: std_logic_vector(3 downto 0) := (others => '0');
	
	signal xin_i_stage2		: std_logic_vector(3 downto 0) := (others => '0');
	signal xin_q_stage2		: std_logic_vector(3 downto 0) := (others => '0');
	signal xout_i_stage2	: std_logic := '0';
	signal xout_q_stage2	: std_logic := '0';
	
	signal xin_i_stage3		: std_logic := '0';
	signal xin_q_stage3		: std_logic := '0';
	signal xout_stage3		: std_logic := '0';
	
	signal stage2_strobe 	: std_logic := '0';
	signal stage3_strobe 	: std_logic := '0';

	file input_file_i  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xin_i_delay.txt";
	file input_file_q  	: text open read_mode   is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xin_q_delay.txt";
	
	file output_file_i_ds  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_i_ds.txt";
	file output_file_q_ds  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_q_ds.txt";
	
	file output_file_i_stage2  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_i_stage2.txt";
	file output_file_q_stage2  	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_q_stage2.txt";
	
	file output_file_stage3 	: text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_i_stage3.txt";
	
begin

	clk1 <= not clk1 after C_CLK1_PERIOD/2;
	clk2 <= not clk2 after C_CLK2_PERIOD/2;
	clk3 <= not clk3 after C_CLK3_PERIOD/2;
	rst <= '0' after 6*C_CLK1_PERIOD;
	
	
	-------------------------------------------------------------------------------
	-- Delta-Sigma Modulation
	-------------------------------------------------------------------------------
	
	xin_i_ds <= to_slv(
		resize(
			to_sfixed(4.0, 3, 0) * to_sfixed(xin_i, 0, -(OSR_WIDTH-1)),
			3,
			-(DS_WIDTH-4)
		)
	);
	
	xin_q_ds <= to_slv(
		resize(
			to_sfixed(4.0, 3, 0) * to_sfixed(xin_q, 0, -(OSR_WIDTH-1)),
			3,
			-(DS_WIDTH-4)
		)
	);
	
	deltaSigma_i: entity work.deltaSigma
		generic map ( XWIDTH => DS_WIDTH )
		port map (
			clk		=> clk1,
			rst 	=> rst,
			x 		=> xin_i_ds,
			y		=> xout_i_ds
		);
		
	deltaSigma_q: entity work.deltaSigma
		generic map ( XWIDTH => DS_WIDTH )
		port map (
			clk		=> clk1,
			rst 	=> rst,
			x 		=> xin_q_ds,
			y		=> xout_q_ds
		);
	
	-------------------------------------------------------------------------------
	-- Stage 2
	-------------------------------------------------------------------------------

	cdc01 : entity work.cdc
		port map (
			rst 		=> rst,
			clk_slow	=> clk1,
		    clk_fast	=> clk2,
			strobe		=> stage2_strobe
		);
	
	process(clk1)
	begin
		if rising_edge(clk1) then
			if rst = '1' then
				xin_i_stage2 	<= (others => '0');
				xin_q_stage2 	<= (others => '0');
			elsif stage2_strobe = '1' then
				xin_i_stage2	<= xout_i_ds;
				xin_q_stage2 	<= xout_q_ds;
			end if;
		end if;
	end process;
	
	stage2_gen : entity work.stage2
		generic map (
			LUT_ID		=> LUT_ID,
			XWIDTH		=> 4
		)
		port map (
			clk   	=> clk2,
			rst   	=> rst,
			strobe 	=> stage2_strobe,
			xin_i  	=> xin_i_stage2,
			xin_q  	=> xin_i_stage2,
			xout_i	=> xout_i_stage2,
			xout_q	=> xout_q_stage2
		);
	
	-------------------------------------------------------------------------------
	-- Stage 3
	-------------------------------------------------------------------------------
	
	cdc23 : entity work.cdc
		port map (
			rst 		=> rst,
			clk_slow	=> clk2,
		    clk_fast	=> clk3,
			strobe		=> stage3_strobe
		);
		
	process(clk3)
	begin
		if rising_edge(clk3) then
			if rst = '1' then
				xin_i_stage3 <= '0';
				xin_q_stage3 <= '0';
			elsif stage3_strobe = '1' then
				xin_i_stage3 <= xout_i_stage2;
				xin_q_stage3 <= xout_q_stage2;
			end if;
		end if;
	end process;
	
	stage3_gen : entity work.stage3
		port map (
			clk   	=> clk3,
			rst   	=> rst,
			strobe	=> stage3_strobe,
			xin_i  	=> xin_i_stage3,
			xin_q  	=> xin_q_stage3,
			xout_iq	=> xout_stage3
		);
		
	xout_i_ds_test <= xout_i_ds;
	xout_q_ds_test <= xout_q_ds;
	
	xout_i_stage2_test	<= xout_i_stage2;
	xout_q_stage2_test	<= xout_q_stage2;
	
	xout_iq_test <= xout_stage3;
	
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
	
	-------------------------------------------------------------------------------
	-- Write Output Data
	-------------------------------------------------------------------------------
	
	write_ds : process(clk2)
		variable L_i, L_q : line;
		variable L_i_test, L_q_test : line;
	begin
		if falling_edge(clk1) then
			if rst = '0' then
				write(L_i, to_integer(signed(xout_i_ds_test)));
				write(L_q, to_integer(signed(xout_q_ds_test)));
				writeline(output_file_i_ds, L_i);
				writeline(output_file_q_ds, L_q);
			end if;
		end if;
	end process;
	
	write_stage2 : process(clk1)
		variable L_i, L_q : line;
		variable L_i_test, L_q_test : line;
	begin
		if falling_edge(clk1) then
			if rst = '0' then
				write(L_i, xout_i_stage2_test);
				write(L_q, xout_q_stage2_test);
				writeline(output_file_i_stage2, L_i);
				writeline(output_file_q_stage2, L_q);
			end if;
		end if;
	end process;
	
	write_stage3 : process(clk1)
		variable L : line;
	begin
		if falling_edge(clk1) then
			if rst = '0' then
				write(L, xout_iq_test);
				writeline(output_file_stage3, L);
			end if;
		end if;
	end process;
	
end architecture;




				

