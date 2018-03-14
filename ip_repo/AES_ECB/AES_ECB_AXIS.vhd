------------------------------------------------------------------------------
-- AES_ECB_AXIS
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

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
   constant NUMBER_OF_INPUT_WORDS  : natural := 8;
   constant NUMBER_OF_OUTPUT_WORDS : natural := 8;
   -- Byte of zeros
   constant zeros : std_logic_vector(31 downto 0) := (others=>'0');

   -- AXIS states
   type STATE_TYPE is (Idle, Read_Inputs, Processing, Send_Outputs, Write_Outputs);
   signal state : STATE_TYPE;

   -- Counters to store the number inputs/ outputs written
   signal num_of_reads   : natural range 0 to NUMBER_OF_INPUT_WORDS-1;
   signal num_of_writes  : natural range 0 to NUMBER_OF_OUTPUT_WORDS-1;
   signal num_of_process : natural range 0 to NUMBER_OF_INPUT_WORDS-1;

   -- Master last and output signals
   signal tlast     : std_logic;
   signal tdata_out : std_logic_vector(31 downto 0);

   -- Input/ output buffers
   type out_array is array (0 to NUMBER_OF_OUTPUT_WORDS-1) of std_logic_vector(31 downto 0);
   type in_array  is array (0 to NUMBER_OF_INPUT_WORDS-1)  of std_logic_vector(31 downto 0);

   signal out_buff : out_array := ((others=> (others=>'0')));
   signal in_buff  : in_array  := ((others=> (others=>'0')));

begin

    -- Connect signals to output ports
    S_AXIS_TREADY <= '0' when state = Write_Outputs else '1';
    M_AXIS_TVALID <= '1' when state = Write_Outputs else '0';
    M_AXIS_TDATA  <= tdata_out when state = Write_Outputs else zeros;
    M_AXIS_TLAST  <= tlast;

    state_machine : process (ACLK) is
    begin
    if rising_edge(ACLK) then
        if ARESETN = '0' then
            num_of_reads   <= 0;
            num_of_writes  <= 0;
            num_of_process <= 0;
            in_buff        <= ((others=> (others=>'0')));
            out_buff       <= ((others=> (others=>'0')));
            tlast          <= '0';
            state          <= Idle;
        else
            case state is
                when Idle =>
                    if (S_AXIS_TVALID = '1') then
                        -- Capture first packet, as it will be lost in next cycle
                        in_buff(NUMBER_OF_INPUT_WORDS-1) <= std_logic_vector(unsigned(S_AXIS_TDATA));
                        -- Start extra 1 less
                        num_of_reads <= NUMBER_OF_INPUT_WORDS-2;
                        state <= Read_Inputs;
                    end if;

                when Read_Inputs =>
                    in_buff(num_of_reads) <= std_logic_vector(unsigned(S_AXIS_TDATA));
                    -- Last packet or input buffer is full, else loop input S_AXIS_TDATA
                    if (S_AXIS_TLAST = '1' or num_of_reads = 0) then
                        -- Set starting address for output buffer
                        num_of_process <= NUMBER_OF_INPUT_WORDS-1;
                        state <= Processing;
                    else
                        num_of_reads <= num_of_reads-1;
                    end if;

                when Processing =>
                    -- *** 
                    -- Inject ECB IP connection here
                    -- ***

                    -- Just copy input to output buffer (could reuse in_buff?)
                    out_buff(num_of_process) <= in_buff(num_of_process);
                    if (num_of_process = 0) then
                        num_of_writes <= NUMBER_OF_OUTPUT_WORDS-1;
                        state <= Send_Outputs;
                    else
                        num_of_process <= num_of_process-1;
                    end if;

                when Send_Outputs =>
                    -- Send packet out of master to DMA
                    tdata_out <= out_buff(num_of_writes);
                    state <= Write_Outputs;

                when Write_Outputs =>
                    if (M_AXIS_TREADY = '1') then
                        -- Signal DMA last packet
                        if (num_of_writes = 0) then
                            tlast <= '0';
                            state <= Idle;
                        else
                            -- Toggle TLAST on last transmitted word
                            if (num_of_writes = 1) then
                                tlast <= '1';
                            end if;
                            num_of_writes <= num_of_writes-1;
                            state <= Send_Outputs;
                        end if;
                    end if;
            end case;
        end if;
    end if;
    end process state_machine;

end architecture arch_imp;
