library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_package.all;

entity nextState is

	port (inState : in  STATE;
			inKey : in  STATE;
			mode  : AES_MODE;
			outState : out STATE
			);
			
end nextState;

architecture Behavioral of nextState is
	signal temp_encrypt : STATE_ARRAY (0 to 3);
	signal temp_decrypt : STATE_ARRAY (0 to 3);

begin
		
		subBytes   : entity work.subBytes    port map (inMode => ENCRYPTION, inState  => inState, outState  => temp_encrypt (0));
		shiftRows  : entity work.shiftRows   port map (inMode => ENCRYPTION, inState => temp_encrypt (0), outState  => temp_encrypt (1));
		mixColumns : entity work.mixColumns  port map (inState  => temp_encrypt (1), mode => ENCRYPTION, outState  => temp_encrypt (2));
		addRoundKey: entity work.addRoundKey port map (inState => temp_encrypt (2), inKey => inKey, outState => temp_encrypt (3));
		
		invShiftRows : entity work.shiftRows  port map (inMode => DECRYPTION, inState => inState, outState => temp_decrypt (0));
		invSubBytes  : entity work.subBytes   port map (inMode => DECRYPTION, inState => temp_decrypt (0), outState => temp_decrypt (1));
		invAddRoundKey  : entity work.addRoundKey   port map (inState => temp_decrypt (1), inKey => inKey, outState => temp_decrypt (2));
		invMixColumns: entity work.mixColumns port map (inState => temp_decrypt (2), mode => DECRYPTION, outState => temp_decrypt (3));
		
		outState <= temp_encrypt (3) when (mode = ENCRYPTION) else temp_decrypt (3);

end Behavioral;

