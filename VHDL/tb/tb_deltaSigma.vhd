-- ===============================================================
--  tb_deltaSigma.vhd  (VHDL-2008)
--  Testbench za delta_sigma: x = sfixed(3 downto -8)  (12 bita)
--                            y = sfixed(3 downto  0)  (4  bita)
--  Upisuje izlaz u "./simulator_output.txt"
-- ===============================================================

-- Instanciraj generic paket sa podrazumevanim parametrima
package fixed_pkg is new work.fixed_generic_pkg;
use work.fixed_pkg.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;  -- tipovi za sfixed/ufixed
use work.fixed_pkg.all;          -- funkcije/to_integer/konverzije

library std;
use std.textio.all;              -- TEXT, LINE, write/writeline, file I/O

entity tb_deltaSigma is
end entity tb_deltaSigma;

architecture tb of tb_deltaSigma is

	constant C_CLK_FREQ		: integer := 150_000_000;
	constant C_CLK_PERIOD 	: time := 1 sec / C_CLK_FREQ;
	
	signal clk	: std_logic := '0';
	signal rst 	: std_logic := '1';
	signal x 	: std_logic_vector(11 downto 0) := (others => '0');
	signal y 	: std_logic_vector(3 downto 0) := (others => '0');
	
	file input_file 	: text open read_mode is "..\data\xin.txt";
	file output_file 	: text open write_mode is "..\data\xout.txt";

  constant C_CLK_PERIOD : time := 1 ns;

  signal clk  : std_logic := '1';
  signal rst  : std_logic := '1';

  -- Usaglašene širine sa DUT (prethodno dobijene iz poruke kompajlera):
  signal x : sfixed(3 downto -8) := (others => '0');  -- 12 bita
  signal y : sfixed(3 downto  0);                     -- 4  bita

  -- Stimulus: 1024 real vrednosti; ovde inicijalizovano na 0.0
  -- (po potrebi kasnije učitaj iz fajla u procesu, ali konstanta mora imati vrednost)
  type x_input is array (0 to 1023) of real;
  constant x_in : x_input := (others => 0.0);

begin

  -- ====== DUT instanca ======
  -- Ako ti je entitet nazvan "deltaSigma", promeni sledeću liniju u: entity work.deltaSigma
  I_DUT : entity work.deltaSigma
    port map (
      clk => clk,
      rst => rst,
      x   => x,
      y   => y
    );

  -- ====== Clock ======
  clk <= not clk after C_CLK_PERIOD/2;

  -- ====== Reset ======
  p_reset : process
  begin
    rst <= '1';
    wait for 5 * C_CLK_PERIOD;  -- kratko drži reset
    rst <= '0';
    wait;
  end process;

  -- ====== Ulazni stimulus ======
  p_input : process(clk)
    variable i : integer := 0;
  begin
    if rst = '1' then
      i := 0;
      x <= (others => '0');
    elsif rising_edge(clk) then
      if i <= 1023 then
        -- Konverzija real -> sfixed(3 downto -8)
        x <= to_sfixed(x_in(i), 3, -8);
        i := i + 1;
      end if;
    end if;
  end process;

  -- ====== Izlazni log u fajl ======
  p_output : process(clk)
    file file_pointer : TEXT open WRITE_MODE is "./simulator_output.txt";
    variable line_el   : LINE;
    variable y_integer : integer;
  begin
    if rising_edge(clk) then
      -- Direktno iz sfixed u integer (funkcija iz fixed_pkg instance)
      y_integer := to_integer(y);
      write(line_el, y_integer);
      writeline(file_pointer, line_el);
    end if;
  end process;

end architecture tb;
