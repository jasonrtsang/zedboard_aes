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
        C_S00_AXI_DATA_WIDTH    : integer   := 32;
        C_S00_AXI_ADDR_WIDTH    : integer   := 4;

        -- Parameters of Axi Slave Bus Interface S00_AXIS
        C_S00_AXIS_TDATA_WIDTH  : integer   := 32;

        -- Parameters of Axi Master Bus Interface M00_AXIS
        C_M00_AXIS_TDATA_WIDTH  : integer   := 32;
        C_M00_AXIS_START_COUNT  : integer   := 32
    );
    port (
        -- Users to add ports here

        -- User ports ends

        -- Do not modify the ports beyond this line
        -- Ports of Axi Slave Bus Interface S00_AXI
        s00_axi_aclk    : in std_logic;
        s00_axi_aresetn : in std_logic;
        s00_axi_awaddr  : in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
        s00_axi_awprot  : in std_logic_vector(2 downto 0);
        s00_axi_awvalid : in std_logic;
        s00_axi_awready : out std_logic;
        s00_axi_wdata   : in std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
        s00_axi_wstrb   : in std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
        s00_axi_wvalid  : in std_logic;
        s00_axi_wready  : out std_logic;
        s00_axi_bresp   : out std_logic_vector(1 downto 0);
        s00_axi_bvalid  : out std_logic;
        s00_axi_bready  : in std_logic;
        s00_axi_araddr  : in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
        s00_axi_arprot  : in std_logic_vector(2 downto 0);
        s00_axi_arvalid : in std_logic;
        s00_axi_arready : out std_logic;
        s00_axi_rdata   : out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
        s00_axi_rresp   : out std_logic_vector(1 downto 0);
        s00_axi_rvalid  : out std_logic;
        s00_axi_rready  : in std_logic;

        -- Ports of Axi Slave Bus Interface S00_AXIS
        s00_axis_aclk   : in std_logic;
        s00_axis_aresetn: in std_logic;
        s00_axis_tready : out std_logic;
        s00_axis_tdata  : in std_logic_vector(C_S00_AXIS_TDATA_WIDTH-1 downto 0);
        s00_axis_tstrb  : in std_logic_vector((C_S00_AXIS_TDATA_WIDTH/8)-1 downto 0);
        s00_axis_tlast  : in std_logic;
        s00_axis_tvalid : in std_logic;

        -- Ports of Axi Master Bus Interface M00_AXIS
        m00_axis_aclk   : in std_logic;
        m00_axis_aresetn: in std_logic;
        m00_axis_tvalid : out std_logic;
        m00_axis_tdata  : out std_logic_vector(C_M00_AXIS_TDATA_WIDTH-1 downto 0);
        m00_axis_tstrb  : out std_logic_vector((C_M00_AXIS_TDATA_WIDTH/8)-1 downto 0);
        m00_axis_tlast  : out std_logic;
        m00_axis_tready : in std_logic
    );
end AES_ECB_v1_0;

architecture arch_imp of AES_ECB_v1_0 is

--  -- component declaration
--  component AES_ECB_v1_0_S00_AXI is
--      generic (
--      C_S_AXI_DATA_WIDTH  : integer   := 32;
--      C_S_AXI_ADDR_WIDTH  : integer   := 4
--      );
--      port (
--      S_AXI_ACLK  : in std_logic;
--      S_AXI_ARESETN   : in std_logic;
--      S_AXI_AWADDR    : in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
--      S_AXI_AWPROT    : in std_logic_vector(2 downto 0);
--      S_AXI_AWVALID   : in std_logic;
--      S_AXI_AWREADY   : out std_logic;
--      S_AXI_WDATA : in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
--      S_AXI_WSTRB : in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
--      S_AXI_WVALID    : in std_logic;
--      S_AXI_WREADY    : out std_logic;
--      S_AXI_BRESP : out std_logic_vector(1 downto 0);
--      S_AXI_BVALID    : out std_logic;
--      S_AXI_BREADY    : in std_logic;
--      S_AXI_ARADDR    : in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
--      S_AXI_ARPROT    : in std_logic_vector(2 downto 0);
--      S_AXI_ARVALID   : in std_logic;
--      S_AXI_ARREADY   : out std_logic;
--      S_AXI_RDATA : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
--      S_AXI_RRESP : out std_logic_vector(1 downto 0);
--      S_AXI_RVALID    : out std_logic;
--      S_AXI_RREADY    : in std_logic
--      );
--  end component AES_ECB_v1_0_S00_AXI;

    -- Users to add components here



    -- Users to add components here

  -- Total number of input data.
   constant NUMBER_OF_INPUT_WORDS  : natural := 4;

   -- Total number of output data
   constant NUMBER_OF_OUTPUT_WORDS : natural := 4;
   
   -- Total number of computation
   constant NUMBER_OF_COMPUTATION : natural := 4;

   type STATE_TYPE is (Idle, Assemble, Read_Inputs, Computing, Write_Outputs);
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

