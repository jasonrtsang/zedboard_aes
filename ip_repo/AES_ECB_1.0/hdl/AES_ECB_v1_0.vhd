library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.aes_package.all;

entity AES_ECB_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


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

	-- component declaration
	component AES_ECB_v1_0_S00_AXIS is
		generic (
		C_S_AXIS_TDATA_WIDTH	: integer	:= 32
		);
		port (
		-- User ports
		S_AXIS_INPUT_KEY : out STATE;
        S_AXIS_INPUT_STATE : out STATE;
        S_AXIS_INPUT_MODE : out AES_MODE;
        -- User ports end
		S_AXIS_ACLK	: in std_logic;
		S_AXIS_ARESETN	: in std_logic;
		S_AXIS_TREADY	: out std_logic;
		S_AXIS_TDATA	: in std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		S_AXIS_TSTRB	: in std_logic_vector((C_S_AXIS_TDATA_WIDTH/8)-1 downto 0);
		S_AXIS_TLAST	: in std_logic;
		S_AXIS_TVALID	: in std_logic
		);
	end component AES_ECB_v1_0_S00_AXIS;

	component AES_ECB_v1_0_M00_AXIS is
		generic (
		C_M_AXIS_TDATA_WIDTH	: integer	:= 32;
		C_M_START_COUNT	: integer	:= 32
		);
		port (
		-- User ports
        M_AXIS_INPUT_STATE : in STATE;
        -- User ports end
		M_AXIS_ACLK	: in std_logic;
		M_AXIS_ARESETN	: in std_logic;
		M_AXIS_TVALID	: out std_logic;
		M_AXIS_TDATA	: out std_logic_vector(C_M_AXIS_TDATA_WIDTH-1 downto 0);
		M_AXIS_TSTRB	: out std_logic_vector((C_M_AXIS_TDATA_WIDTH/8)-1 downto 0);
		M_AXIS_TLAST	: out std_logic;
		M_AXIS_TREADY	: in std_logic
		);
	end component AES_ECB_v1_0_M00_AXIS;
	
	-- Add user components here
    
    component main_ecb is
        port (inMode   : in AES_MODE;
              inKey    : in STATE;
              inState  : in STATE;
              outState : out STATE);        
    end component main_ecb;
    
    signal s00_axis_input_key : STATE;
    signal s00_axis_input_state : STATE;
    signal s00_axis_input_mode : AES_MODE;
    
    signal m00_axis_input_state : STATE;

    -- User components ends

begin

-- Instantiation of Axi Bus Interface S00_AXIS
AES_ECB_v1_0_S00_AXIS_inst : AES_ECB_v1_0_S00_AXIS
	generic map (
		C_S_AXIS_TDATA_WIDTH	=> C_S00_AXIS_TDATA_WIDTH
	)
	port map (
		-- User ports
        S_AXIS_INPUT_KEY => s00_axis_input_key,
        S_AXIS_INPUT_STATE => s00_axis_input_state,
        S_AXIS_INPUT_MODE => s00_axis_input_mode,
        -- User ports end
		S_AXIS_ACLK	=> s00_axis_aclk,
		S_AXIS_ARESETN	=> s00_axis_aresetn,
		S_AXIS_TREADY	=> s00_axis_tready,
		S_AXIS_TDATA	=> s00_axis_tdata,
		S_AXIS_TSTRB	=> s00_axis_tstrb,
		S_AXIS_TLAST	=> s00_axis_tlast,
		S_AXIS_TVALID	=> s00_axis_tvalid
	);

-- Instantiation of Axi Bus Interface M00_AXIS
AES_ECB_v1_0_M00_AXIS_inst : AES_ECB_v1_0_M00_AXIS
	generic map (
		C_M_AXIS_TDATA_WIDTH	=> C_M00_AXIS_TDATA_WIDTH,
		C_M_START_COUNT	=> C_M00_AXIS_START_COUNT
	)
	port map (
		-- User ports
        M_AXIS_INPUT_STATE => m00_axis_input_state,
        -- User ports end
		M_AXIS_ACLK	=> m00_axis_aclk,
		M_AXIS_ARESETN	=> m00_axis_aresetn,
		M_AXIS_TVALID	=> m00_axis_tvalid,
		M_AXIS_TDATA	=> m00_axis_tdata,
		M_AXIS_TSTRB	=> m00_axis_tstrb,
		M_AXIS_TLAST	=> m00_axis_tlast,
		M_AXIS_TREADY	=> m00_axis_tready
	);

	-- Add user logic here
	
-- Instantiation of main ecb fifo
    main_ecb_inst : main_ecb
        port map (
            inMode => s00_axis_input_mode,
            inKey => s00_axis_input_key,
            inState => s00_axis_input_state,
            outState => m00_axis_input_state    
        );

	-- User logic ends

end arch_imp;
