library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;
use ieee.fixed_pkg.all;

use work.fir_coeffs_pkg.all;  -- FIR0/1/2_PHx_R i *_N

entity osr8 is
  generic(
    COEF_L : integer := 15;
    XWIDTH : integer := 12;
    INT    : integer := 1;
    FRAC   : integer := 26
  );
  port(
    clk    : in  std_logic;
    rst    : in  std_logic;
    strobe : in  std_logic;  -- 1-taktni impuls (clk0→clk1 CDC)
    xin    : in  std_logic_vector(XWIDTH-1 downto 0);
    xout   : out std_logic_vector(XWIDTH-1 downto 0);
    vout   : out std_logic
  );
end entity;

architecture rtl of osr8 is
  constant S0_OFF0 : integer := 0;
  constant S0_OFF1 : integer := 4;
  constant S1_OFF0 : integer := 0;
  constant S1_OFF1 : integer := 2;
  constant S2_OFF0 : integer := 0;
  constant S2_OFF1 : integer := 1;

  constant NUM_TAPS0 : integer := FIR0_N;
  constant NUM_TAPS1 : integer := FIR1_N;
  constant NUM_TAPS2 : integer := FIR2_N;

  constant XWIDTH_HB2 : integer := INT+FRAC+1;

  signal frame_cnt : std_logic_vector(2 downto 0) := (others => '0');

  signal x0, x1, x2 : std_logic_vector(XWIDTH_HB2-1 downto 0);
  signal v0, v1, v2 : std_logic;
  signal seed       : std_logic := '0';

  signal xin_hb2    : std_logic_vector(XWIDTH_HB2-1 downto 0);

  -- phase-align
  signal xin_shadow : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
  signal xin_lat    : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
  signal pending    : std_logic := '0';

  -- poravnanje & seed kontrola
  signal locked       : std_logic := '0';
  signal accept_now   : std_logic := '0';  -- 1 ciklus kad usvajamo novi uzorak
  signal seed_reg     : std_logic := '0';  -- registrovani seed (ide 1 takt posle accept_now)

begin
  ------------------------------------------------------------------------------
  -- 0) Modulo-8 brojač sa "lock-on-first-strobe"
  ------------------------------------------------------------------------------
  process(clk)
    variable cnt : unsigned(frame_cnt'range) := (others => '0');
  begin
    if rising_edge(clk) then
      if rst = '1' then
        cnt    := (others => '0');
        locked <= '0';
      else
        -- Zaključavanje faze: na PRVOM strobe-u postavi fazu tako da sledeći takt bude "000".
        if (locked = '0') and (strobe = '1') then
          cnt    := "111";   -- sledeći rising_edge → "000"
          locked <= '1';
        else
          if cnt = "111" then
            cnt := (others => '0');
          else
            cnt := cnt + 1;
          end if;
        end if;
      end if;
    end if;
    frame_cnt <= std_logic_vector(cnt);
  end process;

  ------------------------------------------------------------------------------
  -- 1) Phase aligner + registrovani seed (accept na "111", seed ciklus kasnije)
  ------------------------------------------------------------------------------
  process(clk)
  begin
    if rising_edge(clk) then
      if rst = '1' then
        xin_shadow  <= (others => '0');
        xin_lat     <= (others => '0');
        pending     <= '0';
        accept_now  <= '0';
        seed_reg    <= '0';
      else
        -- defaulti
        accept_now <= '0';
        seed_reg   <= '0';

        -- novi uzorak iz CDC
        if strobe = '1' then
          xin_shadow <= xin;
          pending    <= '1';
        end if;

        -- Usvajaj u fazi "111" (tako da na sledećoj ivici "000" ulaz već stoji)
        if (frame_cnt = "111") and (pending = '1') then
          xin_lat    <= xin_shadow;
          pending    <= '0';
          accept_now <= '1';         -- označi da sledeći takt ide seed
        end if;

        -- Seed JEDAN takt posle accept-a (poravnanje bez race-a)
        if accept_now = '1' then
          seed_reg <= '1';
        end if;
      end if;
    end if;
  end process;

  seed <= seed_reg;

  ------------------------------------------------------------------------------
  -- 2) Fiksna tačka, HB2 stejdževi, izlaz
  ------------------------------------------------------------------------------
  xin_hb2 <= to_slv(
               resize(
                 to_sfixed(xin_lat, 0, -(XWIDTH-1)),
                 INT, -FRAC
               )
             );

  -- Stage 0
  u_s0: entity work.hb2_sched
    generic map(
      STAGE_ID  => 0, COEF_L => COEF_L, INT => INT, FRAC => FRAC,
      XWIDTH    => XWIDTH_HB2, NUM_TAPS => NUM_TAPS0, OFF0 => S0_OFF0, OFF1 => S0_OFF1
    )
    port map(
      clk => clk, rst => rst, seed => seed, frame_cnt => frame_cnt,
      xin => xin_hb2, xout => x0, vout => v0
    );

  -- Stage 1
  u_s1: entity work.hb2_sched
    generic map(
      STAGE_ID  => 1, COEF_L => COEF_L, INT => INT, FRAC => FRAC,
      XWIDTH    => XWIDTH_HB2, NUM_TAPS => NUM_TAPS1, OFF0 => S1_OFF0, OFF1 => S1_OFF1
    )
    port map(
      clk => clk, rst => rst, seed => v0, frame_cnt => frame_cnt,
      xin => x0, xout => x1, vout => v1
    );

  -- Stage 2
  u_s2: entity work.hb2_sched
    generic map(
      STAGE_ID  => 2, COEF_L => COEF_L, INT => INT, FRAC => FRAC,
      XWIDTH    => XWIDTH_HB2, NUM_TAPS => NUM_TAPS2, OFF0 => S2_OFF0, OFF1 => S2_OFF1
    )
    port map(
      clk => clk, rst => rst, seed => v1, frame_cnt => frame_cnt,
      xin => x1, xout => x2, vout => v2
    );

  vout <= v2;

  xout <= to_slv(
            resize(
              to_sfixed(x2, INT, -FRAC),
              0, -(XWIDTH-1)
            )
          );

end architecture;
