library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

entity main_ecb is
    port (inMode   : in AES_MODE;
          inKey    : in STATE;
		  inState  : in STATE;
          outState : out STATE);        
end main_ecb;

architecture behavioral of main_ecb is
begin
    outstate <= inState;
end behavioral;
