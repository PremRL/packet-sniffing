`default_nettype none
`timescale 1 ns / 1 ps

module eth_kernel #(
  parameter integer C_AXI_LITE_ADDRWIDTH    = 10    ,
  parameter integer c_AXI_LITE_DATAWIDTH    = 32 
)
(
  // Platform interface
  input  wire                                   ap_clk              ,
  input  wire                                   ap_rst_n            ,
  
  // AXI4-Lite slave interface
  input  wire                                   s_axi_ctrl_awvalid  ,
  output wire                                   s_axi_ctrl_awready  ,
  input  wire [C_AXI_LITE_ADDRWIDTH-1:0]        s_axi_ctrl_awaddr   ,
  input  wire                                   s_axi_ctrl_wvalid   ,
  output wire                                   s_axi_ctrl_wready   ,
  input  wire [c_AXI_LITE_DATAWIDTH-1:0]        s_axi_ctrl_wdata    ,
  input  wire [c_AXI_LITE_DATAWIDTH/8-1:0]      s_axi_ctrl_wstrb    ,
  input  wire                                   s_axi_ctrl_arvalid  ,
  output wire                                   s_axi_ctrl_arready  ,
  input  wire [C_AXI_LITE_ADDRWIDTH-1:0]        s_axi_ctrl_araddr   ,
  output wire                                   s_axi_ctrl_rvalid   ,
  input  wire                                   s_axi_ctrl_rready   ,
  output wire [c_AXI_LITE_DATAWIDTH-1:0]        s_axi_ctrl_rdata    ,
  output wire [1:0]                             s_axi_ctrl_rresp    ,
  output wire                                   s_axi_ctrl_bvalid   ,
  input  wire                                   s_axi_ctrl_bready   ,
  output wire [1:0]                             s_axi_ctrl_bresp    ,

  // Connectivity AXI4-stream
  output wire           rx0_axis_tvalid     ,
  output wire [511:0]   rx0_axis_tdata      ,
  output wire           rx0_axis_tlast      ,
  output wire [63:0]    rx0_axis_tkeep      ,
  input  wire           rx0_axis_tready     ,
  output wire           rx0_axis_tuser      ,

  output wire           rx1_axis_tvalid     ,
  output wire [511:0]   rx1_axis_tdata      ,
  output wire           rx1_axis_tlast      ,
  output wire [63:0]    rx1_axis_tkeep      ,
  input  wire           rx1_axis_tready     ,
  output wire           rx1_axis_tuser      ,

  output wire           tx0_axis_tready     ,
  input  wire           tx0_axis_tvalid     ,
  input  wire [511:0]   tx0_axis_tdata      ,
  input  wire           tx0_axis_tlast      ,
  input  wire [63:0]    tx0_axis_tkeep      ,
  input  wire           tx0_axis_tuser      ,

  output wire           tx1_axis_tready     ,
  input  wire           tx1_axis_tvalid     ,
  input  wire [511:0]   tx1_axis_tdata      ,
  input  wire           tx1_axis_tlast      ,
  input  wire [63:0]    tx1_axis_tkeep      ,
  input  wire           tx1_axis_tuser      ,
  
  // transceiver
  input  wire           clk_gt_freerun      ,

  input  wire           gt_refclk_p_0       ,
  input  wire           gt_refclk_n_0       ,
  input  wire [3:0]     gt_rxp_in_0         ,
  input  wire [3:0]     gt_rxn_in_0         ,
  output wire [3:0]     gt_txp_out_0        ,
  output wire [3:0]     gt_txn_out_0        ,

  input  wire           gt_refclk_p_1       ,
  input  wire           gt_refclk_n_1       ,
  input  wire [3:0]     gt_rxp_in_1         ,
  input  wire [3:0]     gt_rxn_in_1         ,
  output wire [3:0]     gt_txp_out_1        ,
  output wire [3:0]     gt_txn_out_1
);
///////////////////////////////////////////////////////////////////////////////
// Wires and registers
///////////////////////////////////////////////////////////////////////////////
(* DONT_TOUCH = "yes" *)
genvar          i;
reg [3:0]       sys_reset_count = 4'd0;
reg             app_reset_n = 1'b0;
wire            app_reset;

reg                             wValid;
reg                             rValid;
reg [c_AXI_LITE_DATAWIDTH-1:0]  rData;

wire [1:0]      rx_user_axis_tvalid;
wire [511:0]    rx_user_axis_tdata [1:0];
wire [1:0]      rx_user_axis_tlast;
wire [63:0]     rx_user_axis_tkeep [1:0];
wire [1:0]      rx_user_axis_tready;
wire [1:0]      rx_user_axis_tuser;

wire [1:0]      tx_user_axis_tvalid;
wire [511:0]    tx_user_axis_tdata [1:0];
wire [1:0]      tx_user_axis_tlast;
wire [63:0]     tx_user_axis_tkeep [1:0];
wire [1:0]      tx_user_axis_tready;
wire [1:0]      tx_user_axis_tuser;

wire [1:0]      rx_mac_axis_tvalid;
wire [511:0]    rx_mac_axis_tdata [1:0];
wire [1:0]      rx_mac_axis_tlast;
wire [63:0]     rx_mac_axis_tkeep [1:0];
wire [1:0]      rx_mac_axis_tready;
wire [1:0]      rx_mac_axis_tuser;

