library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.aes_package.all;

entity AES_ECB_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 4;

		-- Parameters of Axi Slave Bus Interface S00_AXIS
		C_S00_AXIS_TDATA_WIDTH	: integer	:= 32;

		-- Parameters of Axi Master Bus Interface M00_AXIS
		C_M00_AXIS_TDATA_WIDTH	: integer	:= 32;
		C_M00_AXIS_START_COUNT	: integer	:= 32
	);
	port (
		-- Users to add ports here

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
		s00_axi_rready	: in std_logic;

		-- Ports of Axi Slave Bus Interface S00_AXIS
		s00_axis_aclk	: in std_logic;
		s00_axis_aresetn	: in std_logic;
		s00_axis_tready	: out std_logic;
		s00_axis_tdata	: in std_logic_vector(C_S00_AXIS_TDATA_WIDTH-1 downto 0);
		s00_axis_tstrb	: in std_logic_vector((C_S00_AXIS_TDATA_WIDTH/8)-1 downto 0);
		s00_axis_tlast	: in std_logic;
		s00_axis_tvalid	: in std_logic;

		-- Ports of Axi Master Bus Interface M00_AXIS
		m00_axis_aclk	: in std_logic;
		m00_axis_aresetn	: in std_logic;
		m00_axis_tvalid	: out std_logic;
		m00_axis_tdata	: out std_logic_vector(C_M00_AXIS_TDATA_WIDTH-1 downto 0);
		m00_axis_tstrb	: out std_logic_vector((C_M00_AXIS_TDATA_WIDTH/8)-1 downto 0);
		m00_axis_tlast	: out std_logic;
		m00_axis_tready	: in std_logic
	);
end AES_ECB_v1_0;

architecture arch_imp of AES_ECB_v1_0 is

--	-- component declaration
--	component AES_ECB_v1_0_S00_AXI is
--		generic (
--		C_S_AXI_DATA_WIDTH	: integer	:= 32;
--		C_S_AXI_ADDR_WIDTH	: integer	:= 4
--		);
--		port (
--		S_AXI_ACLK	: in std_logic;
--		S_AXI_ARESETN	: in std_logic;
--		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
--		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
--		S_AXI_AWVALID	: in std_logic;
--		S_AXI_AWREADY	: out std_logic;
--		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
--		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
--		S_AXI_WVALID	: in std_logic;
--		S_AXI_WREADY	: out std_logic;
--		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
--		S_AXI_BVALID	: out std_logic;
--		S_AXI_BREADY	: in std_logic;
--		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
--		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
--		S_AXI_ARVALID	: in std_logic;
--		S_AXI_ARREADY	: out std_logic;
--		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
--		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
--		S_AXI_RVALID	: out std_logic;
--		S_AXI_RREADY	: in std_logic
--		);
--	end component AES_ECB_v1_0_S00_AXI;

	component AES_ECB_v1_0_S00_AXIS is
		generic (
		C_S_AXIS_TDATA_WIDTH	: integer	:= 32
		);
		port (
		S_AXIS_INPUT : out WORD_FIFO_TYPE;
		S_AXIS_FULL : out std_logic;
		S_AXIS_ACLK	: in std_logic;
		S_AXIS_ARESETN	: in std_logic;
		S_AXIS_TREADY	: out std_logic;
		S_AXIS_TDATA	: in std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		S_AXIS_TSTRB	: in std_logic_vector((C_S_AXIS_TDATA_WIDTH/8)-1 downto 0);
		S_AXIS_TLAST	: in std_logic;
		S_AXIS_TVALID	: in std_logic
		);
	end component AES_ECB_v1_0_S00_AXIS;

