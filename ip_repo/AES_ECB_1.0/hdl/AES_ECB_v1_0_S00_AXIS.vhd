library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.aes_package.all;

entity AES_ECB_v1_0_S00_AXIS is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- AXI4Stream sink: Data Width
		C_S_AXIS_TDATA_WIDTH	: integer	:= 32
	);
	port (
		-- Users to add ports here

		S_AXIS_INPUT_KEY : out STATE;
        S_AXIS_INPUT_STATE : out STATE;
		
		-- User ports ends
		-- Do not modify the ports beyond this line

		-- AXI4Stream sink: Clock
		S_AXIS_ACLK	: in std_logic;
		-- AXI4Stream sink: Reset
		S_AXIS_ARESETN	: in std_logic;
		-- Ready to accept data in
		S_AXIS_TREADY	: out std_logic;
		-- Data in
		S_AXIS_TDATA	: in std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		-- Byte qualifier
		S_AXIS_TSTRB	: in std_logic_vector((C_S_AXIS_TDATA_WIDTH/8)-1 downto 0);
		-- Indicates boundary of last packet
		S_AXIS_TLAST	: in std_logic;
		-- Data is in valid
		S_AXIS_TVALID	: in std_logic
	);
end AES_ECB_v1_0_S00_AXIS;

architecture arch_imp of AES_ECB_v1_0_S00_AXIS is
	-- function called clogb2 that returns an integer which has the 
	-- value of the ceiling of the log base 2.
	function clogb2 (bit_depth : integer) return integer is 
	variable depth  : integer := bit_depth;
	  begin
	    if (depth = 0) then
	      return(0);
	    else
	      for clogb2 in 1 to bit_depth loop  -- Works for up to 32 bit integers
	        if(depth <= 1) then 
	          return(clogb2);      
	        else
	          depth := depth / 2;
	        end if;
	      end loop;
	    end if;
	end;    

	-- Total number of input data.
	constant NUMBER_OF_INPUT_BYTES : integer := 32;
	-- bit_num gives the minimum number of bits needed to address 'NUMBER_OF_INPUT_BYTES' size of FIFO.
	constant bit_num  : integer := clogb2(NUMBER_OF_INPUT_BYTES-1);
	-- Define the states of state machine
	-- The control state machine oversees the writing of input streaming data to the FIFO,
	-- and outputs the streaming data from the FIFO
	type state is ( IDLE,        -- This is the initial/idle state 
	                WRITE_FIFO); -- In this state FIFO is written with the
	                             -- input stream data S_AXIS_TDATA 
	signal axis_tready	: std_logic;
	-- State variable
	signal  mst_exec_state : state;  
	-- FIFO implementation signals
	signal  byte_index : integer;    
	-- FIFO write enable
	signal fifo_wren : std_logic;
	-- FIFO full flag
	signal fifo_full_flag : std_logic;
	-- FIFO write pointer
	signal write_pointer : integer range 0 to bit_num-1 ;
	-- sink has accepted all the streaming data and stored in FIFO
	signal writes_done : std_logic;

	type BYTE_FIFO_TYPE is array (0 to (NUMBER_OF_INPUT_BYTES-1)) of std_logic_vector(((C_S_AXIS_TDATA_WIDTH/4)-1)downto 0);
	
	-- Add user components here
    
    -- Input buffer
    signal stream_data_fifo : BYTE_FIFO_TYPE;

    -- User components ends

