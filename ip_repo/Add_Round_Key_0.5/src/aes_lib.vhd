library IEEE;
use IEEE.STD_LOGIC_1164.all;

package aes_lib is
	subtype BYTE is std_logic_vector (7 downto 0);
	
	type AES_MODE is (ENCRYPTION, DECRYPTION);
	type BYTE_ARRAY is array (natural range <>) of BYTE;
	type WORD is array (0 to 3) of BYTE;
	type WORD_ARRAY is array (natural range <>) of WORD;
	type STATE is array (0 to 3) of WORD;
	type STATE_ARRAY is array (natural range <>) of STATE;
	
	constant RCON : BYTE_ARRAY (1 to 10) := (x"01", x"02", x"04", x"08", x"10", 
														  x"20", x"40", x"80", x"1B", x"36");
	
end aes_lib;

package body aes_lib is
end aes_lib;
