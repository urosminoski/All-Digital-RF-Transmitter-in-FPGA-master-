library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

entity interpolation is
	port(
		clk0, clk1, clk2 	: in std_logic;
		rst 				: in  std_logic;
		x      				: in  std_logic_vector(15 downto 0);
		y      				: out std_logic_vector(31 downto 0)
	);
end entity;

architecture rtl of interpolation is

	constant N_FRAC : integer := -15;

	constant N0 : integer := 42;
	constant N1 : integer := 5;--10;
	constant N2 : integer := 3;--6;
	type fir0_array_t is array (0 to N0-1) of sfixed(0 downto N_FRAC);
	type fir1_array_t is array (0 to N1-1) of sfixed(0 downto N_FRAC);
	type fir2_array_t is array (0 to N2-1) of sfixed(0 downto N_FRAC);

	-- FIR 0 Coeffitients
	constant fir0_phase0 : fir0_array_t := (
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
	
	constant fir0_phase1 : sfixed(0 downto N_FRAC) := to_sfixed(0.5, 0, N_FRAC);


	-- FIR1 Coeffitients
	-- constant fir1_phase0 : fir1_array_t := (
		-- to_sfixed( 0.001465, 0, -11),
		-- to_sfixed(-0.011230, 0, -11),
		-- to_sfixed( 0.046387, 0, -11),
		-- to_sfixed(-0.149414, 0, -11),
		-- to_sfixed( 0.612305, 0, -11)
		-- to_sfixed( 0.612305, 0, -11),
		-- to_sfixed(-0.149414, 0, -11),
		-- to_sfixed( 0.046387, 0, -11),
		-- to_sfixed(-0.011230, 0, -11),
		-- to_sfixed( 0.001465, 0, -11)
	--);
	
	-- constant fir1_phase1 : sfixed(0 downto -11) := to_sfixed(0.999512, 0, -11);
	
	-- FIR2 Coeffitients
	-- constant fir2_phase0 : fir2_array_t := (
		-- to_sfixed( 0.013184, 0, -11),
		-- to_sfixed(-0.102539, 0, -11),
		-- to_sfixed( 0.588867, 0, -11)
		-- to_sfixed( 0.588867, 0, -11),
		-- to_sfixed(-0.102539, 0, -11),
		-- to_sfixed( 0.013184, 0, -11)
	--);
	
	-- constant fir2_phase1 : sfixed(0 downto -11) := to_sfixed(0.999512, 0, -11);
	
	constant N_INT 		: integer := 4;
	constant N_FRAC_2 	: integer := -27;
	
	type mul0_array_t is array (0 to N0-1) of sfixed(N_INT downto N_FRAC_2);
	type add0_array_t is array (0 to N0-1) of sfixed(N_INT downto N_FRAC_2);
	type shift0_array_t is array (0 to N0-1) of sfixed(N_INT downto N_FRAC_2);
	type shift0_array_t_2 is array (0 to N0/2-1) of sfixed(N_INT downto N_FRAC_2); 

	
	signal mul0 	: mul0_array_t;
	signal add0 	: add0_array_t;
	signal shift0 	: shift0_array_t;
	signal shift0_2 : shift0_array_t_2;
	
	signal x_sfixed 	: sfixed(0 downto -15) := (others => '0');

	signal x0_phase0, 		x0_phase1 	: sfixed(N_INT downto N_FRAC_2);
	signal ph0_reg_clk1, 	ph1_reg_clk1 : sfixed(N_INT downto N_FRAC_2);
	
	signal xout0 		: sfixed(N_INT downto N_FRAC_2) := (others => '0');
	

begin

	x_reg: process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				x_sfixed <= (others => '0');
			else
				x_sfixed <= to_sfixed(x, x_sfixed'high, x_sfixed'low);
			end if;
		end if;
	end process;
	
	-- FIR 0
	mul0(0) <= resize(x_sfixed * fir0_phase0(N0-1), mul0(0)'high, mul0(0)'low);
	add0(0) <= mul0(0);
	
	gen_mul0 : for i in 1 to N0-1 generate
	begin
		mul0(i) 	<= resize(x_sfixed * fir0_phase0(N0-1-i), mul0(i)'high, mul0(i)'low);
		add0(i)	<= resize(shift0(i) + mul0(i), add0(i)'high, add0(i)'low);
	end generate;
	
	shift0_process : process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				shift0 <= (others => (others => '0'));
			else
				for i in 1 to N0-1 loop
					shift0(i) <= add0(i-1);
				end loop;
				shift0(0) <= add0(0);
			end if;
		end if;
	end process shift0_process;
	
	shift0_process_2: process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				shift0_2 <= (others => (others => '0'));
			else
				shift0_2(0) <= resize(x_sfixed*fir0_phase1, shift0_2(0)'high, shift0_2(0)'low);
				for i in 1 to N0/2-1 loop
					shift0_2(i) <= shift0_2(i-1);
				end loop;
			end if;
		end if;
	end process;
	
	process(clk0)
	begin
		if rising_edge(clk0) then
			if rst = '1' then
				x0_phase0 <= (others => '0');
				x0_phase1 <= (others => '0');
			else
				x0_phase0 <= add0(N0-1);
				x0_phase1 <= shift0_2(N0/2-1);
			end if;
		end if;
	end process;
	
	-- sync u clk1
	process(clk1)
	begin
	  if rising_edge(clk1) then
		if rst = '1' then
		  ph0_reg_clk1 <= (others => '0');
		  ph1_reg_clk1 <= (others => '0');
		else
		  ph0_reg_clk1 <= x0_phase0;
		  ph1_reg_clk1 <= x0_phase1;
		end if;
	  end if;
	end process;

	-- serializer u clk1
	out0_process: process(clk1)
	  variable toggle : std_logic := '0';
	begin
	  if rising_edge(clk1) then
		if rst = '1' then
		  xout0 <= (others => '0');
		  toggle := '0';
		else
		  if toggle = '0' then
			xout0 <= ph0_reg_clk1;
			toggle := '1';
		  else
			xout0 <= ph1_reg_clk1;
			toggle := '0';
		  end if;
		end if;
	  end if;
	end process;
	
	y <= to_slv(xout0);
	
	-- FIR 1

end architecture;