library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.aes_package.all;

entity AES_Full_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 6
	);
	port (
		-- Users to add ports here
		
		-- Ports required for DMA transfer
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
        M_AXIS_TREADY   : in  std_logic;                     -- Connected slave device is ready to accept data out

		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S00_AXI
		s00_axi_aclk	: in std_logic;
		s00_axi_aresetn	: in std_logic;
		s00_axi_awaddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_awprot	: in std_logic_vector(2 downto 0);
		s00_axi_awvalid	: in std_logic;
		s00_axi_awready	: out std_logic;
		s00_axi_wdata	: in std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_wstrb	: in std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
		s00_axi_wvalid	: in std_logic;
		s00_axi_wready	: out std_logic;
		s00_axi_bresp	: out std_logic_vector(1 downto 0);
		s00_axi_bvalid	: out std_logic;
		s00_axi_bready	: in std_logic;
		s00_axi_araddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_arprot	: in std_logic_vector(2 downto 0);
		s00_axi_arvalid	: in std_logic;
		s00_axi_arready	: out std_logic;
		s00_axi_rdata	: out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_rresp	: out std_logic_vector(1 downto 0);
		s00_axi_rvalid	: out std_logic;
		s00_axi_rready	: in std_logic
	);
end AES_Full_v1_0;

architecture arch_imp of AES_Full_v1_0 is

-- User Signals 

    signal key_state : STATE; -- for key
	signal iv_state : STATE; -- For initialization vector
    signal mode_select_reg : std_logic_vector(31 downto 0); -- The mode register
    
    -- Number of input/ output data packets
    constant NUMBER_OF_INPUT_WORDS  : natural := 4;
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


	-- component declaration
	component AES_Full_v1_0_S00_AXI is
		generic (
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 6
		);
		port (
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic;
		-- User added ports
		AES_KEY         : out STATE;
		AES_IV          : out STATE;
		AES_MODE_REG    : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0)
		);
	end component AES_Full_v1_0_S00_AXI;

begin

-- Instantiation of Axi Bus Interface S00_AXI
AES_Full_v1_0_S00_AXI_inst : AES_Full_v1_0_S00_AXI
	generic map (
		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
	)
	port map (
	-- need to map key, iv, and mode to this layer
		S_AXI_ACLK	=> s00_axi_aclk,
		S_AXI_ARESETN	=> s00_axi_aresetn,
		S_AXI_AWADDR	=> s00_axi_awaddr,
		S_AXI_AWPROT	=> s00_axi_awprot,
		S_AXI_AWVALID	=> s00_axi_awvalid,
		S_AXI_AWREADY	=> s00_axi_awready,
		S_AXI_WDATA	=> s00_axi_wdata,
		S_AXI_WSTRB	=> s00_axi_wstrb,
		S_AXI_WVALID	=> s00_axi_wvalid,
		S_AXI_WREADY	=> s00_axi_wready,
		S_AXI_BRESP	=> s00_axi_bresp,
		S_AXI_BVALID	=> s00_axi_bvalid,
		S_AXI_BREADY	=> s00_axi_bready,
		S_AXI_ARADDR	=> s00_axi_araddr,
		S_AXI_ARPROT	=> s00_axi_arprot,
		S_AXI_ARVALID	=> s00_axi_arvalid,
		S_AXI_ARREADY	=> s00_axi_arready,
		S_AXI_RDATA	=> s00_axi_rdata,
		S_AXI_RRESP	=> s00_axi_rresp,
		S_AXI_RVALID	=> s00_axi_rvalid,
		S_AXI_RREADY	=> s00_axi_rready,
		-- User custom signals here
		AES_KEY       => key_state,
		AES_IV       => iv_state,
		AES_MODE_REG => mode_select_reg
	);

	-- Add user logic here
	
	-- Connect signals to output ports
        S_AXIS_TREADY <= '0' when axis_state = Write_Outputs else '1';
        M_AXIS_TVALID <= '1' when axis_state = Write_Outputs else '0';
        M_AXIS_TDATA  <= tdata_out when axis_state = Write_Outputs else ZEROS;
        M_AXIS_TLAST  <= tlast;
        
        -- Signal assignments to buffers
        -- Read mode from registers
        aes_mode <= ENCRYPTION when (mode_select_reg = ZEROS) else DECRYPTION;
        -- Packets 2-5 for state
        inState <= (
            (in_buff(0)(31 downto 24), in_buff(0)(23 downto 16), in_buff(0)(15 downto 8), in_buff(0)(7 downto 0)),
            (in_buff(1)(31 downto 24), in_buff(1)(23 downto 16), in_buff(1)(15 downto 8), in_buff(1)(7 downto 0)),
            (in_buff(2)(31 downto 24), in_buff(2)(23 downto 16), in_buff(2)(15 downto 8), in_buff(2)(7 downto 0)),
            (in_buff(3)(31 downto 24), in_buff(3)(23 downto 16), in_buff(3)(15 downto 8), in_buff(3)(7 downto 0)));
        -- Key is stored in registers
        inKey <= key_state;
    
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
    --                        axis_state <= Fill_Output_Buffer;
    --                        aes_reset <= '1';
                            num_of_writes <= 0;
                            axis_state <= Send_Outputs;
                        end if;
                        
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

	-- User logic ends

end arch_imp;
