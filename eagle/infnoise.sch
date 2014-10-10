<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.1.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="infnoise">
<description>Parts for the Infinite Noise Generator</description>
<packages>
<package name="0402">
<description>&lt;b&gt;Chip Ferrite Bead for GHz Noise&lt;/b&gt;&lt;p&gt;
</description>
<wire x1="-0.245" y1="0.224" x2="0.245" y2="0.224" width="0.1524" layer="51"/>
<wire x1="0.245" y1="-0.224" x2="-0.245" y2="-0.224" width="0.1524" layer="51"/>
<wire x1="-1.473" y1="0.483" x2="1.473" y2="0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.483" x2="1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.483" x2="-1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.483" x2="-1.473" y2="0.483" width="0.0508" layer="39"/>
<smd name="1" x="-0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<text x="-0.635" y="0.735" size="0.4064" layer="25">&gt;NAME</text>
<rectangle x1="-0.554" y1="-0.3048" x2="-0.254" y2="0.2951" layer="51"/>
<rectangle x1="0.2588" y1="-0.3048" x2="0.5588" y2="0.2951" layer="51"/>
<rectangle x1="-0.1999" y1="-0.4001" x2="0.1999" y2="0.4001" layer="35"/>
</package>
<package name="QFN24-4X4">
<description>&lt;b&gt;UF Package 24-Lead (4mm × 4mm) Plastic QFN&lt;/b&gt;&lt;p&gt;
Source: http://cds.linear.com/docs/Datasheet/5598f.pdf</description>
<wire x1="-1.9" y1="-1.9" x2="1.9" y2="-1.9" width="0.2032" layer="51"/>
<wire x1="1.9" y1="-1.9" x2="1.9" y2="1.9" width="0.2032" layer="51"/>
<wire x1="1.9" y1="1.9" x2="-1.9" y2="1.9" width="0.2032" layer="51"/>
<wire x1="-1.9" y1="1.9" x2="-1.9" y2="-1.9" width="0.2032" layer="51"/>
<circle x="-1.345" y="1.345" radius="0.155" width="0" layer="51"/>
<smd name="1" x="-1.9" y="1.25" dx="0.7" dy="0.25" layer="1"/>
<smd name="2" x="-1.9" y="0.75" dx="0.7" dy="0.25" layer="1"/>
<smd name="3" x="-1.9" y="0.25" dx="0.7" dy="0.25" layer="1"/>
<smd name="4" x="-1.9" y="-0.25" dx="0.7" dy="0.25" layer="1"/>
<smd name="5" x="-1.9" y="-0.75" dx="0.7" dy="0.25" layer="1"/>
<smd name="6" x="-1.9" y="-1.25" dx="0.7" dy="0.25" layer="1"/>
<smd name="7" x="-1.25" y="-1.9" dx="0.7" dy="0.25" layer="1" rot="R90"/>
<smd name="8" x="-0.75" y="-1.9" dx="0.7" dy="0.25" layer="1" rot="R90"/>
<smd name="9" x="-0.25" y="-1.9" dx="0.7" dy="0.25" layer="1" rot="R90"/>
<smd name="10" x="0.25" y="-1.9" dx="0.7" dy="0.25" layer="1" rot="R90"/>
<smd name="11" x="0.75" y="-1.9" dx="0.7" dy="0.25" layer="1" rot="R90"/>
<smd name="12" x="1.25" y="-1.9" dx="0.7" dy="0.25" layer="1" rot="R90"/>
<smd name="13" x="1.9" y="-1.25" dx="0.7" dy="0.25" layer="1" rot="R180"/>
<smd name="14" x="1.9" y="-0.75" dx="0.7" dy="0.25" layer="1" rot="R180"/>
<smd name="15" x="1.9" y="-0.25" dx="0.7" dy="0.25" layer="1" rot="R180"/>
<smd name="16" x="1.9" y="0.25" dx="0.7" dy="0.25" layer="1" rot="R180"/>
<smd name="17" x="1.9" y="0.75" dx="0.7" dy="0.25" layer="1" rot="R180"/>
<smd name="18" x="1.9" y="1.25" dx="0.7" dy="0.25" layer="1" rot="R180"/>
<smd name="19" x="1.25" y="1.9" dx="0.7" dy="0.25" layer="1" rot="R270"/>
<smd name="20" x="0.75" y="1.9" dx="0.7" dy="0.25" layer="1" rot="R270"/>
<smd name="21" x="0.25" y="1.9" dx="0.7" dy="0.25" layer="1" rot="R270"/>
<smd name="22" x="-0.25" y="1.9" dx="0.7" dy="0.25" layer="1" rot="R270"/>
<smd name="23" x="-0.75" y="1.9" dx="0.7" dy="0.25" layer="1" rot="R270"/>
<smd name="24" x="-1.25" y="1.9" dx="0.7" dy="0.25" layer="1" rot="R270"/>
<smd name="EXP" x="0" y="0" dx="2.5" dy="2.5" layer="1" stop="no"/>
<text x="1.27" y="2.627" size="0.6096" layer="25" align="bottom-center">&gt;NAME</text>
<rectangle x1="-1.95" y1="1.525" x2="-1.525" y2="1.975" layer="21"/>
<rectangle x1="-1.4" y1="-1.4" x2="1.4" y2="1.4" layer="29"/>
</package>
<package name="DFN6-2X3">
<description>&lt;b&gt;DCB Package 8-Lead Plastic DFN (2mm × 2mm)&lt;/b&gt;&lt;p&gt;
Source: http://www.linear.com/pc/downloadDocument.do?navId=H0,C1,C1003,C1042,C1032,C1063,P26952,D16357</description>
<wire x1="-1.0922" y1="-0.9" x2="-1.0922" y2="0.9" width="0.2032" layer="21"/>
<wire x1="-1.1" y1="1.1" x2="1.1" y2="1.1" width="0.2032" layer="51"/>
<wire x1="1.0922" y1="0.9" x2="1.0922" y2="-0.9" width="0.2032" layer="21"/>
<wire x1="1.1" y1="-1.1" x2="-1.1" y2="-1.1" width="0.2032" layer="51"/>
<smd name="EXP" x="0" y="0" dx="1.6" dy="0.45" layer="1"/>
<smd name="1" x="-0.75" y="-1.025" dx="0.28" dy="0.75" layer="1"/>
<smd name="2" x="-0.25" y="-1.025" dx="0.28" dy="0.75" layer="1"/>
<smd name="3" x="0.25" y="-1.025" dx="0.28" dy="0.75" layer="1"/>
<smd name="4" x="0.75" y="-1.025" dx="0.28" dy="0.75" layer="1" rot="R180"/>
<smd name="5" x="0.75" y="1.025" dx="0.28" dy="0.75" layer="1" rot="R180"/>
<smd name="6" x="0.25" y="1.025" dx="0.28" dy="0.75" layer="1" rot="R180"/>
<text x="-0.005" y="1.705" size="0.6096" layer="25" align="bottom-center">&gt;NAME</text>
<rectangle x1="-0.777" y1="-0.517" x2="-0.227" y2="-0.392" layer="21"/>
<smd name="7" x="-0.25" y="1.025" dx="0.28" dy="0.75" layer="1" rot="R180"/>
<smd name="8" x="-0.75" y="1.025" dx="0.28" dy="0.75" layer="1" rot="R180"/>
</package>
<package name="SSOP8-P-0.50A">
<smd name="1" x="-0.75" y="-1.35" dx="0.6" dy="0.25" layer="1" rot="R90"/>
<smd name="2" x="-0.25" y="-1.35" dx="0.6" dy="0.25" layer="1" rot="R90"/>
<smd name="3" x="0.25" y="-1.35" dx="0.6" dy="0.25" layer="1" rot="R90"/>
<smd name="4" x="0.75" y="-1.35" dx="0.6" dy="0.25" layer="1" rot="R90"/>
<smd name="5" x="0.75" y="1.35" dx="0.6" dy="0.25" layer="1" rot="R270"/>
<smd name="6" x="0.25" y="1.35" dx="0.6" dy="0.25" layer="1" rot="R270"/>
<smd name="7" x="-0.25" y="1.35" dx="0.6" dy="0.25" layer="1" rot="R270"/>
<smd name="8" x="-0.75" y="1.35" dx="0.6" dy="0.25" layer="1" rot="R270"/>
<wire x1="-1.1" y1="1.2" x2="-1.1" y2="-1.2" width="0.127" layer="21"/>
<wire x1="1.1" y1="1.2" x2="1.1" y2="-1.2" width="0.127" layer="21"/>
<circle x="-0.8" y="-0.7" radius="0.1" width="0.127" layer="21"/>
<text x="-0.035" y="2.005" size="0.6096" layer="25" align="bottom-center">&gt;NAME</text>
</package>
<package name="SC70-5L">
<description>&lt;b&gt;SC-70 Package&lt;/b&gt;</description>
<wire x1="1.1" y1="-0.5" x2="-1.1" y2="-0.5" width="0.2032" layer="51"/>
<wire x1="-1.1" y1="-0.5" x2="-1.1" y2="0.5" width="0.2032" layer="21"/>
<wire x1="-1.1" y1="0.5" x2="1.1" y2="0.5" width="0.2032" layer="51"/>
<wire x1="1.1" y1="0.5" x2="1.1" y2="-0.5" width="0.2032" layer="21"/>
<circle x="-0.65" y="-0.2" radius="0.15" width="0" layer="21"/>
<smd name="4" x="0.65" y="0.85" dx="0.35" dy="0.8" layer="1"/>
<smd name="5" x="-0.65" y="0.85" dx="0.35" dy="0.8" layer="1"/>
<smd name="1" x="-0.65" y="-0.85" dx="0.35" dy="0.8" layer="1"/>
<smd name="2" x="0" y="-0.85" dx="0.35" dy="0.8" layer="1"/>
<smd name="3" x="0.65" y="-0.85" dx="0.35" dy="0.8" layer="1"/>
<text x="0.2" y="-0.15" size="0.3048" layer="25" align="bottom-center">&gt;NAME</text>
<rectangle x1="0.5" y1="0.6" x2="0.8" y2="1.1" layer="51"/>
<rectangle x1="-0.8" y1="0.6" x2="-0.5" y2="1.1" layer="51"/>
<rectangle x1="-0.8" y1="-1.1" x2="-0.5" y2="-0.6" layer="51"/>
<rectangle x1="-0.15" y1="-1.1" x2="0.15" y2="-0.6" layer="51"/>
<rectangle x1="0.5" y1="-1.1" x2="0.8" y2="-0.6" layer="51"/>
</package>
<package name="0603-CAP">
<wire x1="-1.473" y1="0.983" x2="1.473" y2="0.983" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.983" x2="1.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.983" x2="-1.473" y2="-0.983" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.983" x2="-1.473" y2="0.983" width="0.0508" layer="39"/>
<wire x1="-0.356" y1="0.432" x2="0.356" y2="0.432" width="0.1016" layer="51"/>
<wire x1="-0.356" y1="-0.419" x2="0.356" y2="-0.419" width="0.1016" layer="51"/>
<smd name="1" x="-0.85" y="0" dx="1.1" dy="1" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1.1" dy="1" layer="1"/>
<text x="-0.889" y="0.762" size="0.4064" layer="25" font="vector">&gt;NAME</text>
<rectangle x1="-0.8382" y1="-0.4699" x2="-0.3381" y2="0.4801" layer="51"/>
<rectangle x1="0.3302" y1="-0.4699" x2="0.8303" y2="0.4801" layer="51"/>
</package>
<package name="0402-CAP">
<description>&lt;b&gt;CAPACITOR&lt;/b&gt;&lt;p&gt;
chip</description>
<wire x1="-0.245" y1="0.224" x2="0.245" y2="0.224" width="0.1524" layer="51"/>
<wire x1="0.245" y1="-0.224" x2="-0.245" y2="-0.224" width="0.1524" layer="51"/>
<wire x1="-1.473" y1="0.483" x2="1.473" y2="0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.483" x2="1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.483" x2="-1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.483" x2="-1.473" y2="0.483" width="0.0508" layer="39"/>
<smd name="1" x="-0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<text x="-0.889" y="0.6985" size="0.4064" layer="25">&gt;NAME</text>
<rectangle x1="-0.554" y1="-0.3048" x2="-0.254" y2="0.2951" layer="51"/>
<rectangle x1="0.2588" y1="-0.3048" x2="0.5588" y2="0.2951" layer="51"/>
</package>
<package name="0402-RES">
<description>&lt;b&gt;RESISTOR&lt;/b&gt;&lt;p&gt;
chip</description>
<wire x1="-0.245" y1="0.224" x2="0.245" y2="0.224" width="0.1524" layer="51"/>
<wire x1="0.245" y1="-0.224" x2="-0.245" y2="-0.224" width="0.1524" layer="51"/>
<wire x1="-1.473" y1="0.483" x2="1.473" y2="0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="0.483" x2="1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="1.473" y1="-0.483" x2="-1.473" y2="-0.483" width="0.0508" layer="39"/>
<wire x1="-1.473" y1="-0.483" x2="-1.473" y2="0.483" width="0.0508" layer="39"/>
<smd name="1" x="-0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.7" dy="0.9" layer="1"/>
<text x="-0.889" y="0.6985" size="0.4064" layer="25">&gt;NAME</text>
<rectangle x1="-0.554" y1="-0.3048" x2="-0.254" y2="0.2951" layer="51"/>
<rectangle x1="0.2588" y1="-0.3048" x2="0.5588" y2="0.2951" layer="51"/>
</package>
<package name="USB-A-PCB">
<wire x1="-5" y1="6" x2="3.7" y2="6" width="0.127" layer="51"/>
<wire x1="3.7" y1="6" x2="3.7" y2="-6" width="0.127" layer="51"/>
<wire x1="3.7" y1="-6" x2="-5" y2="-6" width="0.127" layer="51"/>
<wire x1="-5" y1="-6" x2="-5" y2="6" width="0.127" layer="51"/>
<smd name="5V" x="-0.2" y="-3.5" dx="7.5" dy="1.5" layer="1"/>
<smd name="USB_M" x="0.3" y="-1" dx="6.5" dy="1" layer="1"/>
<smd name="USB_P" x="0.3" y="1" dx="6.5" dy="1" layer="1"/>
<smd name="GND" x="-0.2" y="3.5" dx="7.5" dy="1.5" layer="1"/>
<text x="-1.27" y="5.08" size="0.4064" layer="25">&gt;Name</text>
<text x="-1.27" y="-5.08" size="0.4064" layer="27">&gt;Value</text>
</package>
<package name="TSOP-5">
<description>&lt;b&gt;TSOP-5&lt;/b&gt; PLASTIC PACKAGE CASE 483-02&lt;p&gt;
Source: http://www.onsemi.com/pub/Collateral/MC34164-D.PDF</description>
<wire x1="-1.5" y1="0.8" x2="1.5" y2="0.8" width="0.1016" layer="21"/>
<wire x1="1.5" y1="0.8" x2="1.5" y2="-0.8" width="0.1016" layer="21"/>
<wire x1="1.5" y1="-0.8" x2="-1.5" y2="-0.8" width="0.1016" layer="21"/>
<wire x1="-1.5" y1="-0.8" x2="-1.5" y2="0.8" width="0.1016" layer="21"/>
<circle x="-1.075" y="-0.5" radius="0.2136" width="0" layer="21"/>
<smd name="1" x="-0.99" y="-1.2" dx="0.7" dy="1" layer="1"/>
<smd name="2" x="0" y="-1.2" dx="0.7" dy="1" layer="1"/>
<smd name="3" x="0.99" y="-1.2" dx="0.7" dy="1" layer="1"/>
<smd name="4" x="0.99" y="1.2" dx="0.7" dy="1" layer="1" rot="R180"/>
<smd name="5" x="-0.99" y="1.2" dx="0.7" dy="1" layer="1" rot="R180"/>
<text x="-0.04" y="2.04" size="0.6096" layer="25" align="bottom-center">&gt;NAME</text>
<rectangle x1="-1.2" y1="0.825" x2="-0.7" y2="1.5" layer="51"/>
<rectangle x1="0.7" y1="0.825" x2="1.2" y2="1.5" layer="51"/>
<rectangle x1="0.7" y1="-1.5" x2="1.2" y2="-0.825" layer="51"/>
<rectangle x1="-0.25" y1="-1.5" x2="0.25" y2="-0.825" layer="51"/>
<rectangle x1="-1.2" y1="-1.5" x2="-0.7" y2="-0.825" layer="51"/>
</package>
</packages>
<symbols>
<symbol name="L">
<text x="-3.81" y="1.3716" size="1.778" layer="95">&gt;NAME</text>
<text x="-3.81" y="-2.921" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-2.54" y1="-0.889" x2="2.54" y2="0.889" layer="94"/>
<pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
</symbol>
<symbol name="FT240X">
<wire x1="-22.86" y1="22.86" x2="22.86" y2="22.86" width="0.254" layer="94"/>
<wire x1="22.86" y1="22.86" x2="22.86" y2="-22.86" width="0.254" layer="94"/>
<wire x1="22.86" y1="-22.86" x2="-22.86" y2="-22.86" width="0.254" layer="94"/>
<wire x1="-22.86" y1="-22.86" x2="-22.86" y2="22.86" width="0.254" layer="94"/>
<pin name="D1" x="-27.94" y="12.7" length="middle"/>
<pin name="D7" x="-27.94" y="7.62" length="middle"/>
<pin name="GND" x="-27.94" y="2.54" length="middle"/>
<pin name="D5" x="-27.94" y="-2.54" length="middle"/>
<pin name="D6" x="-27.94" y="-7.62" length="middle"/>
<pin name="D3" x="-27.94" y="-12.7" length="middle"/>
<pin name="SI/WU#" x="-12.7" y="-27.94" length="middle" rot="R90"/>
<pin name="RD#" x="-7.62" y="-27.94" length="middle" rot="R90"/>
<pin name="WR#" x="-2.54" y="-27.94" length="middle" rot="R90"/>
<pin name="USBDP" x="2.54" y="-27.94" length="middle" rot="R90"/>
<pin name="USBDM" x="7.62" y="-27.94" length="middle" rot="R90"/>
<pin name="3V3OUT" x="12.7" y="-27.94" length="middle" rot="R90"/>
<pin name="RESET#" x="27.94" y="-12.7" length="middle" rot="R180"/>
<pin name="VCORE" x="27.94" y="-7.62" length="middle" rot="R180"/>
<pin name="VCC" x="27.94" y="-2.54" length="middle" rot="R180"/>
<pin name="TXE#" x="27.94" y="7.62" length="middle" rot="R180"/>
<pin name="RXF#" x="27.94" y="12.7" length="middle" rot="R180"/>
<pin name="CBUS6" x="12.7" y="27.94" length="middle" rot="R270"/>
<pin name="CBUS5" x="7.62" y="27.94" length="middle" rot="R270"/>
<pin name="D0" x="2.54" y="27.94" length="middle" rot="R270"/>
<pin name="D4" x="-2.54" y="27.94" length="middle" rot="R270"/>
<pin name="D2" x="-7.62" y="27.94" length="middle" rot="R270"/>
<pin name="VCCIO" x="-12.7" y="27.94" length="middle" rot="R270"/>
<text x="-7.62" y="7.62" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="0" size="1.778" layer="95">&gt;VALUE</text>
</symbol>
<symbol name="TSV912">
<wire x1="-20.32" y1="15.24" x2="22.86" y2="15.24" width="0.254" layer="94"/>
<wire x1="22.86" y1="15.24" x2="22.86" y2="5.08" width="0.254" layer="94"/>
<wire x1="22.86" y1="5.08" x2="22.86" y2="0" width="0.254" layer="94"/>
<wire x1="22.86" y1="0" x2="22.86" y2="-5.08" width="0.254" layer="94"/>
<wire x1="22.86" y1="-5.08" x2="22.86" y2="-10.16" width="0.254" layer="94"/>
<wire x1="22.86" y1="-10.16" x2="-20.32" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-20.32" y1="-10.16" x2="-20.32" y2="0" width="0.254" layer="94"/>
<pin name="OUT1" x="-25.4" y="10.16" length="middle"/>
<pin name="IN1-" x="-25.4" y="5.08" length="middle"/>
<pin name="IN1+" x="-25.4" y="0" length="middle"/>
<pin name="GND" x="-25.4" y="-5.08" length="middle"/>
<pin name="IN2+" x="27.94" y="-5.08" length="middle" rot="R180"/>
<pin name="IN2-" x="27.94" y="0" length="middle" rot="R180"/>
<pin name="OUT2" x="27.94" y="5.08" length="middle" rot="R180"/>
<pin name="VCC" x="27.94" y="10.16" length="middle" rot="R180"/>
<polygon width="0.254" layer="94">
<vertex x="-12.7" y="7.62"/>
<vertex x="-12.7" y="-2.54"/>
<vertex x="-2.54" y="2.54"/>
</polygon>
<wire x1="-20.32" y1="0" x2="-20.32" y2="5.08" width="0.254" layer="94"/>
<wire x1="-20.32" y1="5.08" x2="-20.32" y2="10.16" width="0.254" layer="94"/>
<wire x1="-20.32" y1="10.16" x2="-20.32" y2="15.24" width="0.254" layer="94"/>
<wire x1="-20.32" y1="5.08" x2="-12.7" y2="5.08" width="0.254" layer="94"/>
<wire x1="-20.32" y1="0" x2="-12.7" y2="0" width="0.254" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="10.16" width="0.254" layer="94"/>
<wire x1="0" y1="10.16" x2="-20.32" y2="10.16" width="0.254" layer="94"/>
<polygon width="0.254" layer="94">
<vertex x="15.24" y="-7.62"/>
<vertex x="15.24" y="2.54"/>
<vertex x="5.08" y="-2.54"/>
</polygon>
<wire x1="22.86" y1="0" x2="15.24" y2="0" width="0.254" layer="94"/>
<wire x1="22.86" y1="-5.08" x2="15.24" y2="-5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="-2.54" x2="2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="2.54" y2="5.08" width="0.254" layer="94"/>
<wire x1="2.54" y1="5.08" x2="22.86" y2="5.08" width="0.254" layer="94"/>
<text x="-7.62" y="17.78" size="1.778" layer="95">&gt;NAME</text>
<text x="-7.62" y="-12.7" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="TC75W57">
<wire x1="-20.32" y1="15.24" x2="22.86" y2="15.24" width="0.254" layer="94"/>
<wire x1="22.86" y1="15.24" x2="22.86" y2="5.08" width="0.254" layer="94"/>
<wire x1="22.86" y1="5.08" x2="22.86" y2="0" width="0.254" layer="94"/>
<wire x1="22.86" y1="0" x2="22.86" y2="-5.08" width="0.254" layer="94"/>
<wire x1="22.86" y1="-5.08" x2="22.86" y2="-10.16" width="0.254" layer="94"/>
<wire x1="22.86" y1="-10.16" x2="-20.32" y2="-10.16" width="0.254" layer="96"/>
<wire x1="-20.32" y1="-10.16" x2="-20.32" y2="0" width="0.254" layer="94"/>
<pin name="OUT1" x="-25.4" y="10.16" length="middle"/>
<pin name="IN1-" x="-25.4" y="5.08" length="middle"/>
<pin name="IN1+" x="-25.4" y="0" length="middle"/>
<pin name="VSS" x="-25.4" y="-5.08" length="middle"/>
<pin name="IN2+" x="27.94" y="-5.08" length="middle" rot="R180"/>
<pin name="IN2-" x="27.94" y="0" length="middle" rot="R180"/>
<pin name="OUT2" x="27.94" y="5.08" length="middle" rot="R180"/>
<pin name="VDD" x="27.94" y="10.16" length="middle" rot="R180"/>
<polygon width="0.254" layer="94">
<vertex x="-12.7" y="7.62"/>
<vertex x="-12.7" y="-2.54"/>
<vertex x="-2.54" y="2.54"/>
</polygon>
<wire x1="-20.32" y1="0" x2="-20.32" y2="5.08" width="0.254" layer="94"/>
<wire x1="-20.32" y1="5.08" x2="-20.32" y2="10.16" width="0.254" layer="94"/>
<wire x1="-20.32" y1="10.16" x2="-20.32" y2="15.24" width="0.254" layer="94"/>
<wire x1="-20.32" y1="5.08" x2="-12.7" y2="5.08" width="0.254" layer="94"/>
<wire x1="-20.32" y1="0" x2="-12.7" y2="0" width="0.254" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="10.16" width="0.254" layer="94"/>
<wire x1="0" y1="10.16" x2="-20.32" y2="10.16" width="0.254" layer="94"/>
<polygon width="0.254" layer="94">
<vertex x="15.24" y="-7.62"/>
<vertex x="15.24" y="2.54"/>
<vertex x="5.08" y="-2.54"/>
</polygon>
<wire x1="22.86" y1="0" x2="15.24" y2="0" width="0.254" layer="94"/>
<wire x1="22.86" y1="-5.08" x2="15.24" y2="-5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="-2.54" x2="2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="2.54" y2="5.08" width="0.254" layer="94"/>
<wire x1="2.54" y1="5.08" x2="22.86" y2="5.08" width="0.254" layer="94"/>
<text x="-5.08" y="-12.7" size="1.778" layer="94">&gt;VALUE</text>
<text x="-5.08" y="17.78" size="1.778" layer="95">&gt;NAME</text>
</symbol>
<symbol name="NS5B1G385DTT1G">
<circle x="2.54" y="0.381" radius="0.381" width="0.254" layer="94"/>
<pin name="COM" x="-7.62" y="0" visible="pad" length="middle"/>
<pin name="NO" x="7.62" y="0" visible="pad" length="middle" rot="R180"/>
<wire x1="-2.54" y1="0" x2="1.27" y2="3.81" width="0.254" layer="94"/>
<pin name="IN" x="0" y="10.16" visible="pad" length="middle" rot="R270"/>
<pin name="GND" x="5.08" y="-10.16" length="short" rot="R90"/>
<pin name="VCC" x="5.08" y="10.16" length="short" rot="R270"/>
</symbol>
<symbol name="CAP">
<wire x1="0" y1="2.54" x2="0" y2="2.032" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="0.508" width="0.1524" layer="94"/>
<text x="1.524" y="2.921" size="1.778" layer="95">&gt;NAME</text>
<text x="1.524" y="-2.159" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-2.032" y1="0.508" x2="2.032" y2="1.016" layer="94"/>
<rectangle x1="-2.032" y1="1.524" x2="2.032" y2="2.032" layer="94"/>
<pin name="1" x="0" y="5.08" visible="off" length="short" direction="pas" swaplevel="1" rot="R270"/>
<pin name="2" x="0" y="-2.54" visible="off" length="short" direction="pas" swaplevel="1" rot="R90"/>
</symbol>
<symbol name="RESISTOR">
<wire x1="-2.54" y1="0" x2="-2.159" y2="1.016" width="0.1524" layer="94"/>
<wire x1="-2.159" y1="1.016" x2="-1.524" y2="-1.016" width="0.1524" layer="94"/>
<wire x1="-1.524" y1="-1.016" x2="-0.889" y2="1.016" width="0.1524" layer="94"/>
<wire x1="-0.889" y1="1.016" x2="-0.254" y2="-1.016" width="0.1524" layer="94"/>
<wire x1="-0.254" y1="-1.016" x2="0.381" y2="1.016" width="0.1524" layer="94"/>
<wire x1="0.381" y1="1.016" x2="1.016" y2="-1.016" width="0.1524" layer="94"/>
<wire x1="1.016" y1="-1.016" x2="1.651" y2="1.016" width="0.1524" layer="94"/>
<wire x1="1.651" y1="1.016" x2="2.286" y2="-1.016" width="0.1524" layer="94"/>
<wire x1="2.286" y1="-1.016" x2="2.54" y2="0" width="0.1524" layer="94"/>
<text x="-3.81" y="1.4986" size="1.778" layer="95">&gt;NAME</text>
<text x="-3.81" y="-3.302" size="1.778" layer="96">&gt;VALUE</text>
<pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
</symbol>
<symbol name="USB">
<wire x1="5.08" y1="8.89" x2="0" y2="8.89" width="0.254" layer="94"/>
<wire x1="0" y1="8.89" x2="0" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="-1.27" x2="5.08" y2="-1.27" width="0.254" layer="94"/>
<text x="3.81" y="0" size="2.54" layer="94" rot="R90">USB</text>
<pin name="D+" x="-2.54" y="7.62" visible="pad" length="short"/>
<pin name="D-" x="-2.54" y="5.08" visible="pad" length="short"/>
<pin name="VBUS" x="-2.54" y="2.54" visible="pad" length="short"/>
<pin name="GND" x="-2.54" y="0" visible="pad" length="short"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="CIM05U601NC" prefix="L">
<description>&lt;b&gt;Chip Ferrite Bead for GHz Noise&lt;/b&gt;&lt;p&gt;
</description>
<gates>
<gate name="G$1" symbol="L" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0402">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="FT240X" prefix="USB">
<description>USB to FIFO interface chip</description>
<gates>
<gate name="G$1" symbol="FT240X" x="-2.54" y="-2.54"/>
</gates>
<devices>
<device name="Q" package="QFN24-4X4">
<connects>
<connect gate="G$1" pin="3V3OUT" pad="12"/>
<connect gate="G$1" pin="CBUS5" pad="20"/>
<connect gate="G$1" pin="CBUS6" pad="19"/>
<connect gate="G$1" pin="D0" pad="21"/>
<connect gate="G$1" pin="D1" pad="1"/>
<connect gate="G$1" pin="D2" pad="23"/>
<connect gate="G$1" pin="D3" pad="6"/>
<connect gate="G$1" pin="D4" pad="22"/>
<connect gate="G$1" pin="D5" pad="4"/>
<connect gate="G$1" pin="D6" pad="5"/>
<connect gate="G$1" pin="D7" pad="2"/>
<connect gate="G$1" pin="GND" pad="3 16 EXP"/>
<connect gate="G$1" pin="RD#" pad="8"/>
<connect gate="G$1" pin="RESET#" pad="13"/>
<connect gate="G$1" pin="RXF#" pad="18"/>
<connect gate="G$1" pin="SI/WU#" pad="7"/>
<connect gate="G$1" pin="TXE#" pad="17"/>
<connect gate="G$1" pin="USBDM" pad="11"/>
<connect gate="G$1" pin="USBDP" pad="10"/>
<connect gate="G$1" pin="VCC" pad="15"/>
<connect gate="G$1" pin="VCCIO" pad="24"/>
<connect gate="G$1" pin="VCORE" pad="14"/>
<connect gate="G$1" pin="WR#" pad="9"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="TSV912" prefix="AMP">
<description>Cheep dual op-amp, rail-to-rail</description>
<gates>
<gate name="G$1" symbol="TSV912" x="-12.7" y="-2.54"/>
</gates>
<devices>
<device name="IQ2T" package="DFN6-2X3">
<connects>
<connect gate="G$1" pin="GND" pad="4 EXP"/>
<connect gate="G$1" pin="IN1+" pad="3"/>
<connect gate="G$1" pin="IN1-" pad="2"/>
<connect gate="G$1" pin="IN2+" pad="5"/>
<connect gate="G$1" pin="IN2-" pad="6"/>
<connect gate="G$1" pin="OUT1" pad="1"/>
<connect gate="G$1" pin="OUT2" pad="7"/>
<connect gate="G$1" pin="VCC" pad="8"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="TC75W57" prefix="COMP">
<gates>
<gate name="G$1" symbol="TC75W57" x="0" y="0"/>
</gates>
<devices>
<device name="FK" package="SSOP8-P-0.50A">
<connects>
<connect gate="G$1" pin="IN1+" pad="3"/>
<connect gate="G$1" pin="IN1-" pad="2"/>
<connect gate="G$1" pin="IN2+" pad="5"/>
<connect gate="G$1" pin="IN2-" pad="6"/>
<connect gate="G$1" pin="OUT1" pad="1"/>
<connect gate="G$1" pin="OUT2" pad="7"/>
<connect gate="G$1" pin="VDD" pad="8"/>
<connect gate="G$1" pin="VSS" pad="4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="NS5B1G385" prefix="SW">
<gates>
<gate name="G$1" symbol="NS5B1G385DTT1G" x="0" y="-2.54"/>
</gates>
<devices>
<device name="DTT1G" package="TSOP-5">
<connects>
<connect gate="G$1" pin="COM" pad="1"/>
<connect gate="G$1" pin="GND" pad="3"/>
<connect gate="G$1" pin="IN" pad="4"/>
<connect gate="G$1" pin="NO" pad="2"/>
<connect gate="G$1" pin="VCC" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="DFT2G" package="SC70-5L">
<connects>
<connect gate="G$1" pin="COM" pad="1"/>
<connect gate="G$1" pin="GND" pad="3"/>
<connect gate="G$1" pin="IN" pad="4"/>
<connect gate="G$1" pin="NO" pad="2"/>
<connect gate="G$1" pin="VCC" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAP" prefix="C" uservalue="yes">
<description>&lt;b&gt;Capacitor&lt;/b&gt;
Standard 0603 ceramic capacitor, and 0.1" leaded capacitor.</description>
<gates>
<gate name="G$1" symbol="CAP" x="0" y="0"/>
</gates>
<devices>
<device name="0603-CAP" package="0603-CAP">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0402-CAP" package="0402-CAP">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="RESISTOR" prefix="R" uservalue="yes">
<description>&lt;b&gt;Resistor&lt;/b&gt;
Basic schematic elements and footprints for 0603, 1206, and PTH resistors.</description>
<gates>
<gate name="G$1" symbol="RESISTOR" x="0" y="0"/>
</gates>
<devices>
<device name="0402-RES" package="0402-RES">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="USB" prefix="X">
<description>&lt;b&gt;USB Connectors&lt;/b&gt;
&lt;p&gt;USB-B-PTH is fully proven SKU : PRT-00139
&lt;p&gt;USB-miniB is fully proven SKU : PRT-00587
&lt;p&gt;USB-A-PCB is untested.
&lt;p&gt;USB-A-H is throughly reviewed, but untested. Spark Fun Electronics SKU : PRT-00437
&lt;p&gt;USB-B-SMT is throughly reviewed, but untested. Needs silkscreen touching up.
&lt;p&gt;USB-A-S has not been used/tested
&lt;p&gt;USB-MB-H has not been used/tested</description>
<gates>
<gate name="G$1" symbol="USB" x="0" y="0"/>
</gates>
<devices>
<device name="PCB" package="USB-A-PCB">
<connects>
<connect gate="G$1" pin="D+" pad="USB_P"/>
<connect gate="G$1" pin="D-" pad="USB_M"/>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="VBUS" pad="5V"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply2">
<description>&lt;b&gt;Supply Symbols&lt;/b&gt;&lt;p&gt;
GND, VCC, 0V, +5V, -5V, etc.&lt;p&gt;
Please keep in mind, that these devices are necessary for the
automatic wiring of the supply signals.&lt;p&gt;
The pin name defined in the symbol is identical to the net which is to be wired automatically.&lt;p&gt;
In this library the device names are the same as the pin names of the symbols, therefore the correct signal names appear next to the supply symbols in the schematic.&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="GND">
<wire x1="-1.27" y1="0" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="-1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<text x="-1.905" y="-3.175" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GND" prefix="SUPPLY">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="GND" symbol="GND" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="U1" library="infnoise" deviceset="FT240X" device="Q"/>
<part name="X1" library="infnoise" deviceset="USB" device="PCB"/>
<part name="C4" library="infnoise" deviceset="CAP" device="0402-CAP" value="47pF"/>
<part name="C5" library="infnoise" deviceset="CAP" device="0402-CAP" value="47pF"/>
<part name="C6" library="infnoise" deviceset="CAP" device="0402-CAP" value="100nF"/>
<part name="C2" library="infnoise" deviceset="CAP" device="0603-CAP" value="4.7uF"/>
<part name="C3" library="infnoise" deviceset="CAP" device="0402-CAP" value="100nF"/>
<part name="C7" library="infnoise" deviceset="CAP" device="0402-CAP" value="100pF"/>
<part name="C8" library="infnoise" deviceset="CAP" device="0402-CAP" value="100pF"/>
<part name="C1" library="infnoise" deviceset="CAP" device="0402-CAP" value="10nF"/>
<part name="L1" library="infnoise" deviceset="CIM05U601NC" device=""/>
<part name="R1" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="27"/>
<part name="R2" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="27"/>
<part name="R6" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="8.2K"/>
<part name="R5" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="10K"/>
<part name="R7" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="8.2K"/>
<part name="R8" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="10K"/>
<part name="R3" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="10K"/>
<part name="R4" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="10K"/>
<part name="AMP" library="infnoise" deviceset="TSV912" device="IQ2T"/>
<part name="CMP" library="infnoise" deviceset="TC75W57" device="FK"/>
<part name="SUPPLY1" library="supply2" deviceset="GND" device=""/>
<part name="R9" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="8.2K"/>
<part name="R10" library="infnoise" deviceset="RESISTOR" device="0402-RES" value="8.2K"/>
<part name="SW1" library="infnoise" deviceset="NS5B1G385" device="DFT2G"/>
<part name="SW2" library="infnoise" deviceset="NS5B1G385" device="DFT2G"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="U1" gate="G$1" x="66.04" y="38.1" rot="R270"/>
<instance part="X1" gate="G$1" x="-43.18" y="40.64" rot="R180"/>
<instance part="C4" gate="G$1" x="5.08" y="17.78"/>
<instance part="C5" gate="G$1" x="15.24" y="17.78"/>
<instance part="C6" gate="G$1" x="30.48" y="-2.54"/>
<instance part="C2" gate="G$1" x="-10.16" y="0" rot="R180"/>
<instance part="C3" gate="G$1" x="0" y="0" rot="R180"/>
<instance part="C7" gate="G$1" x="154.94" y="38.1"/>
<instance part="C8" gate="G$1" x="269.24" y="35.56" rot="R180"/>
<instance part="C1" gate="G$1" x="-20.32" y="17.78"/>
<instance part="L1" gate="G$1" x="-10.16" y="17.78" rot="R270"/>
<instance part="R1" gate="G$1" x="15.24" y="35.56"/>
<instance part="R2" gate="G$1" x="22.86" y="30.48"/>
<instance part="R6" gate="G$1" x="172.72" y="60.96"/>
<instance part="R5" gate="G$1" x="167.64" y="38.1" rot="R90"/>
<instance part="R7" gate="G$1" x="251.46" y="55.88"/>
<instance part="R8" gate="G$1" x="256.54" y="33.02" rot="R270"/>
<instance part="R3" gate="G$1" x="119.38" y="30.48" rot="R270"/>
<instance part="R4" gate="G$1" x="119.38" y="10.16" rot="R270"/>
<instance part="AMP" gate="G$1" x="208.28" y="50.8"/>
<instance part="CMP" gate="G$1" x="208.28" y="15.24"/>
<instance part="SUPPLY1" gate="GND" x="66.04" y="-15.24"/>
<instance part="R9" gate="G$1" x="299.72" y="58.42" rot="R270"/>
<instance part="R10" gate="G$1" x="299.72" y="27.94" rot="R90"/>
<instance part="SW1" gate="G$1" x="142.24" y="50.8"/>
<instance part="SW2" gate="G$1" x="281.94" y="45.72" rot="MR0"/>
</instances>
<busses>
</busses>
<nets>
<net name="USBDP" class="0">
<segment>
<pinref part="R1" gate="G$1" pin="2"/>
<pinref part="U1" gate="G$1" pin="USBDP"/>
<wire x1="20.32" y1="35.56" x2="38.1" y2="35.56" width="0.1524" layer="91"/>
</segment>
</net>
<net name="USB_M" class="0">
<segment>
<pinref part="R1" gate="G$1" pin="1"/>
<wire x1="10.16" y1="35.56" x2="5.08" y2="35.56" width="0.1524" layer="91"/>
<wire x1="5.08" y1="35.56" x2="-27.94" y2="35.56" width="0.1524" layer="91"/>
<wire x1="-27.94" y1="35.56" x2="-27.94" y2="33.02" width="0.1524" layer="91"/>
<pinref part="X1" gate="G$1" pin="D+"/>
<wire x1="-27.94" y1="33.02" x2="-40.64" y2="33.02" width="0.1524" layer="91"/>
<pinref part="C4" gate="G$1" pin="1"/>
<wire x1="5.08" y1="22.86" x2="5.08" y2="35.56" width="0.1524" layer="91"/>
<label x="-5.08" y="35.56" size="1.778" layer="95"/>
<junction x="5.08" y="35.56"/>
</segment>
</net>
<net name="USB_P" class="0">
<segment>
<pinref part="R2" gate="G$1" pin="1"/>
<wire x1="17.78" y1="30.48" x2="15.24" y2="30.48" width="0.1524" layer="91"/>
<wire x1="15.24" y1="30.48" x2="-33.02" y2="30.48" width="0.1524" layer="91"/>
<wire x1="-33.02" y1="30.48" x2="-33.02" y2="35.56" width="0.1524" layer="91"/>
<pinref part="X1" gate="G$1" pin="D-"/>
<wire x1="-33.02" y1="35.56" x2="-40.64" y2="35.56" width="0.1524" layer="91"/>
<pinref part="C5" gate="G$1" pin="1"/>
<wire x1="15.24" y1="22.86" x2="15.24" y2="30.48" width="0.1524" layer="91"/>
<label x="-5.08" y="30.48" size="1.778" layer="95"/>
<junction x="15.24" y="30.48"/>
</segment>
</net>
<net name="VCC5" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="VBUS"/>
<wire x1="-40.64" y1="38.1" x2="-20.32" y2="38.1" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="38.1" x2="-10.16" y2="38.1" width="0.1524" layer="91"/>
<pinref part="C1" gate="G$1" pin="1"/>
<wire x1="-20.32" y1="22.86" x2="-20.32" y2="38.1" width="0.1524" layer="91"/>
<junction x="-20.32" y="38.1"/>
<label x="-17.78" y="38.1" size="1.778" layer="95"/>
<pinref part="L1" gate="G$1" pin="1"/>
<wire x1="-10.16" y1="22.86" x2="-10.16" y2="38.1" width="0.1524" layer="91"/>
</segment>
</net>
<net name="VCC" class="0">
<segment>
<pinref part="U1" gate="G$1" pin="VCC"/>
<wire x1="63.5" y1="10.16" x2="63.5" y2="5.08" width="0.1524" layer="91"/>
<wire x1="63.5" y1="5.08" x2="0" y2="5.08" width="0.1524" layer="91"/>
<label x="20.32" y="7.62" size="1.778" layer="95"/>
<pinref part="C3" gate="G$1" pin="2"/>
<wire x1="0" y1="5.08" x2="-10.16" y2="5.08" width="0.1524" layer="91"/>
<wire x1="0" y1="2.54" x2="0" y2="5.08" width="0.1524" layer="91"/>
<pinref part="C2" gate="G$1" pin="2"/>
<wire x1="-10.16" y1="2.54" x2="-10.16" y2="5.08" width="0.1524" layer="91"/>
<pinref part="L1" gate="G$1" pin="2"/>
<wire x1="-10.16" y1="12.7" x2="-10.16" y2="5.08" width="0.1524" layer="91"/>
<junction x="-10.16" y="5.08"/>
<junction x="0" y="5.08"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<wire x1="30.48" y1="-10.16" x2="66.04" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="SUPPLY1" gate="GND" pin="GND"/>
<wire x1="66.04" y1="-12.7" x2="66.04" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="X1" gate="G$1" pin="GND"/>
<wire x1="-40.64" y1="40.64" x2="-25.4" y2="40.64" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="40.64" x2="-25.4" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="-10.16" x2="-20.32" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="C1" gate="G$1" pin="2"/>
<wire x1="-20.32" y1="-10.16" x2="-10.16" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="-10.16" x2="0" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="0" y1="-10.16" x2="15.24" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="15.24" y1="-10.16" x2="30.48" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="15.24" x2="-20.32" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="C6" gate="G$1" pin="2"/>
<wire x1="30.48" y1="-5.08" x2="30.48" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="C4" gate="G$1" pin="2"/>
<wire x1="5.08" y1="15.24" x2="5.08" y2="10.16" width="0.1524" layer="91"/>
<wire x1="5.08" y1="10.16" x2="15.24" y2="10.16" width="0.1524" layer="91"/>
<pinref part="C5" gate="G$1" pin="2"/>
<wire x1="15.24" y1="10.16" x2="15.24" y2="15.24" width="0.1524" layer="91"/>
<wire x1="15.24" y1="-10.16" x2="15.24" y2="10.16" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="GND"/>
<wire x1="68.58" y1="66.04" x2="68.58" y2="71.12" width="0.1524" layer="91"/>
<wire x1="68.58" y1="71.12" x2="35.56" y2="71.12" width="0.1524" layer="91"/>
<wire x1="35.56" y1="71.12" x2="-25.4" y2="71.12" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="71.12" x2="-25.4" y2="40.64" width="0.1524" layer="91"/>
<wire x1="66.04" y1="-10.16" x2="119.38" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="119.38" y1="-10.16" x2="147.32" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="147.32" y1="-10.16" x2="154.94" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="154.94" y1="-10.16" x2="180.34" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="180.34" y1="-10.16" x2="269.24" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="C7" gate="G$1" pin="2"/>
<wire x1="269.24" y1="-10.16" x2="276.86" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="154.94" y1="35.56" x2="154.94" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="147.32" y1="40.64" x2="147.32" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="AMP" gate="G$1" pin="GND"/>
<wire x1="182.88" y1="45.72" x2="180.34" y2="45.72" width="0.1524" layer="91"/>
<wire x1="180.34" y1="45.72" x2="180.34" y2="10.16" width="0.1524" layer="91"/>
<pinref part="CMP" gate="G$1" pin="VSS"/>
<wire x1="180.34" y1="10.16" x2="180.34" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="182.88" y1="10.16" x2="180.34" y2="10.16" width="0.1524" layer="91"/>
<wire x1="276.86" y1="-10.16" x2="299.72" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="R10" gate="G$1" pin="1"/>
<wire x1="299.72" y1="-10.16" x2="299.72" y2="22.86" width="0.1524" layer="91"/>
<pinref part="SW1" gate="G$1" pin="GND"/>
<pinref part="SW2" gate="G$1" pin="GND"/>
<wire x1="276.86" y1="35.56" x2="276.86" y2="-10.16" width="0.1524" layer="91"/>
<junction x="-25.4" y="40.64"/>
<junction x="-20.32" y="-10.16"/>
<junction x="15.24" y="-10.16"/>
<junction x="30.48" y="-10.16"/>
<junction x="66.04" y="-10.16"/>
<junction x="147.32" y="-10.16"/>
<junction x="154.94" y="-10.16"/>
<junction x="180.34" y="-10.16"/>
<junction x="276.86" y="-10.16"/>
<junction x="15.24" y="10.16"/>
<junction x="180.34" y="10.16"/>
<wire x1="35.56" y1="71.12" x2="35.56" y2="50.8" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="WR#"/>
<wire x1="35.56" y1="50.8" x2="35.56" y2="45.72" width="0.1524" layer="91"/>
<wire x1="35.56" y1="45.72" x2="35.56" y2="40.64" width="0.1524" layer="91"/>
<wire x1="35.56" y1="40.64" x2="38.1" y2="40.64" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="RD#"/>
<wire x1="38.1" y1="45.72" x2="35.56" y2="45.72" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="SI/WU#"/>
<wire x1="38.1" y1="50.8" x2="35.56" y2="50.8" width="0.1524" layer="91"/>
<junction x="35.56" y="71.12"/>
<junction x="35.56" y="50.8"/>
<junction x="35.56" y="45.72"/>
<pinref part="C2" gate="G$1" pin="1"/>
<wire x1="-10.16" y1="-5.08" x2="-10.16" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="C3" gate="G$1" pin="1"/>
<wire x1="0" y1="-5.08" x2="0" y2="-10.16" width="0.1524" layer="91"/>
<junction x="-10.16" y="-10.16"/>
<junction x="0" y="-10.16"/>
<pinref part="C8" gate="G$1" pin="1"/>
<wire x1="269.24" y1="30.48" x2="269.24" y2="-10.16" width="0.1524" layer="91"/>
<junction x="269.24" y="-10.16"/>
<pinref part="R4" gate="G$1" pin="2"/>
<wire x1="119.38" y1="5.08" x2="119.38" y2="-10.16" width="0.1524" layer="91"/>
<junction x="119.38" y="-10.16"/>
</segment>
</net>
<net name="VCC33" class="0">
<segment>
<pinref part="U1" gate="G$1" pin="VCCIO"/>
<wire x1="93.98" y1="50.8" x2="104.14" y2="50.8" width="0.1524" layer="91"/>
<wire x1="104.14" y1="50.8" x2="104.14" y2="88.9" width="0.1524" layer="91"/>
<wire x1="104.14" y1="88.9" x2="119.38" y2="88.9" width="0.1524" layer="91"/>
<wire x1="119.38" y1="88.9" x2="147.32" y2="88.9" width="0.1524" layer="91"/>
<wire x1="30.48" y1="88.9" x2="104.14" y2="88.9" width="0.1524" layer="91"/>
<wire x1="147.32" y1="60.96" x2="147.32" y2="88.9" width="0.1524" layer="91"/>
<wire x1="147.32" y1="88.9" x2="238.76" y2="88.9" width="0.1524" layer="91"/>
<pinref part="CMP" gate="G$1" pin="VDD"/>
<wire x1="238.76" y1="88.9" x2="276.86" y2="88.9" width="0.1524" layer="91"/>
<wire x1="276.86" y1="88.9" x2="299.72" y2="88.9" width="0.1524" layer="91"/>
<wire x1="236.22" y1="25.4" x2="238.76" y2="25.4" width="0.1524" layer="91"/>
<wire x1="238.76" y1="25.4" x2="238.76" y2="60.96" width="0.1524" layer="91"/>
<pinref part="AMP" gate="G$1" pin="VCC"/>
<wire x1="238.76" y1="60.96" x2="238.76" y2="88.9" width="0.1524" layer="91"/>
<wire x1="236.22" y1="60.96" x2="238.76" y2="60.96" width="0.1524" layer="91"/>
<pinref part="SW1" gate="G$1" pin="VCC"/>
<pinref part="SW2" gate="G$1" pin="VCC"/>
<wire x1="276.86" y1="55.88" x2="276.86" y2="88.9" width="0.1524" layer="91"/>
<junction x="276.86" y="88.9"/>
<junction x="238.76" y="88.9"/>
<junction x="147.32" y="88.9"/>
<junction x="104.14" y="88.9"/>
<junction x="238.76" y="60.96"/>
<pinref part="U1" gate="G$1" pin="3V3OUT"/>
<wire x1="30.48" y1="25.4" x2="38.1" y2="25.4" width="0.1524" layer="91"/>
<wire x1="30.48" y1="10.16" x2="30.48" y2="25.4" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="RESET#"/>
<wire x1="53.34" y1="10.16" x2="30.48" y2="10.16" width="0.1524" layer="91"/>
<junction x="30.48" y="10.16"/>
<pinref part="C6" gate="G$1" pin="1"/>
<wire x1="30.48" y1="2.54" x2="30.48" y2="10.16" width="0.1524" layer="91"/>
<wire x1="30.48" y1="25.4" x2="30.48" y2="88.9" width="0.1524" layer="91"/>
<junction x="30.48" y="25.4"/>
<pinref part="R3" gate="G$1" pin="1"/>
<wire x1="119.38" y1="35.56" x2="119.38" y2="88.9" width="0.1524" layer="91"/>
<junction x="119.38" y="88.9"/>
<pinref part="R9" gate="G$1" pin="1"/>
<wire x1="299.72" y1="88.9" x2="299.72" y2="63.5" width="0.1524" layer="91"/>
</segment>
</net>
<net name="A" class="0">
<segment>
<pinref part="AMP" gate="G$1" pin="IN1+"/>
<wire x1="182.88" y1="50.8" x2="162.56" y2="50.8" width="0.1524" layer="91"/>
<pinref part="C7" gate="G$1" pin="1"/>
<wire x1="162.56" y1="50.8" x2="154.94" y2="50.8" width="0.1524" layer="91"/>
<wire x1="154.94" y1="50.8" x2="149.86" y2="50.8" width="0.1524" layer="91"/>
<wire x1="154.94" y1="43.18" x2="154.94" y2="50.8" width="0.1524" layer="91"/>
<pinref part="CMP" gate="G$1" pin="IN1+"/>
<wire x1="182.88" y1="15.24" x2="162.56" y2="15.24" width="0.1524" layer="91"/>
<wire x1="162.56" y1="15.24" x2="162.56" y2="50.8" width="0.1524" layer="91"/>
<pinref part="SW1" gate="G$1" pin="NO"/>
<junction x="154.94" y="50.8"/>
<junction x="162.56" y="50.8"/>
<label x="157.48" y="50.8" size="1.778" layer="95"/>
</segment>
</net>
<net name="AMP1OUT" class="0">
<segment>
<wire x1="180.34" y1="76.2" x2="292.1" y2="76.2" width="0.1524" layer="91"/>
<pinref part="R6" gate="G$1" pin="2"/>
<pinref part="AMP" gate="G$1" pin="OUT1"/>
<wire x1="182.88" y1="60.96" x2="180.34" y2="60.96" width="0.1524" layer="91"/>
<wire x1="180.34" y1="60.96" x2="177.8" y2="60.96" width="0.1524" layer="91"/>
<wire x1="180.34" y1="60.96" x2="180.34" y2="76.2" width="0.1524" layer="91"/>
<junction x="180.34" y="60.96"/>
<pinref part="SW2" gate="G$1" pin="COM"/>
<wire x1="289.56" y1="45.72" x2="292.1" y2="45.72" width="0.1524" layer="91"/>
<pinref part="R10" gate="G$1" pin="2"/>
<wire x1="292.1" y1="45.72" x2="299.72" y2="45.72" width="0.1524" layer="91"/>
<wire x1="299.72" y1="33.02" x2="299.72" y2="45.72" width="0.1524" layer="91"/>
<wire x1="292.1" y1="76.2" x2="292.1" y2="45.72" width="0.1524" layer="91"/>
<junction x="292.1" y="45.72"/>
<pinref part="R9" gate="G$1" pin="2"/>
<wire x1="299.72" y1="53.34" x2="299.72" y2="45.72" width="0.1524" layer="91"/>
<junction x="299.72" y="45.72"/>
</segment>
</net>
<net name="FB1" class="0">
<segment>
<pinref part="AMP" gate="G$1" pin="IN1-"/>
<wire x1="182.88" y1="55.88" x2="167.64" y2="55.88" width="0.1524" layer="91"/>
<wire x1="167.64" y1="55.88" x2="167.64" y2="60.96" width="0.1524" layer="91"/>
<pinref part="R6" gate="G$1" pin="1"/>
<pinref part="R5" gate="G$1" pin="2"/>
<wire x1="167.64" y1="43.18" x2="167.64" y2="55.88" width="0.1524" layer="91"/>
<junction x="167.64" y="55.88"/>
</segment>
</net>
<net name="AMP2OUT" class="0">
<segment>
<wire x1="243.84" y1="71.12" x2="132.08" y2="71.12" width="0.1524" layer="91"/>
<wire x1="132.08" y1="71.12" x2="132.08" y2="50.8" width="0.1524" layer="91"/>
<wire x1="132.08" y1="50.8" x2="134.62" y2="50.8" width="0.1524" layer="91"/>
<pinref part="SW1" gate="G$1" pin="COM"/>
<pinref part="AMP" gate="G$1" pin="OUT2"/>
<pinref part="R7" gate="G$1" pin="1"/>
<wire x1="246.38" y1="55.88" x2="243.84" y2="55.88" width="0.1524" layer="91"/>
<wire x1="243.84" y1="55.88" x2="236.22" y2="55.88" width="0.1524" layer="91"/>
<wire x1="243.84" y1="55.88" x2="243.84" y2="71.12" width="0.1524" layer="91"/>
<junction x="243.84" y="55.88"/>
</segment>
</net>
<net name="FB2" class="0">
<segment>
<pinref part="R7" gate="G$1" pin="2"/>
<pinref part="AMP" gate="G$1" pin="IN2-"/>
<wire x1="236.22" y1="50.8" x2="256.54" y2="50.8" width="0.1524" layer="91"/>
<wire x1="256.54" y1="50.8" x2="256.54" y2="55.88" width="0.1524" layer="91"/>
<pinref part="R8" gate="G$1" pin="1"/>
<wire x1="256.54" y1="38.1" x2="256.54" y2="50.8" width="0.1524" layer="91"/>
<junction x="256.54" y="50.8"/>
</segment>
</net>
<net name="B" class="0">
<segment>
<pinref part="AMP" gate="G$1" pin="IN2+"/>
<wire x1="274.32" y1="45.72" x2="269.24" y2="45.72" width="0.1524" layer="91"/>
<wire x1="269.24" y1="45.72" x2="261.62" y2="45.72" width="0.1524" layer="91"/>
<wire x1="261.62" y1="45.72" x2="236.22" y2="45.72" width="0.1524" layer="91"/>
<pinref part="CMP" gate="G$1" pin="IN2+"/>
<wire x1="236.22" y1="10.16" x2="261.62" y2="10.16" width="0.1524" layer="91"/>
<wire x1="261.62" y1="10.16" x2="261.62" y2="45.72" width="0.1524" layer="91"/>
<pinref part="SW2" gate="G$1" pin="NO"/>
<junction x="261.62" y="45.72"/>
<label x="243.84" y="45.72" size="1.778" layer="95"/>
<pinref part="C8" gate="G$1" pin="2"/>
<wire x1="269.24" y1="38.1" x2="269.24" y2="45.72" width="0.1524" layer="91"/>
<junction x="269.24" y="45.72"/>
</segment>
</net>
<net name="COMP1" class="0">
<segment>
<pinref part="CMP" gate="G$1" pin="OUT1"/>
<wire x1="182.88" y1="25.4" x2="167.64" y2="25.4" width="0.1524" layer="91"/>
<wire x1="167.64" y1="25.4" x2="132.08" y2="25.4" width="0.1524" layer="91"/>
<wire x1="132.08" y1="25.4" x2="132.08" y2="45.72" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="D2"/>
<wire x1="132.08" y1="45.72" x2="93.98" y2="45.72" width="0.1524" layer="91"/>
<label x="96.52" y="45.72" size="1.778" layer="95"/>
<pinref part="R5" gate="G$1" pin="1"/>
<wire x1="167.64" y1="33.02" x2="167.64" y2="25.4" width="0.1524" layer="91"/>
<junction x="167.64" y="25.4"/>
</segment>
</net>
<net name="COMP2" class="0">
<segment>
<pinref part="CMP" gate="G$1" pin="OUT2"/>
<wire x1="236.22" y1="20.32" x2="256.54" y2="20.32" width="0.1524" layer="91"/>
<wire x1="256.54" y1="20.32" x2="256.54" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="256.54" y1="-5.08" x2="104.14" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="104.14" y1="-5.08" x2="104.14" y2="35.56" width="0.1524" layer="91"/>
<pinref part="U1" gate="G$1" pin="D0"/>
<wire x1="104.14" y1="35.56" x2="93.98" y2="35.56" width="0.1524" layer="91"/>
<label x="96.52" y="35.56" size="1.778" layer="95"/>
<pinref part="R8" gate="G$1" pin="2"/>
<wire x1="256.54" y1="27.94" x2="256.54" y2="20.32" width="0.1524" layer="91"/>
<junction x="256.54" y="20.32"/>
</segment>
</net>
<net name="VREF" class="0">
<segment>
<pinref part="CMP" gate="G$1" pin="IN1-"/>
<wire x1="182.88" y1="20.32" x2="160.02" y2="20.32" width="0.1524" layer="91"/>
<wire x1="160.02" y1="20.32" x2="160.02" y2="0" width="0.1524" layer="91"/>
<wire x1="160.02" y1="0" x2="238.76" y2="0" width="0.1524" layer="91"/>
<wire x1="238.76" y1="0" x2="238.76" y2="15.24" width="0.1524" layer="91"/>
<pinref part="CMP" gate="G$1" pin="IN2-"/>
<wire x1="238.76" y1="15.24" x2="236.22" y2="15.24" width="0.1524" layer="91"/>
<wire x1="160.02" y1="20.32" x2="119.38" y2="20.32" width="0.1524" layer="91"/>
<junction x="160.02" y="20.32"/>
<label x="124.46" y="20.32" size="1.778" layer="95"/>
<pinref part="R4" gate="G$1" pin="1"/>
<wire x1="119.38" y1="15.24" x2="119.38" y2="20.32" width="0.1524" layer="91"/>
<pinref part="R3" gate="G$1" pin="2"/>
<wire x1="119.38" y1="25.4" x2="119.38" y2="20.32" width="0.1524" layer="91"/>
<junction x="119.38" y="20.32"/>
</segment>
</net>
<net name="SW1EN" class="0">
<segment>
<wire x1="111.76" y1="63.5" x2="142.24" y2="63.5" width="0.1524" layer="91"/>
<wire x1="142.24" y1="63.5" x2="142.24" y2="60.96" width="0.1524" layer="91"/>
<wire x1="111.76" y1="63.5" x2="111.76" y2="40.64" width="0.1524" layer="91"/>
<pinref part="SW1" gate="G$1" pin="IN"/>
<pinref part="U1" gate="G$1" pin="D4"/>
<wire x1="111.76" y1="40.64" x2="93.98" y2="40.64" width="0.1524" layer="91"/>
<label x="96.52" y="40.64" size="1.778" layer="95"/>
</segment>
</net>
<net name="SW2EN" class="0">
<segment>
<pinref part="U1" gate="G$1" pin="D1"/>
<wire x1="78.74" y1="66.04" x2="78.74" y2="83.82" width="0.1524" layer="91"/>
<wire x1="78.74" y1="83.82" x2="281.94" y2="83.82" width="0.1524" layer="91"/>
<wire x1="281.94" y1="83.82" x2="281.94" y2="55.88" width="0.1524" layer="91"/>
<pinref part="SW2" gate="G$1" pin="IN"/>
<label x="81.28" y="83.82" size="1.778" layer="95"/>
</segment>
</net>
<net name="USBDM" class="0">
<segment>
<pinref part="R2" gate="G$1" pin="2"/>
<pinref part="U1" gate="G$1" pin="USBDM"/>
<wire x1="27.94" y1="30.48" x2="38.1" y2="30.48" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
