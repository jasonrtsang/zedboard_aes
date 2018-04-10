library ieee;
use ieee.std_logic_1164.all;

-- aes_package.vhd
-- Contains the datatypes used throughout the AES project

package aes_package is
    constant byte_size : positive := 8;
    constant word_size : positive := 4;
    constant state_size : positive := 4;
    constant num_rounds : positive := 11;
    
    -- BYTE:
    --  (7 downto 0)
    -- WORD:
    --  (7 downto 0) (7 downto 0) (7 downto 0) (7 downto 0)
    -- STATE:
    --  ((7 downto 0) (7 downto 0) (7 downto 0) (7 downto 0)),
    --  ((7 downto 0) (7 downto 0) (7 downto 0) (7 downto 0)),
    --  ((7 downto 0) (7 downto 0) (7 downto 0) (7 downto 0)),
    --  ((7 downto 0) (7 downto 0) (7 downto 0) (7 downto 0))

    subtype BYTE is std_logic_vector (byte_size-1 downto 0);
    type WORD is array (0 to word_size-1) of BYTE;
    type STATE is array (0 to word_size-1) of WORD;
    
    type BYTE_ARRAY is array (natural range <>) of BYTE;
    type WORD_ARRAY is array (natural range <>) of WORD;
    type STATE_ARRAY is array (natural range <>) of STATE;
    
    type AES_MODE is (ENCRYPTION, DECRYPTION);  

    -- Round constant exponentiation of 2
    -- Only rcon[10] needed for 128-bit AES
    -- Start from rcon[1] as x"8d" at 0 is not used in algorithm
    -- https://en.wikipedia.org/wiki/Rijndael_key_schedule
    constant RCON : BYTE_ARRAY (1 to num_rounds-1) := (x"01", x"02", x"04", x"08", x"10", 
                                             x"20", x"40", x"80", x"1B", x"36");
end package aes_package;
