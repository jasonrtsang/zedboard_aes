library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_lib.STATE;

entity addRoundKey is
	port (i1 : in  STATE;
			i2 : in  STATE;
			o  : out STATE
			);
			
end addRoundKey;

architecture Behavioral of addRoundKey is

begin
	loopWords:
	for i in 0 to 3 generate
		
		loopBytes:
		for j in 0 to 3 generate
			o (i)(j) <= i1 (i)(j) xor i2 (i)(j);
				
		end generate;

	end generate;

end Behavioral;

