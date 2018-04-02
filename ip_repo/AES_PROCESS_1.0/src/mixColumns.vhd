library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_package.STATE;
use work.aes_package.AES_MODE;

entity mixColumns is
	port (inState : in STATE;
		  mode    : in AES_MODE;
		  outState : out STATE
			);
end mixColumns;

architecture Behavioral of mixColumns is
	signal mult_2 : STATE;
	signal mult_3 : STATE;
	signal mult_9 : STATE;
	signal mult_b : STATE;
	signal mult_d : STATE;
	signal mult_e : STATE;
	signal mixCol : STATE;
	signal invMixCol : STATE;
	
begin
	loopWords_mult2:
	for j in 0 to 3 generate
		loopBytes_mult2:
		for k in 0 to 3 generate
			mult_2x: entity work.multiply_2 port map (inState (j)(k), mult_2 (j)(k));
		end generate;
	end generate;
		
	loopWords_mult3:
	for j in 0 to 3 generate
		loopBytes_mult3:
		for k in 0 to 3 generate
			mult_3x: entity work.multiply_3 port map (inState (j)(k), mult_3 (j)(k));
		end generate;
	end generate;
	
	loopWords_mult9:
	for j in 0 to 3 generate
		loopBytes_mult9:
		for k in 0 to 3 generate
			mult_9x: entity work.multiply_9 port map (inState (j)(k), mult_9 (j)(k));
		end generate;
	end generate;
	
	loopWords_multb:
	for j in 0 to 3 generate
		loopBytes_multb:
		for k in 0 to 3 generate
			mult_bx: entity work.multiply_b port map (inState (j)(k), mult_b (j)(k));
		end generate;
	end generate;
	
	loopWords_multd:
	for j in 0 to 3 generate
		loopBytes_multd:
		for k in 0 to 3 generate
			mult_dx: entity work.multiply_d port map (inState (j)(k), mult_d (j)(k));
		end generate;
	end generate;
	
	loopWords_multe:
	for j in 0 to 3 generate
		loopBytes_multe:
		for k in 0 to 3 generate
			mult_ex: entity work.multiply_e port map (inState (j)(k), mult_e (j)(k));
		end generate;
	end generate;

	mixColumns:
	for j in 0 to 3 generate
		mixCol (j) <= (inState (j)(2) xor inState (j)(3) xor mult_2 (j)(0) xor mult_3 (j)(1),
				       inState (j)(0) xor inState (j)(3) xor mult_2 (j)(1) xor mult_3 (j)(2),
					   inState (j)(0) xor inState (j)(1) xor mult_2 (j)(2) xor mult_3 (j)(3),    
					   inState (j)(1) xor inState (j)(2) xor mult_2 (j)(3) xor mult_3 (j)(0));  
	end generate;
	
	invMixColumns:
	for j in 0 to 3 generate
		invMixCol (j) <= (mult_b (j)(1) xor mult_d (j)(2) xor mult_9 (j)(3) xor mult_e (j)(0),
					      mult_b (j)(2) xor mult_d (j)(3) xor mult_9 (j)(0) xor mult_e (j)(1),
					      mult_b (j)(3) xor mult_d (j)(0) xor mult_9 (j)(1) xor mult_e (j)(2),
					      mult_b (j)(0) xor mult_d (j)(1) xor mult_9 (j)(2) xor mult_e (j)(3));
	end generate;
	
	-- Select encrypt or decrypt
	outState <= mixCol when (mode = ENCRYPTION) else invMixcol;
		
end Behavioral;
