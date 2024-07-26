----------------------------------------------------------------------------------
----------------------------------------------------------------------------------
-- Filename     axis_data_fifo_0_tb.vhd
-- Title        basic simulation for axi4-stream connector
--
-- Project      packetsniffing
-- PJ No.       
-- Syntax       VHDL
-- Note         

-- Version      1.00
-- Author       L.Ratchanon
-- Date         2024/07/26
-- Remark       New Creation
----------------------------------------------------------------------------------
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

Entity axis_data_fifo_0_tb Is
End Entity axis_data_fifo_0_tb;

Architecture HTWTestBench Of axis_data_fifo_0_tb Is

--------------------------------------------------------------------------------------------
-- constant declaration
--------------------------------------------------------------------------------------------
	
	constant	c_tclka		: time := 10 ns;
	constant	c_tclkb		: time := 15 ns;

-------------------------------------------------------------------------
-- component declaration
-------------------------------------------------------------------------

	component axis_data_fifo_0 is
    port 
	(
		s_axis_aresetn	: in	std_logic;
		
		s_axis_aclk		: in	std_logic;
		s_axis_tvalid	: in	std_logic;
		s_axis_tready	: out	std_logic;
		s_axis_tdata	: in	std_logic_vector(511 downto 0);
		s_axis_tkeep	: in	std_logic_vector(63 downto 0);
		s_axis_tlast	: in	std_logic;
		s_axis_tuser	: in	std_logic;
		
		m_axis_aclk		: in	std_logic;
		m_axis_tvalid	: out	std_logic;
		m_axis_tready	: in	std_logic;
		m_axis_tdata	: out	std_logic_vector(511 downto 0);
		m_axis_tkeep	: out	std_logic_vector(63 downto 0);
		m_axis_tlast	: out	std_logic;
		m_axis_tuser	: out	std_logic
    );
	end component axis_data_fifo_0;

-------------------------------------------------------------------------
-- signal declaration
-------------------------------------------------------------------------
	
	signal	TM					: integer	range 0 to 65535;
	signal	TT					: integer 	range 0 to 65535;
	
	signal	reset_n				: std_logic;
	signal	clka				: std_logic;
	signal	clkb				: std_logic;
	
	-- AXI4-stream
	signal	s_axis_tvalid		: std_logic;
	signal	s_axis_tready		: std_logic;
	signal	s_axis_tdata		: std_logic_vector(511 downto 0);
	signal	s_axis_tkeep		: std_logic_vector(63 downto 0);
	signal	s_axis_tlast		: std_logic;
	signal	s_axis_tuser		: std_logic;
		
	signal	m_axis_tvalid		: std_logic;
	signal	m_axis_tready		: std_logic;
	signal	m_axis_tdata		: std_logic_vector(511 downto 0);
	signal	m_axis_tkeep		: std_logic_vector(63 downto 0);
	signal	m_axis_tlast		: std_logic;
	signal	m_axis_tuser		: std_logic;
	
begin
	
--------------------------------------------------------------------------------------------
-- concurrent signal
--------------------------------------------------------------------------------------------

	u_clka : process
	begin
		clka	<= '1';
		wait for c_tclka/2;
		clka		<= '0';
		wait for c_tclka/2;
	end process u_clka;
	
	u_clkb : process
	begin
		clkb	<= '1';
		wait for c_tclkb/2;
		clkb	<= '0';
		wait for c_tclkb/2;
	end process u_clkb;
	
	u_axis_data_fifo_0 : axis_data_fifo_0
    port map
	(
		s_axis_aresetn	=>	reset_n			,
	
		s_axis_aclk		=>	clka			,
		s_axis_tvalid	=>	s_axis_tvalid	,
		s_axis_tready	=>  s_axis_tready	,
		s_axis_tdata	=>  s_axis_tdata	,
		s_axis_tkeep	=>  s_axis_tkeep	,
		s_axis_tlast	=>  s_axis_tlast	,
		s_axis_tuser	=>  s_axis_tuser	,

		m_axis_aclk		=> 	clkb            ,
		m_axis_tvalid	=>	m_axis_tvalid	,
		m_axis_tready	=>  m_axis_tready	,
		m_axis_tdata	=>  m_axis_tdata	,
		m_axis_tkeep	=>  m_axis_tkeep	,
		m_axis_tlast	=>  m_axis_tlast	,
		m_axis_tuser	=>  m_axis_tuser	
	);
	
-------------------------------------------------------------------------
-- testbench
-------------------------------------------------------------------------

	u_test : process
	begin
		---------------------------------
		TM <= 0; TT <= 0; 
		wait until rising_edge(clka);
		reset_n			<=	'0';
		s_axis_tvalid	<=	'0';	
		s_axis_tdata    <=	(others=>'0');
		s_axis_tkeep    <=	(others=>'0');
		s_axis_tlast    <=	'0';
		s_axis_tuser    <=	'0';
		m_axis_tready   <=	'0';
		wait for 20 * c_tclka;
		reset_n			<=	'1';
		wait for 20 * c_tclka;
		
		---------------------------------
		TM <= 1; TT <= 0; 
		wait until rising_edge(clka);
		s_axis_tvalid	<= 	'1';
		s_axis_tkeep	<= 	(others=>'1');
		for i in 0 to 9 loop 
			for j in 0 to 63 loop
				s_axis_tdata(8*j+7 downto 8*j)	<=	conv_std_logic_vector(i, 8);
			end loop;
			if (i = 9) then 
				s_axis_tlast	<= 	'1';
			end if;
			wait until rising_edge(clka) and s_axis_tready='1';
		end loop;
		s_axis_tvalid	<=	'0';	
		s_axis_tdata    <=	(others=>'0');
		s_axis_tkeep    <=	(others=>'0');
		s_axis_tlast    <=	'0';
		
		---------------------------------
		TM <= 2; TT <= 0; 
		wait until rising_edge(clka);
		assert m_axis_tvalid='0' 
		report "error: testbench - valid should not yet be 0b1" severity failure;
		
		m_axis_tready	<= 	'1';
		wait until rising_edge(clkb) and m_axis_tvalid='1';
		for i in 0 to 9 loop 
			assert m_axis_tvalid='1' 
			report "error: testbench - valid should always be 0b1" severity failure;
			assert m_axis_tkeep=x"ffff_ffff_ffff_ffff"
			report "error: testbench - keep should always be all ones" severity failure;
			assert m_axis_tuser='0' 
			report "error: testbench - user should always be 0b0" severity failure;
			for j in 0 to 63 loop
				assert m_axis_tdata(8*j+7 downto 8*j)=i
				report "error: testbench - output data is incorrect" severity failure;
			end loop;
			if (i = 9) then 
				assert m_axis_tlast='1' 
				report "error: testbench - last should be 0b1" severity failure;
			else
				assert m_axis_tlast='0' 
				report "error: testbench - last should be 0b1" severity failure;
			end if;
			wait until rising_edge(clkb);
		end loop;
		
		assert m_axis_tvalid='0' 
		report "error: testbench - valid should be 0b0" severity failure;
		wait for 10 * c_tclka;
		
		---------------------------------
		TM <= 255; TT <= 255;
		wait for 100 * c_tclka;
		report "##### End Simulation #####" severity failure;	
	end process u_test;

End Architecture HTWTestBench;