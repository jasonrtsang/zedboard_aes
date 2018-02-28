library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

entity subBytes is
    port (inState  : in STATE;
          outState : out STATE);    
end subBytes;

-- Substitute each byte (256 options) with that of sBox
architecture behavioral of subBytes is
begin
    loopWords: for j in 0 to word_size-1 generate
        loopBytes: for k in 0 to word_size-1 generate
            sBox: entity work.sBox port map (inByte => instate(j)(k), outByte => outstate(j)(k));
        end generate;
    end generate;
end behavioral;
