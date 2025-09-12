library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

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
		enable_fast : out std_logic            -- 1 clk_fast impuls kad je novi uzorak spreman
	);
end entity;

architecture rtl of cdc_slowclk_to_fast is
	-- 1) U sporom domenu registruj bus (da bude čist i stabilan između ivica)
	signal xin_slow_reg : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');

	-- 2) U brzom domenu: 2-FF sinhronizacija sporog kloka + detekcija uzlazne ivice
	signal slow_sync1, slow_sync2 : std_logic := '0';
	signal strobe_fast            : std_logic := '0';
	signal strobe_fast_d          : std_logic := '0';

	-- 3) Dvostepeno hvatanje asinh. busa pa uzorkovanje posle jedne fast periode
	signal bus_s1, bus_s2 : std_logic_vector(XWIDTH-1 downto 0) := (others => '0');
begin
	-- slow domain register for data
	process(clk_slow)
	begin
		if rising_edge(clk_slow) then
			if rst_slow='1' then
				xin_slow_reg <= (others => '0');
			else
				xin_slow_reg <= xin_slow;   -- podatak menjaš samo na ivici slow kloka
			end if;
		end if;
	end process;

	-- fast domain: sync slow clock
	process(clk_fast)
	begin
		if rising_edge(clk_fast) then
			if rst_fast='1' then
				slow_sync1    <= '0';
				slow_sync2    <= '0';
				strobe_fast   <= '0';
				strobe_fast_d <= '0';
			else
				slow_sync1    <= clk_slow;
				slow_sync2    <= slow_sync1;
				strobe_fast   <= slow_sync1 and not slow_sync2;  -- detekcija uzlazne ivice
				strobe_fast_d <= strobe_fast;                    -- odloži za 1 fast takt
			end if;
		end if;
	end process;

	-- fast domain: dvostruko uzorkovanje busa (smanjuje rizik meta/glitch)
	process(clk_fast)
	begin
		if rising_edge(clk_fast) then
			if rst_fast = '1' then
				bus_s1 		<= (others => '0');
				bus_s2 		<= (others => '0');
				xin_fast    <= (others => '0');
				enable_fast <= '0';
			else
				-- neprekidno „preuzimanje” asinhronog busa u 2 registra
				bus_s1 <= xin_slow_reg;
				bus_s2 <= bus_s1;

				-- validan uzorak prihvati 1 fast-ciklus posle detektovane ivice slow kloka
				enable_fast <= '0';
				if strobe_fast_d = '1' then
					xin_fast    <= bus_s2;   -- bus je sada sigurno stabilan
					enable_fast <= '1';      -- strobe/enable za tvoj fast modul
				end if;
			end if;
		end if;
	end process;
end architecture;

