library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

entity shiftRows is
    port (inMode   : in AES_MODE;
		  inState  : in STATE;
          outState : out STATE);        
end shiftRows;

-- Shift each row of state to the left by offset row number
architecture behavioral of shiftRows is
    signal outState_encrypt : STATE;
    signal outState_decrypt : STATE;
begin
    -- Encrypt
    outState_encrypt <= ((inState(0)(0), inState(1)(1), inState(2)(2), inState(3)(3)),
						 (inState(1)(0), inState(2)(1), inState(3)(2), inState(0)(3)),
						 (inState(2)(0), inState(3)(1), inState(0)(2), inState(1)(3)),
						 (inState(3)(0), inState(0)(1), inState(1)(2), inState(2)(3)));
    -- Decrypt 
	outState_decrypt <= ((inState(0)(0), inState(3)(1), inState(2)(2), inState(1)(3)),
						 (inState(1)(0), inState(0)(1), inState(3)(2), inState(2)(3)),
						 (inState(2)(0), inState(1)(1), inState(0)(2), inState(3)(3)),
						 (inState(3)(0), inState(2)(1), inState(1)(2), inState(0)(3)));
	-- Mux
    outstate <= outState_encrypt when (inMode = ENCRYPTION) else outState_decrypt;
end behavioral;