--	component AES_ECB_v1_0_M00_AXIS is
--		generic (
--		C_M_AXIS_TDATA_WIDTH	: integer	:= 32;
--		C_M_START_COUNT	: integer	:= 32
--		);
--		port (
--		M_AXIS_ACLK	: in std_logic;
--		M_AXIS_ARESETN	: in std_logic;
--		M_AXIS_TVALID	: out std_logic;
--		M_AXIS_TDATA	: out std_logic_vector(C_M_AXIS_TDATA_WIDTH-1 downto 0);
--		M_AXIS_TSTRB	: out std_logic_vector((C_M_AXIS_TDATA_WIDTH/8)-1 downto 0);
--		M_AXIS_TLAST	: out std_logic;
--		M_AXIS_TREADY	: in std_logic
--		);
--	end component AES_ECB_v1_0_M00_AXIS;
	
	signal m_axis_data : WORD_FIFO_TYPE;
	signal m_start : std_logic := '0';




	
	-- Total number of output data                                              
        constant NUMBER_OF_OUTPUT_WORDS : integer := 2;                                   
    
         -- function called clogb2 that returns an integer which has the   
         -- value of the ceiling of the log base 2.                              
        function clogb2 (bit_depth : integer) return integer is                  
             variable depth  : integer := bit_depth;                               
             variable count  : integer := 1;                                       
         begin                                                                   
              for clogb2 in 1 to bit_depth loop  -- Works for up to 32 bit integers
              if (bit_depth <= 2) then                                           
                count := 1;                                                      
              else                                                               
                if(depth <= 1) then                                              
                    count := count;                                                
                  else                                                             
                    depth := depth / 2;                                            
                  count := count + 1;                                            
                  end if;                                                          
                end if;                                                            
           end loop;                                                             
           return(count);                                                          
         end;                                                                    
    
         -- WAIT_COUNT_BITS is the width of the wait counter.                       
         constant  WAIT_COUNT_BITS  : integer := clogb2(C_M00_AXIS_START_COUNT-1);               
                                                                                          
        -- In this example, Depth of FIFO is determined by the greater of                 
        -- the number of input words and output words.                                    
        constant depth : integer := NUMBER_OF_OUTPUT_WORDS;                               
                                                                                          
        -- bit_num gives the minimum number of bits needed to address 'depth' size of FIFO
        constant bit_num : integer := clogb2(depth);                                      
                                                                                          
        -- Define the states of state machine                                             
        -- The control state machine oversees the writing of input streaming data to the FIFO,
        -- and outputs the streaming data from the FIFO                                   
        type state is ( IDLE,        -- This is the initial/idle state                    
                        INIT_COUNTER,  -- This state initializes the counter, once        
                                        -- the counter reaches C_M_START_COUNT count,     
                                        -- the state machine changes state to SEND_STREAM  
                        SEND_STREAM);  -- In this state the                               
                                     -- stream data is output through M_AXIS_TDATA        
        -- State variable                                                                 
        signal  mst_exec_state : state;                                                   
        -- Example design FIFO read pointer                                               
        signal read_pointer : integer range 0 to bit_num-1;                               
    
        -- AXI Stream internal signals
        --wait counter. The master waits for the user defined number of clock cycles before initiating a transfer.
        signal count    : std_logic_vector(WAIT_COUNT_BITS-1 downto 0);
        --streaming data valid
        signal axis_tvalid    : std_logic;
        --streaming data valid delayed by one clock cycle
        signal axis_tvalid_delay    : std_logic;
        --Last of the streaming data 
        signal axis_tlast    : std_logic;
        --Last of the streaming data delayed by one clock cycle
        signal axis_tlast_delay    : std_logic;
        --FIFO implementation signals
        signal stream_data_out    : std_logic_vector(C_M00_AXIS_TDATA_WIDTH-1 downto 0);
        signal tx_en    : std_logic;
        --The master has issued all the streaming data stored in FIFO
        signal tx_done    : std_logic;
	
	
	
	
	
	
	

begin

---- Instantiation of Axi Bus Interface S00_AXI
--AES_ECB_v1_0_S00_AXI_inst : AES_ECB_v1_0_S00_AXI
--	generic map (
--		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
--		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
--	)
--	port map (
--		S_AXI_ACLK	=> s00_axi_aclk,
--		S_AXI_ARESETN	=> s00_axi_aresetn,
--		S_AXI_AWADDR	=> s00_axi_awaddr,
--		S_AXI_AWPROT	=> s00_axi_awprot,
--		S_AXI_AWVALID	=> s00_axi_awvalid,
--		S_AXI_AWREADY	=> s00_axi_awready,
--		S_AXI_WDATA	=> s00_axi_wdata,
--		S_AXI_WSTRB	=> s00_axi_wstrb,
--		S_AXI_WVALID	=> s00_axi_wvalid,
--		S_AXI_WREADY	=> s00_axi_wready,
--		S_AXI_BRESP	=> s00_axi_bresp,
--		S_AXI_BVALID	=> s00_axi_bvalid,
--		S_AXI_BREADY	=> s00_axi_bready,
--		S_AXI_ARADDR	=> s00_axi_araddr,
--		S_AXI_ARPROT	=> s00_axi_arprot,
--		S_AXI_ARVALID	=> s00_axi_arvalid,
--		S_AXI_ARREADY	=> s00_axi_arready,
--		S_AXI_RDATA	=> s00_axi_rdata,
--		S_AXI_RRESP	=> s00_axi_rresp,
--		S_AXI_RVALID	=> s00_axi_rvalid,
--		S_AXI_RREADY	=> s00_axi_rready
--	);