begin

---- Instantiation of Axi Bus Interface S00_AXI
--AES_ECB_v1_0_S00_AXI_inst : AES_ECB_v1_0_S00_AXI
--  generic map (
--      C_S_AXI_DATA_WIDTH  => C_S00_AXI_DATA_WIDTH,
--      C_S_AXI_ADDR_WIDTH  => C_S00_AXI_ADDR_WIDTH
--  )
--  port map (
--      S_AXI_ACLK  => s00_axi_aclk,
--      S_AXI_ARESETN   => s00_axi_aresetn,
--      S_AXI_AWADDR    => s00_axi_awaddr,
--      S_AXI_AWPROT    => s00_axi_awprot,
--      S_AXI_AWVALID   => s00_axi_awvalid,
--      S_AXI_AWREADY   => s00_axi_awready,
--      S_AXI_WDATA => s00_axi_wdata,
--      S_AXI_WSTRB => s00_axi_wstrb,
--      S_AXI_WVALID    => s00_axi_wvalid,
--      S_AXI_WREADY    => s00_axi_wready,
--      S_AXI_BRESP => s00_axi_bresp,
--      S_AXI_BVALID    => s00_axi_bvalid,
--      S_AXI_BREADY    => s00_axi_bready,
--      S_AXI_ARADDR    => s00_axi_araddr,
--      S_AXI_ARPROT    => s00_axi_arprot,
--      S_AXI_ARVALID   => s00_axi_arvalid,
--      S_AXI_ARREADY   => s00_axi_arready,
--      S_AXI_RDATA => s00_axi_rdata,
--      S_AXI_RRESP => s00_axi_rresp,
--      S_AXI_RVALID    => s00_axi_rvalid,
--      S_AXI_RREADY    => s00_axi_rready
--  );

-- Overwrite with all user logic

   s00_axis_tready  <= '1' when state = Read_Inputs else '0';
   m00_axis_tvalid <= '1' when state = Write_Outputs else '0';
   m00_axis_tlast <= '1' when (state = Write_Outputs and nr_of_writes = 0) else '0';

   m00_axis_tdata <= std_logic_vector(unsigned(output)) when (state = Write_Outputs) else (others => '0');

        
process (s00_axis_aclk) is
   begin  -- process The_SW_accelerator
    if s00_axis_aclk'event and s00_axis_aclk = '1' then     -- Rising clock edge
      if s00_axis_aresetn = '0' then               -- Synchronous reset (active low)
        -- CAUTION: make sure your reset polarity is consistent with the
        -- system reset polarity
        state        <= Idle;
        nr_of_reads  <= 0;
        nr_of_writes <= 0;
        nr_of_computes <= 0;
      else
        case state is
          when Idle =>            
            if (s00_axis_tvalid = '1') then
              state       <= Read_Inputs;
              nr_of_reads <= NUMBER_OF_INPUT_WORDS - 1;
            end if;

          when Read_Inputs =>
            if (s00_axis_tvalid = '1') then
              input_mem(nr_of_reads) <= std_logic_vector(unsigned(s00_axis_tdata));
              if (nr_of_reads = 0) then
                state        <= Computing;
                nr_of_computes <= NUMBER_OF_COMPUTATION;
              else
                nr_of_reads <= nr_of_reads - 1;
              end if;
            end if;
            
          when Computing =>
            if (nr_of_computes = 0) then
              state        <= Write_Outputs;
              nr_of_writes <= NUMBER_OF_OUTPUT_WORDS - 1;
            else
                --enable <= '1';
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
                        --enable <= '0';
                        state <= Idle;
                    end case;
              end if;
              
          when Assemble =>
            --enable <= '0';
            --if valid = '1' then
                output_mem(nr_of_computes) <= input;
                state <= Computing;
                nr_of_computes <= nr_of_computes - 1;
            --else
                --state <= Assemble;
            --end if;
                
          when Write_Outputs =>
            if (m00_axis_tready = '1') then
              if (nr_of_writes = 0) then
                state <= Idle;                
              else
                output <= output_mem(nr_of_writes+1);
                nr_of_writes <= nr_of_writes - 1;
              end if;
            end if;
        end case;
      end if;
    end if;
   end process;

end arch_imp;