begin
	-- I/O Connections assignments

	S_AXIS_TREADY	<= axis_tready;
	-- Control state machine implementation
	process(S_AXIS_ACLK)
	begin
	  if (rising_edge (S_AXIS_ACLK)) then
	    if(S_AXIS_ARESETN = '0') then
	      -- Synchronous reset (active low)
	      mst_exec_state      <= IDLE;
	    else
	      case (mst_exec_state) is
	        when IDLE     => 
	          -- The sink starts accepting tdata when 
	          -- there tvalid is asserted to mark the
	          -- presence of valid streaming data 
	          if (S_AXIS_TVALID = '1')then
	            mst_exec_state <= WRITE_FIFO;
	          else
	            mst_exec_state <= IDLE;
	          end if;
	      
	        when WRITE_FIFO => 
	          -- When the sink has accepted all the streaming input data,
	          -- the interface swiches functionality to a streaming master
	          if (writes_done = '1') then
	            mst_exec_state <= IDLE;
	          else
	            -- The sink accepts and stores tdata 
	            -- into FIFO
	            mst_exec_state <= WRITE_FIFO;
	          end if;
	        
	        when others    => 
	          mst_exec_state <= IDLE;
	        
	      end case;
	    end if;  
	  end if;
	end process;
	-- AXI Streaming Sink 
	-- 
	-- The example design sink is always ready to accept the S_AXIS_TDATA  until
	-- the FIFO is not filled with NUMBER_OF_INPUT_BYTES number of input words.
	axis_tready <= '1' when ((mst_exec_state = WRITE_FIFO) and (write_pointer <= NUMBER_OF_INPUT_BYTES-1)) else '0';

	process(S_AXIS_ACLK)
	begin
	  if (rising_edge (S_AXIS_ACLK)) then
	    if(S_AXIS_ARESETN = '0') then
	      write_pointer <= 0;
	      writes_done <= '0';
	    else
	      if (write_pointer <= NUMBER_OF_INPUT_BYTES-1) then
	        if (fifo_wren = '1') then
	          -- write pointer is incremented after every write to the FIFO
	          -- when FIFO write signal is enabled.
	          write_pointer <= write_pointer + 1;
	          writes_done <= '0';
	        end if;
	        if ((write_pointer = NUMBER_OF_INPUT_BYTES-1) or S_AXIS_TLAST = '1') then
	          -- reads_done is asserted when NUMBER_OF_INPUT_BYTES numbers of streaming data 
	          -- has been written to the FIFO which is also marked by S_AXIS_TLAST(kept for optional usage).
	          writes_done <= '1';
	        end if;
	      end  if;
	    end if;
	  end if;
	end process;

	-- FIFO write enable generation
	fifo_wren <= S_AXIS_TVALID and axis_tready;

	-- FIFO Implementation
	 FIFO_GEN: for byte_index in 0 to (C_S_AXIS_TDATA_WIDTH/8-1) generate

	 begin   
	  -- Streaming input data is stored in FIFO
	  process(S_AXIS_ACLK)
	  begin
	    if (rising_edge (S_AXIS_ACLK)) then
	      if (fifo_wren = '1') then
	        stream_data_fifo(write_pointer) <= S_AXIS_TDATA((byte_index*8+7) downto (byte_index*8));
	      end if;  
	    end  if;
	  end process;

	end generate FIFO_GEN;

	-- Add user logic here

	-- Input Round Key
    S_AXIS_INPUT_KEY <= (
        (stream_data_fifo(0),  stream_data_fifo(1),  stream_data_fifo(2),  stream_data_fifo(3)),
        (stream_data_fifo(4),  stream_data_fifo(5),  stream_data_fifo(6),  stream_data_fifo(7)),
        (stream_data_fifo(8),  stream_data_fifo(9),  stream_data_fifo(10), stream_data_fifo(11)),
        (stream_data_fifo(12), stream_data_fifo(13), stream_data_fifo(14), stream_data_fifo(15))
    );
    
    -- Input State
    S_AXIS_INPUT_STATE <= (
        (stream_data_fifo(16), stream_data_fifo(17), stream_data_fifo(18), stream_data_fifo(19)),
        (stream_data_fifo(20), stream_data_fifo(21), stream_data_fifo(22), stream_data_fifo(23)),
        (stream_data_fifo(24), stream_data_fifo(25), stream_data_fifo(26), stream_data_fifo(27)),
        (stream_data_fifo(28), stream_data_fifo(29), stream_data_fifo(30), stream_data_fifo(31))
    );

	-- User logic ends

end arch_imp;
