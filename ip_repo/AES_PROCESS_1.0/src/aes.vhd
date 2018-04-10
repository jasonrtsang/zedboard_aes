library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_package.all;

-- aes.vhd
-- The overall state machine that runs the AES Algorithm
-- INPUTS:
-- clk   - clock signal
-- reset - resets the state machine, to be used after done = 1 and data has been read from "o"
-- mode  - selects encryption or decryption
-- i     - 128 bit input state to be encrypted to decrypted
-- k     - 128 bit key to be used
-- OUTPUTS:
-- o     - 128 resulting output (encrypted or decrypted input)
-- done  - asserted when data has been processed through the final round, data is ready to be read from "o"
--
-- This 128 bit implimentation of AES requires 10 clock cycles from reset until "done" is asserted

entity aes is	
	port (clk   : in  std_logic;
			reset : in  std_logic;
			done : out std_logic;
			mode : in AES_MODE;
			i     : in  STATE;
			k     : in  STATE;
			o     : out STATE
			);

end aes;

architecture Behavioral of aes is
	type ROUND_TYPE is (round0, round1, round2, round3, round4, round5, round6, round7, 
							  round8, round9, round10);
							  
	signal round : ROUND_TYPE;	
	signal key_schedule : STATE_ARRAY (0 to 10);
	signal curr_key : STATE;
	signal curr_state : STATE;
	signal next_state : STATE;
	signal first_state : STATE;
	
	signal key_state_array : WORD_ARRAY (0 to 3);
	
begin
    key_state_array <= (k(0), k(1), k(2), k(3));

    -- Key schedule created for all 10+1 rounds, kept in the key_schedule state array
	generate_key_schedule:
	entity work.keyExpansion port map (inMode => mode, inKey => key_state_array, outRoundKey => key_schedule);

	-- The initial round consists of only the Add Round Key processs
	generate_first_state:
	entity work.addRoundKey port map (inState => i, inKey => key_schedule (0), outState => first_state);
	
	-- nextState is used for each round of the AES process, and progresses via the state machine below
	-- where we move curr_state and curr_key forward, and set the new key via the held key_schedule state array
	generate_next_state:
	entity work.nextState port map (inState => curr_state, mode=> mode, inKey => curr_key, outState => next_state);

	-- similar to nextState, except internally there is no MixColumns that occurs
	generate_last_state:  
	entity work.lastState port map (inState => curr_state, mode => mode, inKey => curr_key, outState => o);
	
	process (clk, reset) begin
		-- Reset to first round, data no longer valid at output
		if reset = '1' then
			round <= round0;
			done <= '0';
		
		elsif clk'event and clk = '1' then
			case round is
				when round0  => round <= round1;
									 curr_key <= key_schedule (1);
									 curr_state <= first_state;
									 
				when round1  => round <= round2;
									 curr_key <= key_schedule (2);
									 curr_state <= next_state;
									 
				when round2  => round <= round3;
									 curr_key <= key_schedule (3);
									 curr_state <= next_state;
									 
				when round3  => round <= round4;
									 curr_key <= key_schedule (4);
									 curr_state <= next_state;
									 
				when round4  => round <= round5;
									 curr_key <= key_schedule (5);
									 curr_state <= next_state;
									 
				when round5  => round <= round6;
									 curr_key <= key_schedule (6);
									 curr_state <= next_state;
									 
				when round6  => round <= round7;
									 curr_key <= key_schedule (7);
									 curr_state <= next_state;
									 
				when round7  => round <= round8;
									 curr_key <= key_schedule (8);
									 curr_state <= next_state;
									 
				when round8  => round <= round9;
									 curr_key <= key_schedule (9);
									 curr_state <= next_state;
									 
				when round9  => round <= round10;
									 curr_key <= key_schedule (10);
									 curr_state <= next_state;
				
				-- The final round keeps the current state at current state, the data is 
				-- simply held at round 10 until a reset is called
				when round10 => round <= round10;
										curr_key <= key_schedule (10);
										curr_state <= curr_state;
										done <= '1';
									 
				when others  => round <= round0;
									 curr_key <= key_schedule (0);
									 curr_state <= curr_state;
				
			end case;
			
		end if;
		
	end process;

end Behavioral;

