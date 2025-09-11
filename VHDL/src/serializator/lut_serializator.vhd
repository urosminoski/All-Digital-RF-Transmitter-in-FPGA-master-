library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.lut_pkg.all;

entity lut_serializer is
	generic(
		LUT_ID  : integer := 3;
		XWIDTH  : integer := 4              -- širina ulaza (npr. 4 bita za -8..+7)
	);
	port(
		clk      : in  std_logic;
		rst      : in  std_logic;
		enable   : in  std_logic;
		xin      : in  std_logic_vector(XWIDTH-1 downto 0);
		xout     : out std_logic;
		sym_tick : out std_logic
	);
end entity;

architecture rtl of lut_serializer is

	-- Funkcija za dužinu reda po LUT_ID (zamenjuje "case" u konstanti)
	function lut_len(id : integer) return natural is
	begin
		case id is
			when 1      => return LUT1_LEN;
			when 2      => return LUT2_LEN;
			when 3      => return LUT3_LEN;
			when 4      => return LUT4_LEN;
			when 5      => return LUT5_LEN;
			when others => return LUT1_LEN;
		end case;
	end function;

	constant N : natural := lut_len(LUT_ID);

	-- Dohvat reda (ROM vektor) po indeksu
	function get_row(idx : integer) return std_logic_vector is
	begin
		case LUT_ID is
			when 1      => return LUT1_ROM(idx);
			when 2      => return LUT2_ROM(idx);
			when 3      => return LUT3_ROM(idx);
			when 4      => return LUT4_ROM(idx);
			when 5      => return LUT5_ROM(idx);
			when others => return LUT1_ROM(idx);
		end case;
	end function;

	-- mapiranje: d in [-(2^(XWIDTH-1)) .. +(2^(XWIDTH-1)-1)]
	-- 	 -> u = d + 2^(XWIDTH-1)   (0..2^XWIDTH-1)
	-- 	 -> flip da 0 mapira na max (+), a max mapira na min (-)
	function map_row_index(d : std_logic_vector(XWIDTH-1 downto 0)) return unsigned is
		constant BIAS : signed(XWIDTH-1 downto 0) := to_signed(2**(XWIDTH-1), XWIDTH);
		variable u    : unsigned(XWIDTH-1 downto 0);
	begin
		u := unsigned(signed(d) + BIAS);                   -- 0..2^XWIDTH-1
		return (to_unsigned(2**XWIDTH - 1, XWIDTH) - u);   -- flip
	end function;

	signal running  : std_logic := '0';
	signal col      : unsigned(31 downto 0) := (others => '0');  -- šire, pa ograničiti na N
	signal row_idx  : unsigned(XWIDTH-1 downto 0) := (others => '0');
	signal row_reg  : std_logic_vector(N-1 downto 0) := (others => '0');
	signal bit_reg  : std_logic := '0';
	
begin

	process(clk)
		variable running_v	: std_logic := '0';
		variable k 			: integer := 0;
		variable row_idx_v 	: unsigned(XWIDTH-1 downto 0) := (others => '0');
		variable row_reg_v 	: std_logic_vector(N-1 downto 0) := (others => '0');
	begin
		if rising_edge(clk) then
			if rst = '1' then
			
			else
				if running_v = '0' then
					if enable = '1' then
						row_idx_v 	:= map_row_index(xin);
						row_reg_v 	:= get_row(to_integer(row_idx_v));
						col 		<= (others => '0');
						running_v	:= '1';
					end if;
					
				else
					-- running: izbaci jedan bit po kloku
					k := to_integer(col);
					
					bit_reg <= row_reg(k);
				
				
				
					-- Brojac kolona
					if to_integer(col) = N-2 then
						sym_tick 	<= '1';	-- kraj frame-a
						col 		<= (others => '0');
						running_v 	:= '0';
					else
						col <= col + 1;
					end if;
				end if;
			end if;
		end if;
		xout 	<= bit_reg;
		row_idx <= row_idx_v;
		row_reg <= row_reg_v;
		running <= running_v;
	end process;
		
end architecture;