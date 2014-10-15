* WARNING : please consider following remarks before usage
*
* 1) All models are a tradeoff between accuracy and complexity (ie. simulation
* time).
* 2) Macromodels are not a substitute to breadboarding, they rather confirm the
* validity of a design approach and help to select surrounding component values.
*
* 3) A macromodel emulates the NOMINAL performance of a TYPICAL device within
* SPECIFIED OPERATING CONDITIONS (ie. temperature, supply voltage, etc.).
* Thus the macromodel is often not as exhaustive as the datasheet, its goal
* is to illustrate the main parameters of the product.
*
* 4) Data issued from macromodels used outside of its specified conditions
* (Vcc, Temperature, etc) or even worse: outside of the device operating
* conditions (Vcc, Vicm, etc) are not reliable in any way.
*
****************************************************************************
****
****  TSV91X  Spice macromodel subckt
****
****  Revision Date: 10/06/08
****
*********** CONNECTIONS: 
****           INVERTING INPUT
****            |  NON-INVERTING INPUT
****            |   |  OUTPUT 
****            |   |   |  POSITIVE POWER SUPPLY
****            |   |   |   |  NEGATIVE POWER SUPPLY
****            |   |   |   |   | 
****            |   |   |   |   | 
.SUBCKT TSV912 VM  VP  VS  VCCP VCCN
    M_NMOS2 VO_DIFF_MINUS VM VEE_N VCCN_ENHANCED MOS_N L={L} W={W}
    M_NMOS1 VO_DIFF_PLUS NET0158 VEE_N VCCN_ENHANCED MOS_N L={L} W={W}
    IEE_N VEE_N VCCN_ENHANCED DC {IEE}
    HILIM_ICCP NET0241 0 V_OUTVLIM_HIGH -235
    HILIM_ICCN NET0249 0 V_OUTVLIM_LOW 235
    V_OUTVLIM_HIGH NET0201 NET0177 DC -770m
    VVLIM_LOW_VB NET0109 NET0110 DC -770m
    VOS NET0158 VP DC 0
    VPROT_IN_P_VCCP NET0123 NET0134 DC {V_DPROT}
    V_ENHANCE_VCCN VCCN_ENHANCED VCCN DC -360m
    VVLIM_HIGH_VB NET0136 NET0153 DC -770m
    V_ENHANCE_VCCP VCCP_ENHANCED VCCP DC -500m
    VPROT_IN_M_VCCN NET0116 NET0192 DC {V_DPROT}
    VPROT_IN_P_VCCN NET0115 NET096 DC {V_DPROT}
    V_OUTVLIM_LOW NET0182 NET0183 DC -770m
    VPROT_IN_M_VCCP NET0190 NET0135 DC {V_DPROT}
    VREADIO NET0342 VS DC 0
    DILIM_SOURCE VB_3 VB_3_SOURCE DIODE_ILIM
    DILIM_ICCN NET0206 VB_3 DIODE_ILIM
    D_OUTVLIM_LOW NET0183 VB_3 DIODE_VLIM
    DVLIM_HIGH_VB VB NET0136 DIODE_VLIM
    DILIM_SINK VB_3_SINK VB_3 DIODE_ILIM
    DPROT_IN_M_VCCP VM NET0135 DIODE_VLIM
    DVLIM_LOW_VB NET0110 VB DIODE_VLIM
    D_OUTVLIM_HIGH VB_3 NET0201 DIODE_VLIM
    DPROT_IN_M_VCCN NET0116 VM DIODE_VLIM
    DPROT_IN_P_VCCP NET0158 NET0134 DIODE_VLIM
    DPROT_IN_P_VCCN NET0115 NET0158 DIODE_VLIM
    DILIM_ICCP VB_3 NET0203 DIODE_ILIM
    CGATE_VP_PARASITIC VP VREF 7p
    CCOMP VB VB_2 {Ccomp}
    CBD2_PARASITIC VO_DIFF_MINUS VCCN_ENHANCED 7p
    CGD2_PARASITIC VO_DIFF_MINUS VM 300.0f
    CBD1_PARASITIC VO_DIFF_PLUS VCCN_ENHANCED 7p
    CDIFF_PARASITIC VO_DIFF_PLUS VO_DIFF_MINUS 200.0f
    CGATE_VM_PARASITIC VM VREF 7p
    CGD1_PARASITIC VO_DIFF_PLUS NET0158 300.0f
    
    *Eldo:
    *E_VDEP_SOURCE_2 VAL_VDEP_SOURCE_FILTERED 0
    *+VALUE={VALIF(V(val_vdep_source)>=0, 0, V(val_vdep_source))}
    *PSpice: 
    E_VDEP_SOURCE_2 VAL_VDEP_SOURCE_FILTERED 0
    +VALUE={IF(V(val_vdep_source)>=0, 0, V(val_vdep_source))}

    E_VDEP_SOURCE_1 VAL_VDEP_SOURCE 0 VALUE={ 174.5 -5000*I(VreadIo)}
    EMEAS_VOUT_DIFF VOUT_DIFF 0 VO_DIFF_PLUS VO_DIFF_MINUS 1.0
    E_ICCSAT_LOW ICC_OUT_LOW 0 POLY(1) VCCP VCCN 0.0023261764705882347
