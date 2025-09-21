library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;
use work.deltaSigma_pkg.all;

library ieee_proposed;
use ieee_proposed.math_utility_pkg.all;
use ieee_proposed.fixed_pkg.all;

entity deltaSigma_tb is
end entity;

architecture testbench of deltaSigma_tb is
	
	signal clk	: std_logic := '1';
	signal rst 	: std_logic := '1';
	signal x 	: sfixed(C_X_UPPER downto C_X_LOWER) := (others => '0');  -- Adjusted the size to match the entity port
	signal y 	: sfixed(C_Y_UPPER downto C_Y_LOWER);  -- Adjusted the size to match the entity port
	
	constant C_CLK_PERIOD : time := 1 ns;

	-- Define file names as constants
	constant INPUT_FILE_NAME 	: string 	:= ".\data\input_data.txt";
	constant OUTPUT_FILE_NAME 	: string 	:= ".\data\output_data.txt";
	
begin
	-- Instantiation of the delta_sigma entity
	I1 : entity work.deltaSigma
		port map(
			clk	=> clk,
			rst => rst,
			x 	=> x,
			y 	=> y
		);
	
	-- Clock generation
	clk <= not clk after C_CLK_PERIOD / 2;
	
	-- Reset and clock-driven process
	ALWAYS : process
	begin
		wait for C_CLK_PERIOD;
		rst <= '0';
		wait;
	end process;
	
	INPUT : process
		file input_file : TEXT OPEN READ_MODE is INPUT_FILE_NAME;
		variable line_buffer : LINE;
		variable real_value : REAL;
		variable i : integer := 0;
	begin
		wait until rising_edge(clk);  -- Synchronize with the clock

		if rst = '1' then
			i := 0;
		elsif not endfile(input_file) then
			-- Read the next line from the file
			readline(input_file, line_buffer);
			read(line_buffer, real_value);
			-- Convert the real number to sfixed and assign to x
			x <= to_sfixed(real_value, C_X_UPPER, C_X_LOWER);
			i := i + 1;
		else
			-- End the simulation when the file reaches the end
			report "End of input file reached. Stopping simulation.";
			wait for 1 ns;
			assert false report "Simulation finished" severity failure;
		end if;
	end process;
	
	-- Output process to write simulation results to a file
	OUTPUT : process(clk)
		file file_pointer 	: TEXT OPEN WRITE_MODE is OUTPUT_FILE_NAME;  -- Using constant for file name
		variable line_el 	: LINE;
		variable y_integer 	: INTEGER;
	begin
		if rising_edge(clk) then
			y_integer := to_integer(signed(y));  -- Converts 'y' to an integer
			write(line_el, y_integer);
			writeline(file_pointer, line_el);
		end if;
	end process;
	
end architecture;