-- Instantiation of Axi Bus Interface S00_AXIS
AES_ECB_v1_0_S00_AXIS_inst : AES_ECB_v1_0_S00_AXIS
	generic map (
		C_S_AXIS_TDATA_WIDTH	=> C_S00_AXIS_TDATA_WIDTH
	)
	port map (
	    S_AXIS_INPUT => m_axis_data,
	    S_AXIS_FULL => m_start,
		S_AXIS_ACLK	=> s00_axis_aclk,
		S_AXIS_ARESETN	=> s00_axis_aresetn,
		S_AXIS_TREADY	=> s00_axis_tready,
		S_AXIS_TDATA	=> s00_axis_tdata,
		S_AXIS_TSTRB	=> s00_axis_tstrb,
		S_AXIS_TLAST	=> s00_axis_tlast,
		S_AXIS_TVALID	=> s00_axis_tvalid
	);

---- Instantiation of Axi Bus Interface M00_AXIS
--AES_ECB_v1_0_M00_AXIS_inst : AES_ECB_v1_0_M00_AXIS
--	generic map (
--		C_M_AXIS_TDATA_WIDTH	=> C_M00_AXIS_TDATA_WIDTH,
--		C_M_START_COUNT	=> C_M00_AXIS_START_COUNT
--	)
--	port map (
--		M_AXIS_ACLK	=> m00_axis_aclk,
--		M_AXIS_ARESETN	=> m00_axis_aresetn,
--		M_AXIS_TVALID	=> m00_axis_tvalid,
--		M_AXIS_TDATA	=> m00_axis_tdata,
--		M_AXIS_TSTRB	=> m00_axis_tstrb,
--		M_AXIS_TLAST	=> m00_axis_tlast,
--		M_AXIS_TREADY	=> m00_axis_tready
--	);

	-- Add user logic here
	
--	process(m00_axis_aclk)
--        begin
--            if (rising_edge (m00_axis_aclk)) and (m_start = '1') then

--                  s00_axis_tready <= m00_axis_tready;
                           
--                m00_axis_tvalid <= s00_axis_tvalid;
--                m00_axis_tdata <= m_axis_data(0);
--                m00_axis_tstrb <= s00_axis_tstrb;
--                m00_axis_tlast <= s00_axis_tlast;
                
--            end if;
--      end process;

-- I/O Connections assignments
	m00_axis_tvalid	<= axis_tvalid_delay;
	m00_axis_tdata	<= stream_data_out;
	m00_axis_tlast	<= axis_tlast_delay;
	m00_axis_tstrb	<= (others => '1');
	