wire [1:0]      tx_mac_axis_tvalid;
wire [511:0]    tx_mac_axis_tdata [1:0];
wire [1:0]      tx_mac_axis_tlast;
wire [63:0]     tx_mac_axis_tkeep [1:0];
wire [1:0]      tx_mac_axis_tready;
wire [1:0]      tx_mac_axis_tuser;

wire [1:0]      no_packet_transfer;
reg [1:0]       packet_intransition;
reg [1:0]       tx_user_enable;
reg [3:0]       tx_fifo_reset_count [1:0];
wire [1:0]      tx_fifo_reset;
wire [1:0]      rx_fifo_reset_n;

wire [1:0]      tx_data_fifo_write_en;
wire [1:0]      tx_data_fifo_full;
wire [1:0]      tx_data_fifo_read_en;
wire [1:0]      tx_data_fifo_empty;

wire [1:0]      tx_meta_fifo_write_en;
wire [1:0]      tx_meta_fifo_full;
wire [1:0]      tx_meta_fifo_read_en;
wire [1:0]      tx_meta_fifo_empty;

wire [1:0]      macclk;
wire [1:0]      mac_reset_user;
wire [1:0]      mac_tx_reset_user;
wire [1:0]      mac_rx_reset_user;
wire [1:0]      mac_tx_reset;
wire [1:0]      mac_rx_reset;
wire [1:0]      stat_rx_local_fault;
wire [1:0]      stat_rx_remote_fault;
wire [1:0]      stat_rx_local_fault_user;
wire [1:0]      stat_rx_remote_fault_user;

///////////////////////////////////////////////////////////////////////////////
// Main body
///////////////////////////////////////////////////////////////////////////////

// reset 
always @(posedge ap_clk) begin
    if (!ap_rst_n) 
        sys_reset_count <= 4'd0;
    else begin 
        if (sys_reset_count == 15)  
            sys_reset_count <= sys_reset_count;
        else begin
            sys_reset_count <= sys_reset_count + 1;
        end 
    end 
end 

always @(posedge ap_clk) begin
    if (ap_rst_n && (sys_reset_count == 15)) 
        app_reset_n <= 1'b1;
    else begin 
        app_reset_n <= 1'b0;
    end 
end

assign app_reset = ~app_reset_n;

generate 
    for (i = 0; i < 2; i = i + 1) begin
        assign mac_reset_user[i] = mac_rx_reset_user[i] | mac_tx_reset_user[i];

        xpm_cdc_sync_rst #(
            .DEST_SYNC_FF(4),   
            .INIT(1),        
            .INIT_SYNC_FF(0),   
            .SIM_ASSERT_CHK(0)  
        )
        mac_rx_reset_cdc (
            .dest_rst(mac_rx_reset_user[i]),
            .dest_clk(ap_clk), 
            .src_rst(mac_rx_reset[i])  
        );

        xpm_cdc_sync_rst #(
            .DEST_SYNC_FF(4),   
            .INIT(1),        
            .INIT_SYNC_FF(0),   
            .SIM_ASSERT_CHK(0)  
        )
        mac_tx_reset_cdc (
            .dest_rst(mac_tx_reset_user[i]),
            .dest_clk(ap_clk), 
            .src_rst(mac_tx_reset[i])  
        );
    end 
endgenerate

// AXI4-Lite logic 
// write: there's nothing to be written
// read: monitoring ethernet status
assign s_axi_ctrl_awready = s_axi_ctrl_awvalid & s_axi_ctrl_wvalid & ~wValid;
assign s_axi_ctrl_wready = s_axi_ctrl_awvalid & s_axi_ctrl_wvalid & ~wValid;
assign s_axi_ctrl_bvalid = wValid;
assign s_axi_ctrl_bresp = 2'b00;

always @(posedge ap_clk) begin
    if (!ap_rst_n)  
        wValid  <= 1'b0;
    else begin 
        if (s_axi_ctrl_awvalid && s_axi_ctrl_wvalid && !wValid) 
            wValid  <= 1'b1;
        else if (s_axi_ctrl_bready) 
            wValid  <= 1'b0;
        else begin 
            wValid  <= wValid;
        end 
    end 
end 

assign s_axi_ctrl_arready = s_axi_ctrl_arvalid & ~rValid;
assign s_axi_ctrl_rvalid = rValid;
assign s_axi_ctrl_rdata = rData;
assign s_axi_ctrl_rresp = 2'b00;

always @(posedge ap_clk) begin
    if (!ap_rst_n)  
        rValid  <= 1'b0;
    else begin 
        if (s_axi_ctrl_arvalid && !rValid) 
            rValid  <= 1'b1;
        else if (s_axi_ctrl_rready) 
            rValid  <= 1'b0;
        else begin 
            rValid  <= rValid;
        end 
    end 
end 

always @(posedge ap_clk) begin
    if (!rValid)
        case (s_axi_ctrl_araddr) 
            'h10 : begin 
                rData[31:0] <= 32'h30314744; // unique number 
            end 
            'h20 : begin 
                rData[1:0]  <= mac_reset_user[1:0];
                rData[31:2] <= 30'd0;
            end 
            'h30 : begin 
                rData[0]    <= stat_rx_local_fault_user[0];
                rData[1]    <= stat_rx_remote_fault_user[0];
                rData[31:2] <= 30'd0;
            end 
            'h38 : begin 
                rData[0]    <= stat_rx_local_fault_user[1];
                rData[1]    <= stat_rx_remote_fault_user[1];
                rData[31:2] <= 30'd0;
            end 
            default: begin
                rData[31:0] <= 32'd0;
            end  
        endcase
    else begin 
        rData   <= rData;
    end 
