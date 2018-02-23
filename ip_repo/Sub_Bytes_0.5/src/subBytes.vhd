library ieee;
use ieee.std_logic_1164.all;
use work.aes_lib.STATE;

entity subBytes is
	port (i : in STATE;
			o : out STATE
			);
			
end subBytes;

architecture Behavioral of subBytes is

begin
	loopWords:	
	for j in 0 to 3 generate
		
		loopBytes:
		for k in 0 to 3 generate
			sBox: entity work.sBox port map (i (j)(k), o (j)(k));
		
		end generate;
		
	end generate;
			
end Behavioral;

