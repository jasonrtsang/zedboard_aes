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
    constant NUMBER_OF_OUTPUT_WORDS : natural := 9;
    -- Byte of zeros
    constant ZEROS : std_logic_vector(31 downto 0) := (others=>'0');
    constant ONES : std_logic_vector(31 downto 0) := (others=>'1');

    -- AXIS states
    type AXIS_STATE_TYPE is (Idle, Read_Inputs, Processing, Send_Outputs, Write_Outputs);
    signal axis_state : AXIS_STATE_TYPE;

    -- Counters to store the number inputs/ outputs written
    signal num_of_reads   : natural range 0 to NUMBER_OF_INPUT_WORDS-1;
    signal num_of_writes  : natural range 0 to NUMBER_OF_OUTPUT_WORDS-1;
    signal num_of_process : natural range 0 to NUMBER_OF_INPUT_WORDS-1;

    -- Master last and output signals
    signal tlast     : std_logic;
    signal tdata_out : std_logic_vector(31 downto 0);

    -- Input/ output buffers
    type OUT_ARRAY is array (0 to NUMBER_OF_OUTPUT_WORDS-1) of std_logic_vector(31 downto 0);
    type IN_ARRAY  is array (0 to NUMBER_OF_INPUT_WORDS-1)  of std_logic_vector(31 downto 0);
    signal out_buff : OUT_ARRAY := ((others=> (others=>'0')));
    signal in_buff  : IN_ARRAY  := ((others=> (others=>'0')));

    -- Mode, states and roundkey
    signal aes_mode : AES_MODE;
    signal inState  : STATE;
    signal inKey    : STATE;
    signal outState : STATE;
    
    signal outMode : std_logic_vector(31 downto 0);

begin

    -- Connect signals to output ports
    S_AXIS_TREADY <= '0' when axis_state = Write_Outputs else '1';
    M_AXIS_TVALID <= '1' when axis_state = Write_Outputs else '0';
    M_AXIS_TDATA  <= tdata_out when axis_state = Write_Outputs else ZEROS;
    M_AXIS_TLAST  <= tlast;
    
    aes_mode <= ENCRYPTION when (in_buff(8) = ZEROS) else DECRYPTION;
    inState <= (
        (in_buff(7)(31 downto 24), in_buff(7)(23 downto 16), in_buff(7)(15 downto 8), in_buff(7)(7 downto 0)),
        (in_buff(6)(31 downto 24), in_buff(6)(23 downto 16), in_buff(6)(15 downto 8), in_buff(6)(7 downto 0)),
        (in_buff(5)(31 downto 24), in_buff(5)(23 downto 16), in_buff(5)(15 downto 8), in_buff(5)(7 downto 0)),
        (in_buff(4)(31 downto 24), in_buff(4)(23 downto 16), in_buff(4)(15 downto 8), in_buff(4)(7 downto 0)));
    inKey <= (
        (in_buff(3)(31 downto 24), in_buff(3)(23 downto 16), in_buff(3)(15 downto 8), in_buff(3)(7 downto 0)),
        (in_buff(2)(31 downto 24), in_buff(2)(23 downto 16), in_buff(2)(15 downto 8), in_buff(2)(7 downto 0)),
        (in_buff(1)(31 downto 24), in_buff(1)(23 downto 16), in_buff(1)(15 downto 8), in_buff(1)(7 downto 0)),
        (in_buff(0)(31 downto 24), in_buff(0)(23 downto 16), in_buff(0)(15 downto 8), in_buff(0)(7 downto 0)));

    outMode <= ZEROS when (aes_mode = ENCRYPTION) else ONES;

    state_machine : process (ACLK) is
    begin
    if rising_edge(ACLK) then
        if ARESETN = '0' then
            num_of_reads   <= 0;
            num_of_writes  <= 0;
            num_of_process <= NUMBER_OF_INPUT_WORDS-1;
            in_buff        <= ((others=> (others=>'0')));
            out_buff       <= ((others=> (others=>'0')));
            tlast          <= '0';
            axis_state     <= Idle;
        else
            case axis_state is
                when Idle =>
                    if (S_AXIS_TVALID = '1') then
                        -- Capture first packet, as it will be lost in next cycle
                        in_buff(NUMBER_OF_INPUT_WORDS-1) <= std_logic_vector(unsigned(S_AXIS_TDATA));
                        -- Start extra 1 less
                        num_of_reads <= NUMBER_OF_INPUT_WORDS-2;
                        axis_state <= Read_Inputs;
                    end if;

                when Read_Inputs =>
                    in_buff(num_of_reads) <= std_logic_vector(unsigned(S_AXIS_TDATA));
                    -- Last packet or input buffer is full, else loop input S_AXIS_TDATA
                    if (S_AXIS_TLAST = '1' or num_of_reads = 0) then
                        -- Set starting address for output buffer
                        num_of_process <= 0;
                        axis_state <= Processing;
                    else
                        num_of_reads <= num_of_reads-1;
                    end if;

                when Processing =>
                    -- *** 
                    -- Inject ECB IP connection here
                    -- 9 input words, 1 for AES_mode, 4 for state, 4 for round key
                    -- Technically only need 4 output words for the output state
                    -- ***

                    -- Just copy input to output buffer (could reuse in_buff?)
                    if (num_of_process = 0) then
                        out_buff(num_of_process) <= outMode;
                        num_of_process <= num_of_process+1;
                    elsif (num_of_process >= 1 and num_of_process <= 4) then
                        out_buff(num_of_process) <= inState(num_of_process-1)(0) & inState(num_of_process-1)(1) & inState(num_of_process-1)(2) & inState(num_of_process-1)(3);
                        num_of_process <= num_of_process+1;
                    elsif (num_of_process >= 5 and num_of_process <= 7) then
                        out_buff(num_of_process) <= inKey(num_of_process-5)(0) & inKey(num_of_process-5)(1) & inKey(num_of_process-5)(2) & inKey(num_of_process-5)(3);
                        num_of_process <= num_of_process+1;
                    elsif (num_of_process = 8) then
                        out_buff(num_of_process) <= inKey(3)(0) & inKey(3)(1) & inKey(3)(2) & inKey(3)(3);
                        num_of_writes <= NUMBER_OF_OUTPUT_WORDS-1;
                        axis_state <= Send_Outputs;
                    else
                        axis_state <= Idle;
                    end if;

                when Send_Outputs =>
                    -- Send packet out of master to DMA
                    tdata_out <= out_buff(num_of_writes);
                    axis_state <= Write_Outputs;

                when Write_Outputs =>
                    if (M_AXIS_TREADY = '1') then
                        -- Signal DMA last packet
                        if (num_of_writes = 0) then
                            tlast <= '0';
                            axis_state <= Idle;
                        else
                            -- Toggle TLAST on last transmitted word
                            if (num_of_writes = 1) then
                                tlast <= '1';
                            end if;
                            num_of_writes <= num_of_writes-1;
                            axis_state <= Send_Outputs;
                        end if;
                    end if;
            end case;
        end if;
    end if;
    end process state_machine;

end architecture arch_imp;
