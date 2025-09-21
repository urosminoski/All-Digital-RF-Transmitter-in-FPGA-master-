library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.fixed_float_types.all;

entity stage3 is
	port(
		clk   	: in  std_logic;
		rst   	: in  std_logic;
		strobe	: in  std_logic;
		xin_i  	: in  std_logic;
		xin_q  	: in  std_logic;
		xout_iq	: out std_logic
	);
end entity;

architecture rtl of stage3 is
	
	signal phase_cnt	: unsigned(1 downto 0) := (others => '0');
	signal reg_i, reg_q	: std_logic := '0';

begin

	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				phase_cnt <= (others => '0');
			else
				if phase_cnt = "11" or strobe = '1' then
					phase_cnt <= (others => '0');
				else
					phase_cnt <= phase_cnt + 1;
				end if;
			end if;
		end if;
	end process;
	
	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				reg_i <= '0';
				reg_q <= '0';
			elsif strobe = '1' then
				reg_i <= xin_i;
				reg_q <= xin_q;
			end if;
		end if;	
	end process;
	
	process(clk)
	begin
		if rising_edge(clk) then
			if rst = '1' then
				xout_iq <= '0';
			else
				case phase_cnt is
					when "00" => xout_iq <= reg_i;
					when "01" => xout_iq <= reg_q;
					when "10" => xout_iq <= not reg_i;
					when "11" => xout_iq <= not reg_q;
					when others => xout_iq <= '0';
				end case;
			end if;
		end if;
	end process;

end architecture;