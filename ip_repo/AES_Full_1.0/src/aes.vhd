library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.aes_package.all;

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

	generate_key_schedule:
	entity work.keyExpansion port map (inMode => mode, inKey => key_state_array, outRoundKey => key_schedule); -- Need to fix this, key expansion takes word array

	generate_first_state:
	entity work.addRoundKey port map (inState => i, inKey => key_schedule (0), outState => first_state);
	
	generate_next_state:
	entity work.nextState port map (inState => curr_state, mode=> mode, inKey => curr_key, outState => next_state);

	generate_last_state:  
	entity work.lastState port map (inState => curr_state, mode => mode, inKey => curr_key, outState => o);
	
	process (clk, reset) begin
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