+-5.5705882352941006E-5 5.294117647058802E-6
    E_ICCSAT_HIGH ICC_OUT_HIGH 0 POLY(1) VCCP VCCN 0.0023261764705882347
+-5.5705882352941006E-5 5.294117647058802E-6
    EVLIM_HIGH_VB NET0153 0 VCCP 0 1.0
    EILIM_SOURCE VB_3_SOURCE VDEP_SOURCE VB_3 0 1.0
    
    *Eldo:
    *E_RO1 VB_3 NET0342 VALUE={VALIF(I(VreadIo)>0,
    *+V(Ro1_Voh)*I(VreadIo),V(Ro1_Vol)*I(VreadIo))}
    *PSpice:
    E_RO1 VB_3 NET0342 VALUE={IF(I(VreadIo)>0,
    +V(Ro1_Voh)*I(VreadIo),V(Ro1_Vol)*I(VreadIo))}

    EVLIM_LOW_VB NET0109 0 VCCN 0 1.0
    EILIM_ICCN NET0206 NET0249 VB_3 0 1.0
    E2_REF NET0238 0 VCCN 0 1.0
    EVLIM_HIGH_VOUT NET0177 0 VCCP 0 1.0
    E_VREF VREF 0 NET0250 0 1.0

    *Eldo:
    *E_VDEP_SINK_2 VAL_VDEP_SINK_FILTERED 0
    *+VALUE={VALIF(V(val_vdep_sink)<=0 , 0 , V(val_vdep_sink))}
    *E_VDEP_SINK_3 VDEP_SINK 0 VALUE={VALIF( abs(I(VreadIo))<1m , 0 ,
    *+V(val_vdep_sink_filtered))}
    *E_VDEP_SOURCE_3 VDEP_SOURCE 0 VALUE={VALIF( abs(I(VreadIo))<1m , 0 ,
    *+V(val_vdep_source_filtered))}
    *PSpice:
    E_VDEP_SINK_2 VAL_VDEP_SINK_FILTERED 0
    +VALUE={IF(V(val_vdep_sink)<=0 , 0 , V(val_vdep_sink))}
    E_VDEP_SINK_3 VDEP_SINK 0 VALUE={IF( abs(I(VreadIo))<1m , 0 ,
    +V(val_vdep_sink_filtered))}
    E_VDEP_SOURCE_3 VDEP_SOURCE 0 VALUE={IF( abs(I(VreadIo))<1m , 0 ,
    +V(val_vdep_source_filtered))}

    EILIM_SINK VB_3_SINK VDEP_SINK VB_3 0 1.0

    *Eldo:    
    *E_RO1_VOL RO1_VOL 0 PWL(1) VCCP VCCN ( 2.5 , 14 ) ( 3.3 , 11 ) ( 5.0 , 7.5 )
    *PSpice:
    E_RO1_VOL RO1_VOL 0 VALUE={TABLE(V(VCCP,VCCN), 2.5 , 14 , 3.3 , 11 , 5.0 , 7.5 )}
    
    E1_REF NET0210 0 VCCP 0 1.0
    E_VDEP_SINK_1 VAL_VDEP_SINK 0 VALUE={ -159.5 -5000*I(VreadIo)}
    EVLIM_LOW_VOUT NET0182 0 VCCN 0 1.0
    
    *Eldo:
    *E_RO1_VOH RO1_VOH 0 PWL(1) VCCP VCCN ( 2.5 , 14 ) ( 3.3 , 11 ) ( 5.0 , 7.5 )
    *PSpice:
    E_RO1_VOH RO1_VOH 0 VALUE={TABLE(V(VCCP,VCCN), 2.5 , 14 , 3.3 , 11 , 5.0 , 7.5 )}

    EILIM_ICCP NET0203 NET0241 VB_3 0 1.0
    RO2_2 VB_3 VB_2 {Ro2_2}
    R_ICCSAT_LOW ICC_OUT_LOW 0 1K
    RPROT_IN_P_VCCP NET0123 VCCP 1K
    RPROT_IN_M_VCCP VCCP NET0190 1K
    RD1 VCCP_ENHANCED VO_DIFF_PLUS {RD}
    RD2 VCCP_ENHANCED VO_DIFF_MINUS {RD}
    RO2_1 VREF VB_2 {Ro2_1}
    R1_REF NET0210 NET0250 1Meg
    R_ICCSAT_HIGH ICC_OUT_HIGH 0 1K
    R1 VB VREF {R1}
    RPROT_IN_M_VCCN VCCN NET0192 15K
    R2_REF NET0250 NET0238 1Meg
    RPROT_IN_P_VCCN NET096 VCCN 15K
    
    *Eldo:
    *G_ICCSAT_OUTLOW VCCP VCCN VALUE={VALIF(I(V_OUTVLIM_LOW)>1u ,
    *+V(Icc_out_low) , 0)}
    *PSpice:
    G_ICCSAT_OUTLOW VCCP VCCN VALUE={IF(I(V_OUTVLIM_LOW)>1u ,
    +V(Icc_out_low) , 0)}

    *Eldo:
    *G_IOUT_SOUCED VCCP 0 VALUE={VALIF(I(VreadIo)>0, I(VreadIo),0)}
    *G_ICCSAT_OUTHIGH VCCP VCCN VALUE={VALIF(I(V_OUTVLIM_HIGH)>1u ,
    *+V(Icc_out_high), 0)}
    *PSpice:
    G_IOUT_SOUCED VCCP 0 VALUE={IF(I(VreadIo)>0, I(VreadIo),0)}
    G_ICCSAT_OUTHIGH VCCP VCCN VALUE={IF(I(V_OUTVLIM_HIGH)>1u ,
    +V(Icc_out_high), 0)}    

    G_I_VB VB_2 VREF POLY(1) VB VREF 7.773528173127232E-19
