library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package delay_kernel_pkg is
	-- Jednostavan tip za niz real koeficijenata (dužina se zadaje pri deklaraciji konstante)
	type real_vec_t is array (natural range <>) of real;

	--------------------
	-- Kernel K3
	--------------------
	constant N_K3 : natural := 3;

	constant K3_R : real_vec_t(0 to N_K3-1) := (
	 	0.5,
		0.0,
		-0.5
	);

	--------------------
	-- Kernel K5
	--------------------
	constant N_K5 : natural := 5;

	constant K5_R : real_vec_t(0 to N_K5-1) := (
		-0.083333333333,
		0.666666666667,
		0.000000000000,
		-0.666666666667,
		0.083333333333
	);

	--------------------
	-- Kernel K7
	--------------------
	constant N_K7 : natural := 7;

	constant K7_R : real_vec_t(0 to N_K7-1) := (
		0.016666666667,
		-0.150000000000,
		0.750000000000,
		0.000000000000,
		-0.750000000000,
		0.150000000000,
		-0.016666666667
	);

end package delay_kernel_pkg;