end 

xpm_cdc_array_single #(
    .DEST_SYNC_FF(4),   
    .INIT_SYNC_FF(0),   
    .SIM_ASSERT_CHK(0),
    .SRC_INPUT_REG(1),
    .WIDTH(2)
) sync_mac0_reg (
    .dest_out({stat_rx_local_fault_user[0], stat_rx_remote_fault_user[0]}), 
    .dest_clk(ap_clk), 
    .src_clk(macclk[0]),  
    .src_in({stat_rx_local_fault[0], stat_rx_remote_fault[0]})  
);

xpm_cdc_array_single #(
    .DEST_SYNC_FF(4),   
    .INIT_SYNC_FF(0),   
    .SIM_ASSERT_CHK(0),
    .SRC_INPUT_REG(1),
    .WIDTH(2)
) sync_mac1_reg (
    .dest_out({stat_rx_local_fault_user[1], stat_rx_remote_fault_user[1]}), 
    .dest_clk(ap_clk), 
    .src_clk(macclk[1]),  
    .src_in({stat_rx_local_fault[1], stat_rx_remote_fault[1]})
);

// assigning data interface
assign  rx0_axis_tvalid         = rx_user_axis_tvalid[0];
assign  rx0_axis_tdata          = rx_user_axis_tdata[0];
assign  rx0_axis_tlast          = rx_user_axis_tlast[0];
assign  rx0_axis_tkeep          = rx_user_axis_tkeep[0];
assign  rx_user_axis_tready[0]  = rx0_axis_tready;
assign  rx0_axis_tuser          = rx_user_axis_tuser[0];

assign  rx1_axis_tvalid         = rx_user_axis_tvalid[1];
assign  rx1_axis_tdata          = rx_user_axis_tdata[1];
assign  rx1_axis_tlast          = rx_user_axis_tlast[1];
assign  rx1_axis_tkeep          = rx_user_axis_tkeep[1];
assign  rx_user_axis_tready[1]  = rx1_axis_tready;
assign  rx1_axis_tuser          = rx_user_axis_tuser[1];

assign  tx0_axis_tready         = (tx_user_enable[0]) ? tx_user_axis_tready[0] : 1'b1;
assign  tx_user_axis_tvalid[0]  = (tx_user_enable[0]) ? tx0_axis_tvalid: 1'b0;
assign  tx_user_axis_tdata[0]   = tx0_axis_tdata;
assign  tx_user_axis_tlast[0]   = tx0_axis_tlast;
assign  tx_user_axis_tkeep[0]   = tx0_axis_tkeep;
assign  tx_user_axis_tuser[0]   = tx0_axis_tuser;

assign  tx1_axis_tready         = (tx_user_enable[1]) ? tx_user_axis_tready[1] : 1'b1;
assign  tx_user_axis_tvalid[1]  = (tx_user_enable[1]) ? tx1_axis_tvalid: 1'b0;
assign  tx_user_axis_tdata[1]   = tx1_axis_tdata;
assign  tx_user_axis_tlast[1]   = tx1_axis_tlast;
assign  tx_user_axis_tkeep[1]   = tx1_axis_tkeep;
assign  tx_user_axis_tuser[1]   = tx1_axis_tuser;

