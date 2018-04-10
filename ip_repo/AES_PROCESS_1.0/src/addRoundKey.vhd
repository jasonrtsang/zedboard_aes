library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

-- addRoundKey.vhd
-- Performs the Add Round Key operation of the AES process
-- Essemtially XORs the inState with the inKey, and sets the output
-- to outState

entity addRoundKey is
    port (inState  : in  STATE;
          inKey    : in  STATE;
          outState : out STATE);
end addRoundKey;

-- Adds the round key to state by an XOR function
architecture behavioral of addRoundKey is
begin
    loopWords: for i in 0 to (word_size-1) generate
        loopBytes: for j in 0 to (word_size-1) generate
            outState(i)(j) <= inState(i)(j) xor inKey(i)(j);
        end generate loopBytes;
    end generate loopWords;
end behavioral;
