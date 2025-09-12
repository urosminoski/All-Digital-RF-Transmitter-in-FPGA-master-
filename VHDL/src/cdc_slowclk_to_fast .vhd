library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--  Spor->Brz CDC za paralelni bus:
--  - xin_slow se registruje u sporom domenu (čist bus)
--  - u brzom: 2-FF sync sporog kloka + detekcija uzlazne ivice
--  - bus uzorkujemo u tri brza registra, a prihvatamo ga tek
--    nakon dodatnog jednog fast ciklusa od ivice (strobe_d2)
--  - enable_fast je tačno jedan clk_fast ciklus
--
--  Napomena za constraints:
--    - postavite false-path ili asynch edges od clk_slow ka clk_fast
--    - zadržite ASYNC_REG na slow_sync1/2

entity cdc_slowclk_to_fast is
  generic (
    XWIDTH : integer := 4
  );
  port (
    -- slow domain
    clk_slow    : in  std_logic;
    rst_slow    : in  std_logic;
    xin_slow    : in  std_logic_vector(XWIDTH-1 downto 0);

    -- fast domain
    clk_fast    : in  std_logic;
    rst_fast    : in  std_logic;
    xin_fast    : out std_logic_vector(XWIDTH-1 downto 0);
    enable_fast : out std_logic            -- 1 clk_fast impuls kada stigne novi uzorak
  );
end entity;

architecture rtl of cdc_slowclk_to_fast is
  -- 1) U sporom domenu registruj bus (stabilan između ivica)
  signal xin_slow_reg : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');

  -- 2) U brzom: 2-FF sinhronizacija sporog kloka + detekcija uzlazne ivice
  signal slow_sync1, slow_sync2 : std_logic := '0';
  signal strobe_fast            : std_logic := '0';
  signal strobe_fast_d          : std_logic := '0';
  signal strobe_fast_d2         : std_logic := '0';

  -- 3) Trokorak hvatanja asinhr. busa u brzom domenu
  signal bus_s1, bus_s2, bus_s3 : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');

  -- Synthesis hint: označi sinhronizatore
  attribute ASYNC_REG : string;
  attribute ASYNC_REG of slow_sync1 : signal is "TRUE";
  attribute ASYNC_REG of slow_sync2 : signal is "TRUE";
begin
  ---------------------------------------------------------------------------
  -- SLOW DOMAIN: reg bus
  ---------------------------------------------------------------------------
  process(clk_slow)
  begin
    if rising_edge(clk_slow) then
      if rst_slow='1' then
        xin_slow_reg <= (others => '0');
      else
        xin_slow_reg <= xin_slow;  -- menja se samo na ivici slow kloka
      end if;
    end if;
  end process;

  ---------------------------------------------------------------------------
  -- FAST DOMAIN: sync sporog kloka i detekcija uzlazne ivice
  ---------------------------------------------------------------------------
  process(clk_fast)
  begin
    if rising_edge(clk_fast) then
      if rst_fast='1' then
        slow_sync1     <= '0';
        slow_sync2     <= '0';
        strobe_fast    <= '0';
        strobe_fast_d  <= '0';
        strobe_fast_d2 <= '0';
      else
        slow_sync1     <= clk_slow;               -- 1. sync
        slow_sync2     <= slow_sync1;             -- 2. sync
        strobe_fast    <= slow_sync1 and not slow_sync2; -- uzlazna ivica
        strobe_fast_d  <= strobe_fast;            -- +1 fast ciklus
        strobe_fast_d2 <= strobe_fast_d;          -- +2 fast ciklusa (bezbedan momenat)
      end if;
    end if;
  end process;

  ---------------------------------------------------------------------------
  -- FAST DOMAIN: hvatanje busa i izdavanje 1-taktnog enable
  ---------------------------------------------------------------------------
  process(clk_fast)
  begin
    if rising_edge(clk_fast) then
      if rst_fast='1' then
        bus_s1      <= (others => '0');
        bus_s2      <= (others => '0');
        bus_s3      <= (others => '0');
        xin_fast    <= (others => '0');
        enable_fast <= '0';
      else
        -- kontinuirano uvlačenje asinhr. busa kroz 3 registra
        bus_s1 <= xin_slow_reg;
        bus_s2 <= bus_s1;
        bus_s3 <= bus_s2;

        -- 1-taktni impuls i prihvat posle dodatnog ciklusa od ivice
        enable_fast <= '0';
        if strobe_fast_d2 = '1' then
          xin_fast    <= bus_s3;   -- bus je stabilan
          enable_fast <= '1';      -- tačno 1 clk_fast ciklus
        end if;
      end if;
    end if;
  end process;
end architecture;
