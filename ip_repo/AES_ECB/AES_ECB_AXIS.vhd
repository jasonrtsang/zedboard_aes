------------------------------------------------------------------------------
-- AES_ECB_AXIS
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.aes_package.all;

-------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Entity Section
------------------------------------------------------------------------------

entity AES_ECB_AXIS is
    port
    (
        -- Bus protocol ports
        ACLK            : in  std_logic;                     -- Synchronous clock
        ARESETN         : in  std_logic;                     -- System reset, active low
        S_AXIS_TREADY   : out std_logic;                     -- Ready to accept data in
        S_AXIS_TDATA    : in  std_logic_vector(31 downto 0); -- Data in
        S_AXIS_TLAST    : in  std_logic;                     -- Optional data in qualifier
        S_AXIS_TVALID   : in  std_logic;                     -- Data in is valid
        M_AXIS_TVALID   : out std_logic;                     -- Data out is valid
        M_AXIS_TDATA    : out std_logic_vector(31 downto 0); -- Data Out
        M_AXIS_TLAST    : out std_logic;                     -- Optional data out qualifier
        M_AXIS_TREADY   : in  std_logic                      -- Connected slave device is ready to accept data out
    );
end AES_ECB_AXIS;

------------------------------------------------------------------------------
-- Architecture Section
------------------------------------------------------------------------------

-- 1. Read all inputs
-- 2. Save all inputs to input buffer
-- 3. Copy input buffer content to output buffer
-- 3. Write out contents of output buffer NUMBER_OF_OUTPUT_WORDS times

architecture arch_imp of AES_ECB_AXIS is

    -- Number of input/ output data packets
    constant NUMBER_OF_INPUT_WORDS  : natural := 9;
    constant NUMBER_OF_OUTPUT_WORDS : natural := 4;
    -- Byte of zeros
    constant ZEROS : std_logic_vector(31 downto 0) := (others=>'0');
    constant ONES : std_logic_vector(31 downto 0) := (others=>'1');

    -- AXIS states
    type AXIS_STATE_TYPE is (Idle, Read_Inputs, Processing, Send_Outputs, Write_Outputs);
    signal axis_state : AXIS_STATE_TYPE;

    -- Counters to store the number inputs/ outputs written
    signal num_of_reads   : natural range 0 to NUMBER_OF_INPUT_WORDS-1;
    signal num_of_process : natural range 0 to NUMBER_OF_INPUT_WORDS-1;
    signal num_of_writes  : natural range 0 to NUMBER_OF_OUTPUT_WORDS-1;

    -- Master last and output signals
    signal tlast     : std_logic;
    signal tdata_out : std_logic_vector(31 downto 0);

    -- Input/ output buffers
    type IN_ARRAY  is array (0 to NUMBER_OF_INPUT_WORDS-1)  of std_logic_vector(31 downto 0);
    type OUT_ARRAY is array (0 to NUMBER_OF_OUTPUT_WORDS-1) of std_logic_vector(31 downto 0);
    signal in_buff  : IN_ARRAY  := ((others=> (others=>'0')));
    signal out_buff : OUT_ARRAY := ((others=> (others=>'0')));

    -- Mode, states and roundkey
    signal aes_mode : AES_MODE;
    signal inState  : STATE;
    signal inKey    : STATE;
    signal outState : STATE;
	
	-- AES Control signals
	signal aes_reset : std_logic;
	signal aes_done : std_logic;
    
    -- Converting aes_mode back to 32-bit for testing
    --signal outMode : std_logic_vector(31 downto 0);

begin

    -- Connect signals to output ports
    S_AXIS_TREADY <= '0' when axis_state = Write_Outputs else '1';
    M_AXIS_TVALID <= '1' when axis_state = Write_Outputs else '0';
    M_AXIS_TDATA  <= tdata_out when axis_state = Write_Outputs else ZEROS;
    M_AXIS_TLAST  <= tlast;
    
    -- Signal assignments to buffers
    -- First packet is aes mode
    aes_mode <= ENCRYPTION when (in_buff(0) = ZEROS) else DECRYPTION;
    -- Packets 2-5 for state
    inState <= (
        (in_buff(1)(31 downto 24), in_buff(1)(23 downto 16), in_buff(1)(15 downto 8), in_buff(1)(7 downto 0)),
        (in_buff(2)(31 downto 24), in_buff(2)(23 downto 16), in_buff(2)(15 downto 8), in_buff(2)(7 downto 0)),
        (in_buff(3)(31 downto 24), in_buff(3)(23 downto 16), in_buff(3)(15 downto 8), in_buff(3)(7 downto 0)),
        (in_buff(4)(31 downto 24), in_buff(4)(23 downto 16), in_buff(4)(15 downto 8), in_buff(4)(7 downto 0)));
    -- Packets 6-9 for round keys
    inKey <= (
        (in_buff(5)(31 downto 24), in_buff(5)(23 downto 16), in_buff(5)(15 downto 8), in_buff(5)(7 downto 0)),
        (in_buff(6)(31 downto 24), in_buff(6)(23 downto 16), in_buff(6)(15 downto 8), in_buff(6)(7 downto 0)),
        (in_buff(7)(31 downto 24), in_buff(7)(23 downto 16), in_buff(7)(15 downto 8), in_buff(7)(7 downto 0)),
        (in_buff(8)(31 downto 24), in_buff(8)(23 downto 16), in_buff(8)(15 downto 8), in_buff(8)(7 downto 0)));

    -- Convert aes_mode back to std_logic_vector
    --outMode <= ZEROS when (aes_mode = ENCRYPTION) else ONES;
	
	-- AES port map
	aes : entity work.aes port map (clk => ACLK, reset => aes_reset, done => aes_done, mode => aes_mode, i => inState, k => inKey, o => outState);

    state_machine : process (ACLK) is
    begin
    if rising_edge(ACLK) then
        if ARESETN = '0' then
            num_of_reads   <= NUMBER_OF_INPUT_WORDS-1;
            num_of_writes  <= NUMBER_OF_OUTPUT_WORDS-1;
