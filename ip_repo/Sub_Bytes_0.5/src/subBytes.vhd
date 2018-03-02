library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

entity subBytes is
    port (inMode : in AES_MODE;
          inState  : in STATE;
          outState : out STATE);    
end subBytes;

-- Substitute each byte (256 options) with that of sBox
architecture behavioral of subBytes is
    signal outState_encrypt : STATE;
    signal outState_decrypt : STATE;
begin
    -- Encrypt
    loopWords_encrypt: for j in 0 to word_size-1 generate
        loopBytes_encrypt: for k in 0 to word_size-1 generate
            sBox: entity work.sBox_encrypt port map (inByte => instate(j)(k), outByte => outState_encrypt(j)(k));
        end generate;
    end generate;
    -- Decrypt
    loopWords_decrypt: for j in 0 to word_size-1 generate
        loopBytes_decrypt: for k in 0 to word_size-1 generate
            sBox: entity work.sBox_decrypt port map (inByte => instate(j)(k), outByte => outState_decrypt(j)(k));
        end generate;
    end generate;
    -- Mux
    outstate <= outState_encrypt when (inMode = ENCRYPTION) else outState_decrypt;
end behavioral;