// asynchronous fifos for crossing to/from mac clock and user clock
generate 
    for (i = 0; i < 2; i = i + 1) begin
        // additional tx logic to filter egressing packets if the ethernet module is yet to ready
        always @(posedge ap_clk) begin
            if (!app_reset_n || mac_reset_user[i])
                packet_intransition[i]  <= 1'b0;
            else begin 
                if (tx_user_axis_tvalid[i] && tx_user_axis_tlast[i])
                    packet_intransition[i]  <= 1'b0;
                else if (tx_user_axis_tvalid[i])
                    packet_intransition[i]  <= 1'b1;
                else begin 
                    packet_intransition[i]  <= packet_intransition[i];
                end 
            end 
        end 

        assign no_packet_transfer[i] = ((packet_intransition[i] == 0) & (tx_user_axis_tvalid[i] == 0)) | ((tx_user_axis_tvalid[i] == 1) & (tx_user_axis_tlast[i] == 1));
         
        always @(posedge ap_clk) begin
            if (!app_reset_n || tx_fifo_reset[i]) 
                tx_user_enable[i]   <= 1'b0;
            else begin
                if (no_packet_transfer[i])  
                    tx_user_enable[i]   <= 1'b1;
                else begin
                    tx_user_enable[i]   <= tx_user_enable[i];
                end
            end 
        end 

        always @(posedge ap_clk) begin
            if (!app_reset_n || mac_reset_user[i]) 
                tx_fifo_reset_count[i] <= 4'd0;
            else begin
                if ((stat_rx_local_fault_user[i] == 0) && (stat_rx_remote_fault_user[i] == 0))
                    if (tx_fifo_reset_count[i] == 15) 
                        tx_fifo_reset_count[i] <= tx_fifo_reset_count[i];
                    else begin 
                        tx_fifo_reset_count[i] <= tx_fifo_reset_count[i] + 1;
                    end 
                else begin
                    tx_fifo_reset_count[i] <= 4'd0;
                end
            end 
        end 
       
        // tx fifo using fwft type
        // CMAC requires the axi stream to be continuous 
        // so we add another meta fifo to delay the operation of the read side

        assign tx_fifo_reset[i] = (tx_fifo_reset_count[i] == 15) ? 1'b0 : 1'b1;

        assign tx_user_axis_tready[i] = ~tx_data_fifo_full[i] & ~tx_meta_fifo_full[i];
        assign tx_data_fifo_write_en[i] = tx_user_axis_tvalid[i] & tx_user_axis_tready[i];
        assign tx_meta_fifo_write_en[i] = tx_user_axis_tvalid[i] & tx_user_axis_tready[i] & tx_user_axis_tlast[i];

        fifo_generator_0 tx_fifo(
            .rst(tx_fifo_reset[i]),

            .wr_clk(ap_clk),
            .wr_en(tx_data_fifo_write_en[i]),
            .din({tx_user_axis_tuser[i], tx_user_axis_tlast[i], tx_user_axis_tkeep[i], tx_user_axis_tdata[i]}),
            .full(tx_data_fifo_full[i]),

            .rd_clk(macclk[i]),
            .rd_en(tx_data_fifo_read_en[i]),
            .dout({tx_mac_axis_tuser[i], tx_mac_axis_tlast[i], tx_mac_axis_tkeep[i], tx_mac_axis_tdata[i]}),
            .empty(tx_data_fifo_empty[i])
        );

        // using fifo_generator with a larger value of CDC stages
        fifo_generator_1 tx_meta(
            .rst(tx_fifo_reset[i]),

            .wr_clk(ap_clk),
            .wr_en(tx_meta_fifo_write_en[i]),
            .din(1),
            .full(tx_meta_fifo_full[i]),

            .rd_clk(macclk[i]),
            .rd_en(tx_meta_fifo_read_en[i]),
            .dout(),
            .empty(tx_meta_fifo_empty[i])
        );

        // always expected that tx_data_fifo_empty[i] is not empty during one packet transfer
        assign tx_mac_axis_tvalid[i] = ~tx_meta_fifo_empty[i] & ~tx_data_fifo_empty[i];
        assign tx_data_fifo_read_en[i] = tx_mac_axis_tvalid[i] & tx_mac_axis_tready[i];
        assign tx_meta_fifo_read_en[i] = tx_mac_axis_tvalid[i] & tx_mac_axis_tready[i] & tx_mac_axis_tlast[i];

        // rx fifo with disabling packet mode
        // It's expected that tx_user_axis_tready will never dropped to '0'
        
        assign rx_fifo_reset_n[i] = app_reset_n & ~mac_reset_user[i];

        axis_data_fifo_0 rx_fifo(
            .s_axis_aresetn(rx_fifo_reset_n[i]),

            .s_axis_aclk(macclk[i]),
            .s_axis_tvalid(rx_mac_axis_tvalid[i]),
            .s_axis_tready(rx_mac_axis_tready[i]),
            .s_axis_tdata(rx_mac_axis_tdata[i]),
            .s_axis_tkeep(rx_mac_axis_tkeep[i]),
            .s_axis_tlast(rx_mac_axis_tlast[i]),
            .s_axis_tuser(rx_mac_axis_tuser[i]),

            .m_axis_aclk(ap_clk),
            .m_axis_tvalid(rx_user_axis_tvalid[i]),
            .m_axis_tready(rx_user_axis_tready[i]),
            .m_axis_tdata(rx_user_axis_tdata[i]),
            .m_axis_tkeep(rx_user_axis_tkeep[i]),
            .m_axis_tlast(rx_user_axis_tlast[i]),
            .m_axis_tuser(rx_user_axis_tuser[i])
        );
    end 
endgenerate

