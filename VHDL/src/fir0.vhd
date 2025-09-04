library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

entity fir0 is
	generic(
		WIDTH_X		: integer := 12;
		WIDTH_Y 	: integer := 28
	);
	port(
		clk0, clk1, clk2 	: in std_logic;
		rst 				: in  std_logic;
		x      				: in  std_logic_vector(WIDTH_X-1 downto 0);
		y, y0, y1      		: out std_logic_vector(WIDTH_Y-1 downto 0)
	);
end entity;

architecture rtl of fir0 is

	constant N_FRAC 	: integer := -(WIDTH_Y-WIDTH_X-1);
	constant N_INT 		: integer := 1;
	constant N_FRAC_2 	: integer := -(WIDTH_Y-1-N_INT);

	constant N : integer := 42;
	type fir_array_t is array (0 to N-1) of sfixed(0 downto N_FRAC);

	-- FIR 0 Coeffitients
	constant fir_ph0 : fir_array_t := (
		to_sfixed( 0.000014291322, 0, N_FRAC),
		to_sfixed(-0.000039516421, 0, N_FRAC),
		to_sfixed( 0.000091072642, 0, N_FRAC),
		to_sfixed(-0.000183517720, 0, N_FRAC),
		to_sfixed( 0.000337020847, 0, N_FRAC),
		to_sfixed(-0.000578304995, 0, N_FRAC),
		to_sfixed( 0.000940386412, 0, N_FRAC),
		to_sfixed(-0.001464323420, 0, N_FRAC),
		to_sfixed( 0.002199199950, 0, N_FRAC),
		to_sfixed(-0.003204611510, 0, N_FRAC),
		to_sfixed( 0.004552420300, 0, N_FRAC),
		to_sfixed(-0.006332841260, 0, N_FRAC),
		to_sfixed( 0.008663313130, 0, N_FRAC),
		to_sfixed(-0.011708605900, 0, N_FRAC),
		to_sfixed( 0.015718495800, 0, N_FRAC),
		to_sfixed(-0.021111531700, 0, N_FRAC),
		to_sfixed( 0.028669391700, 0, N_FRAC),
		to_sfixed(-0.040069744000, 0, N_FRAC),
		to_sfixed( 0.059694815000, 0, N_FRAC),
		to_sfixed(-0.103678482000, 0, N_FRAC),
		to_sfixed( 0.317492818000, 0, N_FRAC),
		to_sfixed( 0.317492818000, 0, N_FRAC),
		to_sfixed(-0.103678482000, 0, N_FRAC),
		to_sfixed( 0.059694815000, 0, N_FRAC),
		to_sfixed(-0.040069744000, 0, N_FRAC),
		to_sfixed( 0.028669391700, 0, N_FRAC),
		to_sfixed(-0.021111531700, 0, N_FRAC),
		to_sfixed( 0.015718495800, 0, N_FRAC),
		to_sfixed(-0.011708605900, 0, N_FRAC),
		to_sfixed( 0.008663313130, 0, N_FRAC),
		to_sfixed(-0.006332841260, 0, N_FRAC),
		to_sfixed( 0.004552420300, 0, N_FRAC),
		to_sfixed(-0.003204611510, 0, N_FRAC),
		to_sfixed( 0.002199199950, 0, N_FRAC),
		to_sfixed(-0.001464323420, 0, N_FRAC),
		to_sfixed( 0.000940386412, 0, N_FRAC),
		to_sfixed(-0.000578304995, 0, N_FRAC),
		to_sfixed( 0.000337020847, 0, N_FRAC),
		to_sfixed(-0.000183517720, 0, N_FRAC),
		to_sfixed( 0.000091072642, 0, N_FRAC),
		to_sfixed(-0.000039516421, 0, N_FRAC),
		to_sfixed( 0.000014291322, 0, N_FRAC)
	);
	
	constant fir_ph1 : fir_array_t := (
		to_sfixed(0, 0, N_FRAC),
		to_sfixed(-0.000000125470, 0, N_FRAC),
		to_sfixed( 0.000000200697, 0, N_FRAC),
		to_sfixed(-0.000000503721, 0, N_FRAC),
		to_sfixed( 0.000000828311, 0, N_FRAC),
		to_sfixed(-0.000001458836, 0, N_FRAC),
		to_sfixed( 0.000002191536, 0, N_FRAC),
		to_sfixed(-0.000003306305, 0, N_FRAC),
		to_sfixed( 0.000004591620, 0, N_FRAC),
		to_sfixed(-0.000006307723, 0, N_FRAC),
		to_sfixed( 0.000008197802, 0, N_FRAC),
		to_sfixed(-0.000010484277, 0, N_FRAC),
		to_sfixed( 0.000012878196, 0, N_FRAC),
		to_sfixed(-0.000015551202, 0, N_FRAC),
		to_sfixed( 0.000018184220, 0, N_FRAC),
		to_sfixed(-0.000020897352, 0, N_FRAC),
		to_sfixed( 0.000023348090, 0, N_FRAC),
		to_sfixed(-0.000025659990, 0, N_FRAC),
		to_sfixed( 0.000027483840, 0, N_FRAC),
		to_sfixed(-0.000028956372, 0, N_FRAC),
		to_sfixed( 0.000029782526, 0, N_FRAC),
		to_sfixed( 0.499967636000, 0, N_FRAC),
		to_sfixed( 0.000029782526, 0, N_FRAC),
		to_sfixed(-0.000028956372, 0, N_FRAC),
		to_sfixed( 0.000027483840, 0, N_FRAC),
		to_sfixed(-0.000025659990, 0, N_FRAC),
		to_sfixed( 0.000023348090, 0, N_FRAC),
		to_sfixed(-0.000020897352, 0, N_FRAC),
		to_sfixed( 0.000018184220, 0, N_FRAC),
		to_sfixed(-0.000015551202, 0, N_FRAC),
		to_sfixed( 0.000012878196, 0, N_FRAC),
		to_sfixed(-0.000010484277, 0, N_FRAC),
		to_sfixed( 0.000008197802, 0, N_FRAC),
		to_sfixed(-0.000006307723, 0, N_FRAC),
		to_sfixed( 0.000004591620, 0, N_FRAC),
		to_sfixed(-0.000003306305, 0, N_FRAC),
		to_sfixed( 0.000002191536, 0, N_FRAC),
		to_sfixed(-0.000001458836, 0, N_FRAC),
		to_sfixed( 0.000000828311, 0, N_FRAC),
		to_sfixed(-0.000000503721, 0, N_FRAC),
		to_sfixed( 0.000000200697, 0, N_FRAC),
		to_sfixed(-0.000000125470, 0, N_FRAC)
	);
	
	type mul_array_t is array (0 to N-1) of sfixed(N_INT downto N_FRAC_2);
	type add_array_t is array (0 to N-1) of sfixed(N_INT downto N_FRAC_2);
	type shift_array_t is array (0 to N-1) of sfixed(N_INT downto N_FRAC_2);

	
	signal x_sfixed 				: sfixed(0 downto -(WIDTH_X-1)) := (others => '0');
	signal mul_ph0, 	mul_ph1 	: mul_array_t;
	signal add_ph0, 	add_ph1 	: add_array_t;
	signal shift_ph0, 	shift_ph1 	: shift_array_t;
	signal acc_ph0, 	acc_ph1 	: sfixed(N_INT downto N_FRAC_2);
	-- signal y_ph0, 		y_ph1 		: sfixed(N_INT downto N_FRAC_2);
	
	signal ph0_reg_clk1, ph1_reg_clk1	: sfixed(N_INT downto N_FRAC_2);
	signal y_clk1 						: sfixed(N_INT downto N_FRAC_2);
	
	signal xen 	: std_logic := '0';
	signal ph	: std_logic := '0';
	
	signal y_ph0, y_ph1 : std_logic_vector(WIDTH_Y-1 downto 0);
	

