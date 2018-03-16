library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

entity xorWord is
    port (inWord1 : in  WORD;
          inWord2 : in  WORD;
          outWord : out WORD);
end xorWord;

-- XOR word (4-8 bit per in)
architecture behavioral of xorWord is
begin
    outWord <= (inWord1(0) xor inWord2(0), inWord1(1) xor inWord2(1), inWord1(2) xor inWord2(2), inWord1(3) xor inWord2(3));
end behavioral;
