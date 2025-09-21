library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;

entity stage1 is
	generic (
		-- OSR Parameters
		OSR_WIDTH		: integer := 12;
		OSR_COEFF		: integer := 11;
		OSR_GUARD_BITS 	: integer := 4;
		-- Fract Delay Parameters
		KERNEL_ID		: integer := 7;
		DELTA			: real := 0.00390625;
		-- Delta-Sigma Parameters
		DS_WIDTH		: integer := 12
	);
	port(
		clk   				: in  std_logic;
		rst   				: in  std_logic;
		strobe				: in  std_logic;
		xin_i  				: in  std_logic_vector(OSR_WIDTH-1 downto 0);
		xin_q  				: in  std_logic_vector(OSR_WIDTH-1 downto 0);
		xout_i_osr8_test	: out std_logic_vector(OSR_WIDTH-1 downto 0);
		xout_q_osr8_test	: out std_logic_vector(OSR_WIDTH-1 downto 0);
		xout_i_delay_test	: out std_logic_vector(OSR_WIDTH-1 downto 0);
		xout_q_delay_test	: out std_logic_vector(OSR_WIDTH-1 downto 0);
		xin_i_ds_test		: out std_logic_vector(DS_WIDTH-1 downto 0);
		xin_q_ds_test		: out std_logic_vector(DS_WIDTH-1 downto 0);
		xout_i				: out std_logic_vector(3 downto 0);
		xout_q				: out std_logic_vector(3 downto 0)
	);
end entity;

architecture rtl of stage1 is

	constant OSR_INT 	: integer := 0;

	constant DELTA_I 	: real := -DELTA;
	constant DELTA_Q 	: real := DELTA;
	constant factor		: real := 4.0/(1.0+DELTA);

	signal xin_i_delay, xin_q_delay 	: std_logic_vector(OSR_WIDTH-1 downto 0);
	signal xout_i_delay, xout_q_delay 	: std_logic_vector(OSR_WIDTH-1 downto 0);
	
	signal xout_i_osr8, xout_q_osr8 : std_logic_vector(OSR_WIDTH-1 downto 0);
	signal vout_i, vout_q 			: std_logic := '0';
	
	signal xin_i_ds, xin_q_ds 	: std_logic_vector(DS_WIDTH-1 downto 0);
	signal xout_i_ds, xout_q_ds	: std_logic_vector(3 downto 0);
	
begin

	xout_i_osr8_test	<= xout_i_osr8;
	xout_q_osr8_test	<= xout_q_osr8;
	xout_i_delay_test	<= xout_i_delay;
	xout_q_delay_test	<= xout_q_delay;
	xin_i_ds_test 		<= xin_i_ds;
	xin_q_ds_test 		<= xin_q_ds;

	-------------------------------------------------------------------------------
	-- INTERPOLATION x8
	-------------------------------------------------------------------------------

	osr8_i: entity work.osr8
		generic map (
			COEF_L		=> OSR_COEFF,
			XWIDTH		=> OSR_WIDTH,
			INT  		=> OSR_INT,
			GUARD_BITS	=> OSR_GUARD_BITS
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			strobe 	=> strobe,
			xin   	=> xin_i,
			xout   	=> xout_i_osr8,
			vout 	=> vout_i
		);
		
	osr8_q: entity work.osr8
		generic map (
			COEF_L		=> OSR_COEFF,
			XWIDTH		=> OSR_WIDTH,
			INT  		=> OSR_INT,
			GUARD_BITS	=> OSR_GUARD_BITS			
		)
		port map (
			clk 	=> clk,
			rst 	=> rst,
			strobe 	=> strobe,
			xin   	=> xin_q,
			xout   	=> xout_q_osr8,
			vout 	=> vout_q
		);
		
	-------------------------------------------------------------------------------
	-- Dealay & Normalization to x4 [-4, 4]
	-------------------------------------------------------------------------------
	
	xin_i_delay <= to_slv(
		resize(
			to_sfixed(factor, 3, -8) * to_sfixed(xout_i_osr8, OSR_INT, -(OSR_WIDTH-OSR_INT-1)),
			3,
			-(OSR_WIDTH-4)
		)
	);
	
	xin_q_delay <= to_slv(
		resize(
			to_sfixed(factor, 3, -8) * to_sfixed(xout_q_osr8, OSR_INT, -(OSR_WIDTH-OSR_INT-1)),
			3,
			-(OSR_WIDTH-4)
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
			clk		=> clk,
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
			clk		=> clk,
			rst		=> rst, 
			en		=> '1', 		
			xin		=> xin_q_delay,
			xout	=> xout_q_delay      
		);
	
	-------------------------------------------------------------------------------
	-- Delta-Sigma Modulation
	-------------------------------------------------------------------------------
	
	xin_i_ds <= to_slv(
		resize(
			to_sfixed(xout_i_delay, 3, -(OSR_WIDTH-4)),
			3,
			-(DS_WIDTH-4)
		)
	);
	
	xin_q_ds <= to_slv(
		resize(
			to_sfixed(xout_q_delay, 3, -(OSR_WIDTH-4)),
			3,
			-(DS_WIDTH-4)
		)
	);
	
	deltaSigma_i: entity work.deltaSigma
		generic map ( XWIDTH => DS_WIDTH )
		port map (
			clk		=> clk,
			rst 	=> rst,
			x 		=> xin_i_ds,
			y		=> xout_i_ds
		);
		
	deltaSigma_q: entity work.deltaSigma
		generic map ( XWIDTH => DS_WIDTH )
		port map (
			clk		=> clk,
			rst 	=> rst,
			x 		=> xin_q_ds,
			y		=> xout_q_ds
		);

	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xout_i 	<= (others => '0');
				xout_q 	<= (others => '0');
			else	
				xout_i 	<= xout_i_ds;
				xout_q 	<= xout_q_ds;
			end if;
		end if;
	end process;

end architecture;