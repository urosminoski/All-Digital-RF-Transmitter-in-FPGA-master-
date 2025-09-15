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
	constant C_CLK_FREQ   : integer := 150_000_000;
	constant C_CLK_PERIOD : time    := 1 sec / C_CLK_FREQ;
	
	constant KERNEL_ID	: integer := 7;
	constant XWIDTH		: integer := 12;
	constant COEF_L		: integer := 17;
	constant INT 		: integer := 0;
	constant FRAC 		: integer := XWIDTH + COEF_L;
	constant NUM_TAPS	: integer := 7;
	
	constant DELTA 		: real := 0.00390625;
	constant DELTA_I 	: real := -DELTA;
	constant DELTA_Q 	: real := DELTA;

	signal clk   		: std_logic := '0';
	signal rst      	: std_logic := '1';
	signal en 			: std_logic := '0';
	signal xin_i        : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xin_q        : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout_i       : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	signal xout_q       : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
	
	signal tb_cnt 		: unsigned(2 downto 0) := (others => '0');
	signal out_ready 	: std_logic := '0';

	file input_file_i  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xin_test.txt";
	-- file input_file_q  	: text open read_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xin_q_test.txt";
	file output_file_i  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_test.txt";
	-- file output_file_q  : text open write_mode  is "C:\Users\Korisnik\Desktop\FAKS\MASTER\All-Digital-RF-Transmitter-in-FPGA-master-\VHDL\data\delay_test\xout_q_test.txt";

begin
	uut_i: entity work.delay
		generic map (
			KERNEL_ID	=> KERNEL_ID,
			COEF_L		=> COEF_L,
			XWIDTH		=> XWIDTH,
			INT  		=> INT,
			FRAC 		=> FRAC,
			NUM_TAPS	=> NUM_TAPS,
			DELTA		=> DELTA_i
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			en 		=> '1',
			xin   	=> xin_i,
			xout	=> xout_i
		);
		
	-- uut_q: entity work.delay
		-- generic map (
			-- KERNEL_ID	=> KERNEL_ID,
			-- COEF_L		=> COEF_L,
			-- XWIDTH		=> XWIDTH,
			-- INT  		=> INT,
			-- FRAC 		=> FRAC,
			-- NUM_TAPS	=> NUM_TAPS,
			-- DELTA		=> DELTA_q
		-- )
		-- port map (
			-- clk 	=> clk,
			-- rst 	=> rst,
			-- en 		=> en,
			-- xin   	=> xin_q,
			-- xout	=> xout_q
		-- );
	
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
				if (not endfile(input_file_i)) then --and (not endfile(input_file_q)) then
					readline(input_file_i, L_i);
					read(L_i, r_i);
					s_i := to_sfixed(r_i, s_i'high, s_i'low);

					-- readline(input_file_q, L_q);
					-- read(L_q, r_q);
					-- s_q := to_sfixed(r_q, s_q'high, s_q'low);

					xin_i <= to_slv(s_i);
					-- xin_q <= to_slv(s_q);

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
				-- write(L_q, to_integer(signed(xout_q)));
				-- writeline(output_file_q, L_q);
			end if;
		end if;
	end process;


end architecture;