--            num_of_process <= NUMBER_OF_INPUT_WORDS-1;
            in_buff        <= ((others=> (others=>'0')));
--            out_buff       <= ((others=> (others=>'0')));
            tlast          <= '0';
            axis_state     <= Idle;
			aes_reset      <= '1';
        else
            case axis_state is
                when Idle =>
                    if (S_AXIS_TVALID = '1') then
                        -- Capture first packet, as it will be lost in next cycle
                        in_buff(0) <= std_logic_vector(unsigned(S_AXIS_TDATA));
                        -- Start extra 1 forward
                        num_of_reads <= 1;
                        axis_state <= Read_Inputs;
                    end if;

                when Read_Inputs =>
                    in_buff(num_of_reads) <= std_logic_vector(unsigned(S_AXIS_TDATA));
                    -- Last packet or input buffer is full, else loop input S_AXIS_TDATA
                    if (S_AXIS_TLAST = '1' or num_of_reads = NUMBER_OF_INPUT_WORDS-1) then
                        -- Set starting address for output buffer
                        num_of_process <= 0;
						aes_reset <= '1';
                        axis_state <= Processing;
                    else
                        num_of_reads <= num_of_reads+1;
                    end if;
					
				when Processing =>
					-- Do the port map and actual processing of AES data here, once that is "complete" and we 
					-- get a done signal, move on to fill output buffer
					-- Turn off first round reset bit
					aes_reset <= '0';
					if (aes_done = '1') then
						-- Move to the next state, data is good
--						axis_state <= Fill_Output_Buffer;
--						aes_reset <= '1';
						num_of_writes <= 0;
						axis_state <= Send_Outputs;
					end if;

--                when Fill_Output_Buffer =>
                    -- *** 
                    -- Inject ECB IP connection here
                    -- 9 input words, 1 for AES_mode, 4 for state, 4 for round key
                    -- Technically only need 4 output words for the output state
                    -- ***
					
					-- Hold the AES module from processing
--					aes_reset <= '1';

                    -- Just copy input to states, then to output buffers, assuming processing is done at port map
--                    out_buff(num_of_process) <= outMode;
						-- Empty for now, will need to have a fancy way of sending the output when AES is don                    
--                    if (num_of_process = NUMBER_OF_OUTPUT_WORDS-1) then
--                        out_buff(num_of_process) <= inKey(3)(0) & inKey(3)(1) & inKey(3)(2) & inKey(3)(3);
--                        num_of_writes <= 0;
--                        axis_state <= Send_Outputs;
--                    else
--                        num_of_process <= num_of_process+1;
--                    end if;
                    
--                    out_buff(0) <= outState(0)(0) & outState(0)(1) & outState(0)(2) & outState(0)(3);
                    

                when Send_Outputs =>
                    -- Send packet out of master to DMA
                    tdata_out <= outState(num_of_writes)(0) & outState(num_of_writes)(1) & outState(num_of_writes)(2) & outState(num_of_writes)(3);
                    axis_state <= Write_Outputs;

                when Write_Outputs =>
                    if (M_AXIS_TREADY = '1') then
                        -- Signal DMA last packet
                        if (num_of_writes = NUMBER_OF_OUTPUT_WORDS-1) then
                            tlast <= '0';
                            aes_reset <= '1'; -- Reset the AES module
                            axis_state <= Idle;
                        else
                            -- Toggle TLAST on last transmitted word
                            if (num_of_writes = NUMBER_OF_OUTPUT_WORDS-2) then
                                tlast <= '1';
                            end if;
                            num_of_writes <= num_of_writes+1;
                            axis_state <= Send_Outputs;
                        end if;
                    end if;
            end case;
        end if;
    end if;
    end process state_machine;

end architecture arch_imp;