// Ethernet Subsystem instance
ethernet_0 cmac_0 (
    .sys_reset(app_reset),
    .gtwiz_reset_tx_datapath(1'b0),
    .gtwiz_reset_rx_datapath(1'b0),

    .gt_ref_clk_p(gt_refclk_p_0),
    .gt_ref_clk_n(gt_refclk_n_0),
    .init_clk(clk_gt_freerun),
    .gt_rxrecclkout(),
    .gt_ref_clk_out(),
    .gt_rxp_in(gt_rxp_in_0),
    .gt_rxn_in(gt_rxn_in_0),
    .gt_txp_out(gt_txp_out_0),
    .gt_txn_out(gt_txn_out_0),
    .gt_loopback_in(12'b0),
    .gt_powergoodout(),

    // reset, clock and setting
    .core_tx_reset(1'b0),
    .usr_tx_reset(mac_tx_reset[0]),
    .gt_txusrclk2(macclk[0]),

    .core_rx_reset(1'b0),
    .rx_clk(macclk[0]),
    .usr_rx_reset(mac_rx_reset[0]),
    .gt_rxusrclk2(),

    .tx_preamblein(56'b0),
    .ctl_tx_enable(1'b1),
    .ctl_tx_rsfec_enable(1'b1),
    .ctl_tx_test_pattern(1'b0),
    .ctl_tx_send_idle(1'b0),
    .ctl_tx_send_rfi(1'b0),
    .ctl_tx_send_lfi(1'b0),
	.ctl_rx_enable(1'b1),
    .ctl_rx_force_resync(1'b0),
    .ctl_rx_test_pattern(1'b0),
	.ctl_rsfec_ieee_error_indication_mode(1'b1),
    .ctl_rx_rsfec_enable(1'b1),
    .ctl_rx_rsfec_enable_correction(1'b1),
    .ctl_rx_rsfec_enable_indication(1'b1),

    // tx path 
    .tx_axis_tready(tx_mac_axis_tready[0]),
    .tx_axis_tvalid(tx_mac_axis_tvalid[0]),
    .tx_axis_tdata(tx_mac_axis_tdata[0]),
    .tx_axis_tlast(tx_mac_axis_tlast[0]),
    .tx_axis_tkeep(tx_mac_axis_tkeep[0]),
    .tx_axis_tuser(tx_mac_axis_tuser[0]),

    // rx path
    .rx_axis_tvalid(rx_mac_axis_tvalid[0]),
    .rx_axis_tdata(rx_mac_axis_tdata[0]),
    .rx_axis_tlast(rx_mac_axis_tlast[0]),
    .rx_axis_tkeep(rx_mac_axis_tkeep[0]),
    .rx_axis_tuser(rx_mac_axis_tuser[0]),

    // status
    .tx_ovfout(),
    .tx_unfout(),
    .stat_tx_bad_fcs(),
    .stat_tx_broadcast(),
    .stat_tx_frame_error(),
    .stat_tx_local_fault(),
    .stat_tx_multicast(),
    .stat_tx_packet_1024_1518_bytes(),
    .stat_tx_packet_128_255_bytes(),
    .stat_tx_packet_1519_1522_bytes(),
    .stat_tx_packet_1523_1548_bytes(),
    .stat_tx_packet_1549_2047_bytes(),
    .stat_tx_packet_2048_4095_bytes(),
    .stat_tx_packet_256_511_bytes(),
    .stat_tx_packet_4096_8191_bytes(),
    .stat_tx_packet_512_1023_bytes(),
    .stat_tx_packet_64_bytes(),
    .stat_tx_packet_65_127_bytes(),
    .stat_tx_packet_8192_9215_bytes(),
    .stat_tx_packet_large(),
    .stat_tx_packet_small(),
    .stat_tx_total_bytes(),
    .stat_tx_total_good_bytes(),
    .stat_tx_total_good_packets(),
    .stat_tx_total_packets(),
    .stat_tx_unicast(),
    .stat_tx_vlan(),
    .rx_otn_bip8_0(),
    .rx_otn_bip8_1(),
    .rx_otn_bip8_2(),
    .rx_otn_bip8_3(),
    .rx_otn_bip8_4(),
    .rx_otn_data_0(),
    .rx_otn_data_1(),
    .rx_otn_data_2(),
    .rx_otn_data_3(),
    .rx_otn_data_4(),
    .rx_otn_ena(),
    .rx_otn_lane0(),
    .rx_otn_vlmarker(),
    .rx_preambleout(),
    .stat_rx_aligned(),
    .stat_rx_aligned_err(),
    .stat_rx_bad_code(),
    .stat_rx_bad_fcs(),
    .stat_rx_bad_preamble(),
    .stat_rx_bad_sfd(),
    .stat_rx_bip_err_0(),
    .stat_rx_bip_err_1(),
    .stat_rx_bip_err_10(),
    .stat_rx_bip_err_11(),
    .stat_rx_bip_err_12(),
    .stat_rx_bip_err_13(),
    .stat_rx_bip_err_14(),
    .stat_rx_bip_err_15(),
    .stat_rx_bip_err_16(),
    .stat_rx_bip_err_17(),
    .stat_rx_bip_err_18(),
    .stat_rx_bip_err_19(),
    .stat_rx_bip_err_2(),
    .stat_rx_bip_err_3(),
    .stat_rx_bip_err_4(),
    .stat_rx_bip_err_5(),
    .stat_rx_bip_err_6(),
    .stat_rx_bip_err_7(),
    .stat_rx_bip_err_8(),
    .stat_rx_bip_err_9(),
    .stat_rx_block_lock(),
    .stat_rx_broadcast(),
    .stat_rx_fragment(),
    .stat_rx_framing_err_0(),
    .stat_rx_framing_err_1(),
    .stat_rx_framing_err_10(),
    .stat_rx_framing_err_11(),
    .stat_rx_framing_err_12(),
    .stat_rx_framing_err_13(),
    .stat_rx_framing_err_14(),
    .stat_rx_framing_err_15(),
    .stat_rx_framing_err_16(),
    .stat_rx_framing_err_17(),
    .stat_rx_framing_err_18(),
    .stat_rx_framing_err_19(),
    .stat_rx_framing_err_2(),
    .stat_rx_framing_err_3(),
    .stat_rx_framing_err_4(),
    .stat_rx_framing_err_5(),
    .stat_rx_framing_err_6(),
    .stat_rx_framing_err_7(),
    .stat_rx_framing_err_8(),
    .stat_rx_framing_err_9(),
    .stat_rx_framing_err_valid_0(),
    .stat_rx_framing_err_valid_1(),
    .stat_rx_framing_err_valid_10(),
    .stat_rx_framing_err_valid_11(),
    .stat_rx_framing_err_valid_12(),
    .stat_rx_framing_err_valid_13(),
    .stat_rx_framing_err_valid_14(),
    .stat_rx_framing_err_valid_15(),
    .stat_rx_framing_err_valid_16(),
    .stat_rx_framing_err_valid_17(),
    .stat_rx_framing_err_valid_18(),
    .stat_rx_framing_err_valid_19(),
    .stat_rx_framing_err_valid_2(),
    .stat_rx_framing_err_valid_3(),
    .stat_rx_framing_err_valid_4(),
    .stat_rx_framing_err_valid_5(),
    .stat_rx_framing_err_valid_6(),
    .stat_rx_framing_err_valid_7(),
    .stat_rx_framing_err_valid_8(),
    .stat_rx_framing_err_valid_9(),
    .stat_rx_got_signal_os(),
    .stat_rx_hi_ber(),
    .stat_rx_inrangeerr(),
    .stat_rx_internal_local_fault(),
    .stat_rx_jabber(),
    .stat_rx_local_fault(stat_rx_local_fault[0]),
    .stat_rx_mf_err(),
    .stat_rx_mf_len_err(),
    .stat_rx_mf_repeat_err(),
    .stat_rx_misaligned(),
    .stat_rx_multicast(),
    .stat_rx_oversize(),
    .stat_rx_packet_1024_1518_bytes(),
    .stat_rx_packet_128_255_bytes(),
    .stat_rx_packet_1519_1522_bytes(),
    .stat_rx_packet_1523_1548_bytes(),
    .stat_rx_packet_1549_2047_bytes(),
    .stat_rx_packet_2048_4095_bytes(),
    .stat_rx_packet_256_511_bytes(),
    .stat_rx_packet_4096_8191_bytes(),
    .stat_rx_packet_512_1023_bytes(),
    .stat_rx_packet_64_bytes(),
    .stat_rx_packet_65_127_bytes(),
    .stat_rx_packet_8192_9215_bytes(),
    .stat_rx_packet_bad_fcs(),
    .stat_rx_packet_large(),
    .stat_rx_packet_small(),
    .stat_rx_received_local_fault(),
    .stat_rx_remote_fault(stat_rx_remote_fault[0]),
    .stat_rx_status(),
    .stat_rx_stomped_fcs(),
    .stat_rx_synced(),
    .stat_rx_synced_err(),
    .stat_rx_test_pattern_mismatch(),
    .stat_rx_toolong(),
    .stat_rx_total_bytes(),
    .stat_rx_total_good_bytes(),
    .stat_rx_total_good_packets(),
    .stat_rx_total_packets(),
    .stat_rx_truncated(),
    .stat_rx_undersize(),
    .stat_rx_unicast(),
    .stat_rx_vlan(),
    .stat_rx_pcsl_demuxed(),
    .stat_rx_pcsl_number_0(),
    .stat_rx_pcsl_number_1(),
    .stat_rx_pcsl_number_10(),
    .stat_rx_pcsl_number_11(),
    .stat_rx_pcsl_number_12(),
    .stat_rx_pcsl_number_13(),
    .stat_rx_pcsl_number_14(),
    .stat_rx_pcsl_number_15(),
    .stat_rx_pcsl_number_16(),
    .stat_rx_pcsl_number_17(),
    .stat_rx_pcsl_number_18(),
    .stat_rx_pcsl_number_19(),
    .stat_rx_pcsl_number_2(),
    .stat_rx_pcsl_number_3(),
    .stat_rx_pcsl_number_4(),
    .stat_rx_pcsl_number_5(),
    .stat_rx_pcsl_number_6(),
    .stat_rx_pcsl_number_7(),
    .stat_rx_pcsl_number_8(),
    .stat_rx_pcsl_number_9(),
    .stat_rx_rsfec_am_lock0(),
    .stat_rx_rsfec_am_lock1(),
    .stat_rx_rsfec_am_lock2(),
    .stat_rx_rsfec_am_lock3(),
    .stat_rx_rsfec_corrected_cw_inc(),
    .stat_rx_rsfec_cw_inc(),
    .stat_rx_rsfec_err_count0_inc(),
    .stat_rx_rsfec_err_count1_inc(),
    .stat_rx_rsfec_err_count2_inc(),
    .stat_rx_rsfec_err_count3_inc(),
    .stat_rx_rsfec_hi_ser(),
    .stat_rx_rsfec_lane_alignment_status(),
    .stat_rx_rsfec_lane_fill_0(),
    .stat_rx_rsfec_lane_fill_1(),
    .stat_rx_rsfec_lane_fill_2(),
    .stat_rx_rsfec_lane_fill_3(),
    .stat_rx_rsfec_lane_mapping(),
    .stat_rx_rsfec_uncorrected_cw_inc(),

    // configuration
    .core_drp_reset(1'b0),
    .drp_clk(clk_gt_freerun),
    .drp_addr(10'b0),
    .drp_di(16'b0),
    .drp_en(1'b0),
    .drp_do(),
    .drp_rdy(),
    .drp_we(1'b0)
);

ethernet_1 cmac_1 (
    .sys_reset(app_reset),
    .gtwiz_reset_tx_datapath(1'b0),
    .gtwiz_reset_rx_datapath(1'b0),

    .gt_ref_clk_p(gt_refclk_p_1),
    .gt_ref_clk_n(gt_refclk_n_1),
    .init_clk(clk_gt_freerun),
    .gt_rxrecclkout(),
    .gt_ref_clk_out(),
    .gt_rxp_in(gt_rxp_in_1),
    .gt_rxn_in(gt_rxn_in_1),
    .gt_txp_out(gt_txp_out_1),
    .gt_txn_out(gt_txn_out_1),
    .gt_loopback_in(12'b0),
    .gt_powergoodout(),

    // reset, clock and setting
    .core_tx_reset(1'b0),
    .usr_tx_reset(mac_tx_reset[1]),
    .gt_txusrclk2(macclk[1]),

    .core_rx_reset(1'b0),
    .rx_clk(macclk[1]),
    .usr_rx_reset(mac_rx_reset[1]),
    .gt_rxusrclk2(),

    .tx_preamblein(56'b0),
    .ctl_tx_enable(1'b1),
    .ctl_tx_rsfec_enable(1'b1),
    .ctl_tx_test_pattern(1'b0),
    .ctl_tx_send_idle(1'b0),
    .ctl_tx_send_rfi(1'b0),
    .ctl_tx_send_lfi(1'b0),
	.ctl_rx_enable(1'b1),
    .ctl_rx_force_resync(1'b0),
    .ctl_rx_test_pattern(1'b0),
	.ctl_rsfec_ieee_error_indication_mode(1'b1),
    .ctl_rx_rsfec_enable(1'b1),
    .ctl_rx_rsfec_enable_correction(1'b1),
    .ctl_rx_rsfec_enable_indication(1'b1),

    // tx path 
    .tx_axis_tready(tx_mac_axis_tready[1]),
    .tx_axis_tvalid(tx_mac_axis_tvalid[1]),
    .tx_axis_tdata(tx_mac_axis_tdata[1]),
    .tx_axis_tlast(tx_mac_axis_tlast[1]),
    .tx_axis_tkeep(tx_mac_axis_tkeep[1]),
    .tx_axis_tuser(tx_mac_axis_tuser[1]),

    // rx path
    .rx_axis_tvalid(rx_mac_axis_tvalid[1]),
    .rx_axis_tdata(rx_mac_axis_tdata[1]),
    .rx_axis_tlast(rx_mac_axis_tlast[1]),
    .rx_axis_tkeep(rx_mac_axis_tkeep[1]),
    .rx_axis_tuser(rx_mac_axis_tuser[1]),

    // status
    .tx_ovfout(),
    .tx_unfout(),
    .stat_tx_bad_fcs(),
    .stat_tx_broadcast(),
    .stat_tx_frame_error(),
    .stat_tx_local_fault(),
    .stat_tx_multicast(),
    .stat_tx_packet_1024_1518_bytes(),
    .stat_tx_packet_128_255_bytes(),
    .stat_tx_packet_1519_1522_bytes(),
    .stat_tx_packet_1523_1548_bytes(),
    .stat_tx_packet_1549_2047_bytes(),
    .stat_tx_packet_2048_4095_bytes(),
    .stat_tx_packet_256_511_bytes(),
    .stat_tx_packet_4096_8191_bytes(),
    .stat_tx_packet_512_1023_bytes(),
    .stat_tx_packet_64_bytes(),
    .stat_tx_packet_65_127_bytes(),
    .stat_tx_packet_8192_9215_bytes(),
    .stat_tx_packet_large(),
    .stat_tx_packet_small(),
    .stat_tx_total_bytes(),
    .stat_tx_total_good_bytes(),
    .stat_tx_total_good_packets(),
    .stat_tx_total_packets(),
    .stat_tx_unicast(),
    .stat_tx_vlan(),
    .rx_otn_bip8_0(),
    .rx_otn_bip8_1(),
    .rx_otn_bip8_2(),
    .rx_otn_bip8_3(),
    .rx_otn_bip8_4(),
    .rx_otn_data_0(),
    .rx_otn_data_1(),
    .rx_otn_data_2(),
    .rx_otn_data_3(),
    .rx_otn_data_4(),
    .rx_otn_ena(),
    .rx_otn_lane0(),
    .rx_otn_vlmarker(),
    .rx_preambleout(),
    .stat_rx_aligned(),
    .stat_rx_aligned_err(),
    .stat_rx_bad_code(),
    .stat_rx_bad_fcs(),
    .stat_rx_bad_preamble(),
    .stat_rx_bad_sfd(),
    .stat_rx_bip_err_0(),
    .stat_rx_bip_err_1(),
    .stat_rx_bip_err_10(),
    .stat_rx_bip_err_11(),
    .stat_rx_bip_err_12(),
    .stat_rx_bip_err_13(),
    .stat_rx_bip_err_14(),
    .stat_rx_bip_err_15(),
    .stat_rx_bip_err_16(),
    .stat_rx_bip_err_17(),
    .stat_rx_bip_err_18(),
    .stat_rx_bip_err_19(),
    .stat_rx_bip_err_2(),
    .stat_rx_bip_err_3(),
    .stat_rx_bip_err_4(),
    .stat_rx_bip_err_5(),
    .stat_rx_bip_err_6(),
    .stat_rx_bip_err_7(),
    .stat_rx_bip_err_8(),
    .stat_rx_bip_err_9(),
    .stat_rx_block_lock(),
    .stat_rx_broadcast(),
    .stat_rx_fragment(),
    .stat_rx_framing_err_0(),
    .stat_rx_framing_err_1(),
    .stat_rx_framing_err_10(),
    .stat_rx_framing_err_11(),
    .stat_rx_framing_err_12(),
    .stat_rx_framing_err_13(),
    .stat_rx_framing_err_14(),
    .stat_rx_framing_err_15(),
    .stat_rx_framing_err_16(),
    .stat_rx_framing_err_17(),
    .stat_rx_framing_err_18(),
    .stat_rx_framing_err_19(),
    .stat_rx_framing_err_2(),
    .stat_rx_framing_err_3(),
    .stat_rx_framing_err_4(),
    .stat_rx_framing_err_5(),
    .stat_rx_framing_err_6(),
    .stat_rx_framing_err_7(),
    .stat_rx_framing_err_8(),
    .stat_rx_framing_err_9(),
    .stat_rx_framing_err_valid_0(),
    .stat_rx_framing_err_valid_1(),
    .stat_rx_framing_err_valid_10(),
    .stat_rx_framing_err_valid_11(),
    .stat_rx_framing_err_valid_12(),
    .stat_rx_framing_err_valid_13(),
    .stat_rx_framing_err_valid_14(),
    .stat_rx_framing_err_valid_15(),
    .stat_rx_framing_err_valid_16(),
    .stat_rx_framing_err_valid_17(),
    .stat_rx_framing_err_valid_18(),
    .stat_rx_framing_err_valid_19(),
    .stat_rx_framing_err_valid_2(),
    .stat_rx_framing_err_valid_3(),
    .stat_rx_framing_err_valid_4(),
    .stat_rx_framing_err_valid_5(),
    .stat_rx_framing_err_valid_6(),
    .stat_rx_framing_err_valid_7(),
    .stat_rx_framing_err_valid_8(),
    .stat_rx_framing_err_valid_9(),
    .stat_rx_got_signal_os(),
    .stat_rx_hi_ber(),
    .stat_rx_inrangeerr(),
    .stat_rx_internal_local_fault(),
    .stat_rx_jabber(),
    .stat_rx_local_fault(stat_rx_local_fault[1]),
    .stat_rx_mf_err(),
    .stat_rx_mf_len_err(),
    .stat_rx_mf_repeat_err(),
    .stat_rx_misaligned(),
    .stat_rx_multicast(),
    .stat_rx_oversize(),
    .stat_rx_packet_1024_1518_bytes(),
    .stat_rx_packet_128_255_bytes(),
    .stat_rx_packet_1519_1522_bytes(),
    .stat_rx_packet_1523_1548_bytes(),
    .stat_rx_packet_1549_2047_bytes(),
    .stat_rx_packet_2048_4095_bytes(),
    .stat_rx_packet_256_511_bytes(),
    .stat_rx_packet_4096_8191_bytes(),
    .stat_rx_packet_512_1023_bytes(),
    .stat_rx_packet_64_bytes(),
    .stat_rx_packet_65_127_bytes(),
    .stat_rx_packet_8192_9215_bytes(),
    .stat_rx_packet_bad_fcs(),
    .stat_rx_packet_large(),
    .stat_rx_packet_small(),
    .stat_rx_received_local_fault(),
    .stat_rx_remote_fault(stat_rx_remote_fault[1]),
    .stat_rx_status(),
    .stat_rx_stomped_fcs(),
    .stat_rx_synced(),
    .stat_rx_synced_err(),
    .stat_rx_test_pattern_mismatch(),
    .stat_rx_toolong(),
    .stat_rx_total_bytes(),
    .stat_rx_total_good_bytes(),
    .stat_rx_total_good_packets(),
    .stat_rx_total_packets(),
    .stat_rx_truncated(),
    .stat_rx_undersize(),
    .stat_rx_unicast(),
    .stat_rx_vlan(),
    .stat_rx_pcsl_demuxed(),
    .stat_rx_pcsl_number_0(),
    .stat_rx_pcsl_number_1(),
    .stat_rx_pcsl_number_10(),
    .stat_rx_pcsl_number_11(),
    .stat_rx_pcsl_number_12(),
    .stat_rx_pcsl_number_13(),
    .stat_rx_pcsl_number_14(),
    .stat_rx_pcsl_number_15(),
    .stat_rx_pcsl_number_16(),
    .stat_rx_pcsl_number_17(),
    .stat_rx_pcsl_number_18(),
    .stat_rx_pcsl_number_19(),
    .stat_rx_pcsl_number_2(),
    .stat_rx_pcsl_number_3(),
    .stat_rx_pcsl_number_4(),
    .stat_rx_pcsl_number_5(),
    .stat_rx_pcsl_number_6(),
    .stat_rx_pcsl_number_7(),
    .stat_rx_pcsl_number_8(),
    .stat_rx_pcsl_number_9(),
    .stat_rx_rsfec_am_lock0(),
    .stat_rx_rsfec_am_lock1(),
    .stat_rx_rsfec_am_lock2(),
    .stat_rx_rsfec_am_lock3(),
    .stat_rx_rsfec_corrected_cw_inc(),
    .stat_rx_rsfec_cw_inc(),
    .stat_rx_rsfec_err_count0_inc(),
    .stat_rx_rsfec_err_count1_inc(),
    .stat_rx_rsfec_err_count2_inc(),
    .stat_rx_rsfec_err_count3_inc(),
    .stat_rx_rsfec_hi_ser(),
    .stat_rx_rsfec_lane_alignment_status(),
    .stat_rx_rsfec_lane_fill_0(),
    .stat_rx_rsfec_lane_fill_1(),
    .stat_rx_rsfec_lane_fill_2(),
    .stat_rx_rsfec_lane_fill_3(),
    .stat_rx_rsfec_lane_mapping(),
    .stat_rx_rsfec_uncorrected_cw_inc(),

    // configuration
    .core_drp_reset(1'b0),
    .drp_clk(clk_gt_freerun),
    .drp_addr(10'b0),
    .drp_di(16'b0),
    .drp_en(1'b0),
    .drp_do(),
    .drp_rdy(),
    .drp_we(1'b0)
);

endmodule
`default_nettype wire