begin

	x_reg: process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				x_sfixed <= (others => '0');
			else
				if xen = '1' then
					x_sfixed <= to_sfixed(x, x_sfixed'high, x_sfixed'low);
				end if;
			end if;
		end if;
	end process;
	
	process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				ph <= '0';
			else
				ph <= not ph;
			end if;
		end if;
	end process;
	
	xen <= '1' when (ph = '0') else '0';
	
	-- Phase 0
	gen_mul_ph0: for i in 0 to N-1 generate
	begin
		mul_ph0(i) 	<= resize(x_sfixed * fir_ph0(N-1-i), 	mul_ph0(i)'high, mul_ph0(i)'low);
	end generate;
	
	process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				shift_ph0 <= (others => (others => '0'));
			else
				if xen = '1' then
					for i in 0 to N-3 loop
						shift_ph0(i) <= resize(shift_ph0(i+1) + mul_ph0(i+1), shift_ph0(i)'high, shift_ph0(i)'low); --add_out(i+1); 
					end loop;
					shift_ph0(N-2) <= mul_ph0(N-1); --add_out(C_NUM_TAMPS-1)
				end if;
			end if;
		end if;
	end process;

	acc_ph0 <= resize(shift_ph0(0) + mul_ph0(0), acc_ph0'high, acc_ph0'low); -- add_out(0);
	
	-- Phase 1
	gen_mul_ph1: for i in 0 to N-1 generate
	begin
		mul_ph1(i) 	<= resize(x_sfixed * fir_ph1(N-1-i), 	mul_ph1(i)'high, mul_ph1(i)'low);
	end generate;
	
	process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				shift_ph1 <= (others => (others => '0'));
			else
				if xen = '1' then
					for i in 0 to N-3 loop
						shift_ph1(i) <= resize(shift_ph1(i+1) + mul_ph1(i+1), shift_ph1(i)'high, shift_ph1(i)'low); --add_out(i+1); 
					end loop;
					shift_ph1(N-2) <= mul_ph1(N-1); --add_out(C_NUM_TAMPS-1)
				end if;
			end if;
		end if;
	end process;

	acc_ph1 <= resize(shift_ph1(0) + mul_ph1(0), acc_ph1'high, acc_ph1'low); -- add_out(0);
	
	process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				y_ph0 <= (others => '0');
				y_ph1 <= (others => '0');
			else
				if xen = '1' then
					y_ph0 <= to_slv(acc_ph0);
					y_ph1 <= to_slv(acc_ph1);
				end if;
			end if;
		end if;
	end process;
	
	process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				y 	<= (others => '0');
				y0 	<= (others => '0');
				y1 	<= (others => '0');
			else
				if xen = '1' then
					y0 <= y_ph0;
					y1 <= y_ph1;
				end if;
				
				if ph = '1' then
					y <= y_ph0;
				else
					y <= y_ph1;
				end if;
			end if;
		end if;
	end process;
	
	-- process(clk0)
	-- begin
		-- if rising_edge(clk0) then
			-- if rst = '1' then
				-- y_ph0 <= (others => '0');
				-- y_ph1 <= (others => '0');
			-- else
				-- y_ph0 <= acc_ph0;
				-- y_ph1 <= acc_ph1;
			-- end if;
		-- end if;
	-- end process;
	
	-- process(clk1)
	-- begin
	  -- if rising_edge(clk1) then
		-- if rst = '1' then
		  -- ph0_reg_clk1 <= (others => '0');
		  -- ph1_reg_clk1 <= (others => '0');
		-- else
		  -- ph0_reg_clk1 <= y_ph0;
		  -- ph1_reg_clk1 <= y_ph1;
		-- end if;
	  -- end if;
	-- end process;
	
	-- out0_process: process(clk1)
	  -- variable toggle : std_logic := '0';
	-- begin
	  -- if rising_edge(clk1) then
		-- if rst = '1' then
		  -- y_clk1 <= (others => '0');
		  -- toggle := '0';
		-- else
		  -- if toggle = '0' then
			-- y_clk1 <= ph0_reg_clk1;
			-- toggle := '1';
		  -- else
			-- y_clk1 <= ph1_reg_clk1;
			-- toggle := '0';
		  -- end if;
		-- end if;
	  -- end if;
	-- end process;
	
	-- y <= to_slv(y_clk1);

end architecture;