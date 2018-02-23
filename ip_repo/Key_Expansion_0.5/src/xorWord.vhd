library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_lib.WORD;

entity xorWord is
	port (i1 : in  WORD;
			i2 : in  WORD;
			o  : out WORD
			);
end xorWord;

architecture Behavioral of xorWord is

begin
	o <= (i1 (0) xor i2 (0), i1 (1) xor i2 (1),
			i1 (2) xor i2 (2), i1 (3) xor i2 (3));

end Behavioral;

