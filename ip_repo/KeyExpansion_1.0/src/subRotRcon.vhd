library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_lib.WORD, work.aes_lib.BYTE;

entity subRotRcon is
	port (i1 : in  WORD;
			i2 : in  WORD;
			r  : in  BYTE;
			o  : out WORD
			);
			
end subRotRcon;

architecture Behavioral of subRotRcon is
	signal tmp : WORD;
	
begin
	sBox1: entity work.sBox port map (i => i2 (1), o => tmp (0));
	sBox2: entity work.sBox port map (i => i2 (2), o => tmp (1));
	sBox3: entity work.sBox port map (i => i2 (3), o => tmp (2));
	sBox4: entity work.sBox port map (i => i2 (0), o => tmp (3));
	
	o <= (tmp (0) xor i1 (0) xor     r, tmp (1) xor i1 (1) xor x"00", 
			tmp (2) xor i1 (2) xor x"00", tmp (3) xor i1 (3) xor x"00");

end Behavioral;