+0.010310813241023556 0 0.1401392933601382
    GM1 VREF VB VOUT_DIFF 0 {1/RD}
    G_ICC VCCP VCCN POLY(1) VCCP VCCN +6.321735294117647E-4
++5.570588235294106E-5 -5.294117647058808E-6
   
    *Eldo:
    *G_IOUT_SINKED VCCN 0 VALUE={VALIF(I(VreadIo)>0, 0, I(VreadIo))}
    *PSpice:
    G_IOUT_SINKED VCCN 0 VALUE={IF(I(VreadIo)>0, 0, I(VreadIo))}    
.ENDS
*** End of subcircuit definition.

.PARAM RD=1k
.PARAM Ccomp=8.5p
.PARAM IEE=41.65u
.PARAM A0=97.93103448E3
.PARAM Ro=17587.2
.PARAM W=20u
.PARAM L=2u
.PARAM gm_mos=0.0004565941678681337
.PARAM GB=11m
.PARAM Ro1=11
.PARAM Ro2_2=1e-3
.PARAM Ro2_1={Ro - Ro2_2 - Ro1}
.PARAM R1={A0/(gm_mos*GB*Ro2_1)}
.PARAM V_DPROT=0.6

*Eldo:
*.MODEL MOS_N  NMOS LEVEL=1 MODTYPE=ELDO VTO=+0.65  KP=500E-6 
*.MODEL DIODE_VLIM D LEVEL=1 MODTYPE=ELDO IS=0.8E-15 
*.MODEL DIODE_ILIM D LEVEL=1 MODTYPE=ELDO IS=0.8E-15 
*.MODEL DX D LEVEL=1 MODTYPE=ELDO IS=1E-14
*PSpice:
.MODEL MOS_N  NMOS LEVEL=1 VTO=+0.65  KP=500E-6 
.MODEL DIODE_VLIM D LEVEL=1 IS=0.8E-15 
.MODEL DIODE_ILIM D LEVEL=1 IS=0.8E-15 
.MODEL DX D LEVEL=1 IS=1E-14
*********************************************************************************** 
