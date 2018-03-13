library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity ECB_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 4
	);
	port (
		-- Users to add ports here

		S_AXIS_TREADY	: out std_logic;
        S_AXIS_TDATA    : in std_logic_vector(31 downto 0);
        S_AXIS_TLAST    : in std_logic;
        S_AXIS_TVALID   : in std_logic;
        M_AXIS_TVALID   : out std_logic;
        M_AXIS_TDATA    : out std_logic_vector(31 downto 0);
        M_AXIS_TLAST    : out std_logic;
        M_AXIS_TREADY   : in std_logic;
		
		ACLK : in std_logic;
		ARESETN : in std_logic;

		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S00_AXI
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
end ECB_v1_0;

architecture arch_imp of ECB_v1_0 is

	-- Users to add components here

    -- Total number of input data.
    constant NUMBER_OF_INPUT_WORDS  : natural := 4;
    
    -- Total number of output data
    constant NUMBER_OF_OUTPUT_WORDS : natural := 4;
    
    -- Total number of computation
    constant NUMBER_OF_COMPUTATION : natural := 4;
    
    type STATE_TYPE is (Idle, Assemble, Read_Inputs, Computing, Transmit, Write_Outputs);
    signal state : STATE_TYPE;
    
    -- Counters to store the number inputs read & outputs written
    signal nr_of_reads  : natural range 0 to NUMBER_OF_INPUT_WORDS - 1;
    signal nr_of_writes : natural range 0 to NUMBER_OF_OUTPUT_WORDS - 1;
    signal nr_of_computes : natural range 0 to NUMBER_OF_COMPUTATION;
    
    -- For s-box
    signal enable: std_logic := '0';
    signal valid: std_logic;
    signal input: std_logic_vector(31 downto 0) := (others => '0');
    signal output: std_logic_vector(31 downto 0);
    type out_array is array (1 to 4) of std_logic_vector(31 downto 0);
    type in_array is array (0 to 3) of std_logic_vector(31 downto 0);
    signal output_mem: out_array := ((others=> (others=>'0')));
    signal input_mem: in_array := ((others=> (others=>'0')));
    signal key: in_array := ((others=> (others=>'0')));
    
    -- User components ends

	-- component declaration
	component ECB_v1_0_S00_AXI is
		generic (
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 4
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
		S_AXI_RREADY	: in std_logic
		);
	end component ECB_v1_0_S00_AXI;

begin

-- Instantiation of Axi Bus Interface S00_AXI
ECB_v1_0_S00_AXI_inst : ECB_v1_0_S00_AXI
	generic map (
		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
	)
	port map (
		S_AXI_ACLK	=> ACLK,
		S_AXI_ARESETN	=> ARESETN,
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
		S_AXI_RREADY	=> s00_axi_rready
	);

	-- Add user logic here
	
    S_AXIS_TREADY  <= '1' when state = Read_Inputs else '0';
    M_AXIS_TVALID <= '1' when state = Write_Outputs else '0';
    M_AXIS_TLAST <= '1' when (state = Write_Outputs and nr_of_writes = 1) else '0';
        
    process (ACLK) is
       begin  -- process The_SW_accelerator
        if ACLK'event and ACLK = '1' then     -- Rising clock edge
          if ARESETN = '0' then               -- Synchronous reset (active low)
            -- CAUTION: make sure your reset polarity is consistent with the
            -- system reset polarity
            state        <= Idle;
            nr_of_reads  <= 0;
            nr_of_writes <= 0;
            nr_of_computes <= 0;
          else
            case state is
              when Idle =>            
                if (S_AXIS_TVALID = '1') then
                  state       <= Read_Inputs;
                  nr_of_reads <= NUMBER_OF_INPUT_WORDS - 1;
                end if;
    
              when Read_Inputs =>
                  input_mem(nr_of_reads) <= S_AXIS_TDATA;
                  if (nr_of_reads = 0) then
                    state        <= Computing;
                    nr_of_computes <= NUMBER_OF_COMPUTATION;
                  else
                    nr_of_reads <= nr_of_reads - 1;
                  end if;
                
              when Computing =>
                if (nr_of_computes = 0) then
                  state        <= Write_Outputs;
                  nr_of_writes <= NUMBER_OF_OUTPUT_WORDS;
                else
                    case nr_of_computes is
                        when 4 =>
                            state <= Assemble;
                            input <= input_mem(3);
                        when 3 =>
                            state <= Assemble;
                            input <= input_mem(2);
                        when 2 =>
                            state <= Assemble;
                            input <= input_mem(1);
                        when 1 =>
                            state <= Assemble;
                            input <= input_mem(0);
                        when others =>
                            state <= Idle;
                        end case;
                  end if;
                  
              when Assemble =>
                state <= Computing;
                output_mem(nr_of_computes) <= input;
                nr_of_computes <= nr_of_computes - 1;
                    
              when Write_Outputs =>
                if (M_AXIS_TREADY = '1') then
                  if (nr_of_writes = 0) then
                    state <= Idle;                
                  else
                  case nr_of_writes is
                      when 4 =>
                          state <= Transmit;
                          output <= output_mem(1);
                      when 3 =>
                            state <= Transmit;
                          output <= output_mem(2);
                      when 2 =>
                            state <= Transmit;
                          output <= output_mem(3);
                      when 1 =>
                            state <= Transmit;
                          output <= output_mem(4);
                      when others =>
                          state <= Idle;
                      end case;
                  end if;
                end if;
                
               when Transmit =>
                  state <= Write_Outputs;
                  M_AXIS_TDATA <= output;
                  nr_of_writes <= nr_of_writes - 1;

            end case;
          end if;
        end if;
       end process;

	-- User logic ends

end arch_imp;
