library ieee;
use ieee.std_logic_1164.all;
use work.aes_package.all;

entity subRotRcon is
    port (inWord1 : in  WORD;
          inWord2 : in  WORD;
          rcon    : in  BYTE;
          outWord : out WORD);
end subRotRcon;

-- Substitute input2 (xor'd) with sBox value, then XOR with rcon and input1 for rotation
architecture behavioral of subRotRcon is
    signal tmp : WORD;
begin
    sBox1: entity work.sBox_encrypt port map (inByte => inWord2(1), outByte => tmp(0));
    sBox2: entity work.sBox_encrypt port map (inByte => inWord2(2), outByte => tmp(1));
    sBox3: entity work.sBox_encrypt port map (inByte => inWord2(3), outByte => tmp(2));
    sBox4: entity work.sBox_encrypt port map (inByte => inWord2(0), outByte => tmp(3));
    outWord <= (tmp(0) xor inWord1(0) xor rcon, tmp(1) xor inWord1(1) xor x"00", tmp(2) xor inWord1(2) xor x"00", tmp(3) xor inWord1(3) xor x"00");
end behavioral;