--	m00_axis_tready <= m_start;


	-- Control state machine implementation                                               
	process(m00_axis_aclk)                                                                        
	begin                                                                                       
	  if (rising_edge (m00_axis_aclk)) then                                                       
	    if(m00_axis_aresetn = '0') then                                                           
	      -- Synchronous reset (active low)                                                     
	      mst_exec_state      <= IDLE;                                                          
	      count <= (others => '0');                                                             
	    else                                                                                    
	      case (mst_exec_state) is                                                              
	        when IDLE     =>                                                                    
	          -- The slave starts accepting tdata when                                          
	          -- there tvalid is asserted to mark the                                           
	          -- presence of valid streaming data                                               
	          --if (count = "0")then                                                            
	            mst_exec_state <= INIT_COUNTER;                                                 
	          --else                                                                              
	          --  mst_exec_state <= IDLE;                                                         
	          --end if;                                                                           
	                                                                                            
	          when INIT_COUNTER =>                                                              
	            -- This state is responsible to wait for user defined C_M_START_COUNT           
	            -- number of clock cycles.                                                      
	            if ( count = std_logic_vector(to_unsigned((C_M00_AXIS_START_COUNT - 1), WAIT_COUNT_BITS))) then
	              mst_exec_state  <= SEND_STREAM;                                               
	            else                                                                            
	              count <= std_logic_vector (unsigned(count) + 1);                              
	              mst_exec_state  <= INIT_COUNTER;                                              
	            end if;                                                                         
	                                                                                            
	        when SEND_STREAM  =>                                                                
	          -- The example design streaming master functionality starts                       
	          -- when the master drives output tdata from the FIFO and the slave                
	          -- has finished storing the S_AXIS_TDATA                                          
	          if (tx_done = '1') then                                                           
	            mst_exec_state <= IDLE;                                                         
	          else                                                                              
	            mst_exec_state <= SEND_STREAM;                                                  
	          end if;                                                                           
	                                                                                            
	        when others    =>                                                                   
	          mst_exec_state <= IDLE;                                                           
	                                                                                            
	      end case;                                                                             
	    end if;                                                                                 
	  end if;                                                                                   
	end process;                                                                                


	--tvalid generation
	--axis_tvalid is asserted when the control state machine's state is SEND_STREAM and
	--number of output streaming data is less than the NUMBER_OF_OUTPUT_WORDS.
	axis_tvalid <= '1' when ((mst_exec_state = SEND_STREAM) and (read_pointer < NUMBER_OF_OUTPUT_WORDS)) else '0';
	                                                                                               
	-- AXI tlast generation                                                                        
	-- axis_tlast is asserted number of output streaming data is NUMBER_OF_OUTPUT_WORDS-1          
	-- (0 to NUMBER_OF_OUTPUT_WORDS-1)                                                             
	axis_tlast <= '1' when (read_pointer = NUMBER_OF_OUTPUT_WORDS-1) else '0';                     
	                                                                                               
	-- Delay the axis_tvalid and axis_tlast signal by one clock cycle                              
	-- to match the latency of M_AXIS_TDATA                                                        
	process(m00_axis_aclk)                                                                           
	begin                                                                                          
	  if (rising_edge (m00_axis_aclk)) then                                                          
	    if(m00_axis_aresetn = '0') then                                                              
	      axis_tvalid_delay <= '0';                                                                
	      axis_tlast_delay <= '0';                                                                 
	    else                                                                                       
	      axis_tvalid_delay <= axis_tvalid;                                                        
	      axis_tlast_delay <= axis_tlast;                                                          
	    end if;                                                                                    
	  end if;                                                                                      
	end process;                                                                                   


	--read_pointer pointer

	process(m00_axis_aclk)                                                       
	begin                                                                            
	  if (rising_edge (m00_axis_aclk)) then                                            
	    if(m00_axis_aresetn = '0') then                                                
	      read_pointer <= 0;                                                         
	      tx_done  <= '0';                                                           
	    else                                                                         
	      if (read_pointer <= NUMBER_OF_OUTPUT_WORDS-1) then                         
	        if (tx_en = '1') then                                                    
	          -- read pointer is incremented after every read from the FIFO          
	          -- when FIFO read signal is enabled.                                   
	          read_pointer <= read_pointer + 1;                                      
	          tx_done <= '0';                                                        
	        end if;                                                                  
	      elsif (read_pointer = NUMBER_OF_OUTPUT_WORDS) then                         
	        -- tx_done is asserted when NUMBER_OF_OUTPUT_WORDS numbers of streaming data
	        -- has been out.                                                         
	        tx_done <= '1';                                                          
	      end  if;                                                                   
	    end  if;                                                                     
	  end  if;                                                                       
	end process;                                                                     


	--FIFO read enable generation 

	tx_en <= m_start and axis_tvalid;                                   
	                                                                                
	-- FIFO Implementation                                                          
	                                                                                
	-- Streaming output data is read from FIFO                                      
	  process(m00_axis_aclk)                                                          
	  variable  sig_one : integer := 1;                                             
	  begin                                                                         
	    if (rising_edge (m00_axis_aclk)) then                                         
	      if(m00_axis_aresetn = '0') then                                             
	    	stream_data_out <= std_logic_vector(to_unsigned(sig_one,C_M00_AXIS_TDATA_WIDTH));  
	      elsif (tx_en = '1') then -- && M_AXIS_TSTRB(byte_index)                   
	        stream_data_out <= m_axis_data(0);
	      end if;                                                                   
	     end if;                                                                    
	   end process;             
      
	
	-- User logic ends

end arch_imp;
