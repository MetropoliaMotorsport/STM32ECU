function CodeDefine() { 
this.def = new Array();
this.def["rt_OneStep"] = {file: "ert_main_c.html",line:39,type:"fcn"};
this.def["main"] = {file: "ert_main_c.html",line:76,type:"fcn"};
this.def["rtU"] = {file: "Controller_design_c.html",line:26,type:"var"};
this.def["rtY"] = {file: "Controller_design_c.html",line:29,type:"var"};
this.def["rtM_"] = {file: "Controller_design_c.html",line:32,type:"var"};
this.def["rtM"] = {file: "Controller_design_c.html",line:33,type:"var"};
this.def["BigEndianIEEEDouble"] = {file: "Controller_design_c.html",line:54,type:"type"};
this.def["LittleEndianIEEEDouble"] = {file: "Controller_design_c.html",line:61,type:"type"};
this.def["IEEESingle"] = {file: "Controller_design_c.html",line:68,type:"type"};
this.def["rtInf"] = {file: "Controller_design_c.html",line:70,type:"var"};
this.def["rtMinusInf"] = {file: "Controller_design_c.html",line:71,type:"var"};
this.def["rtNaN"] = {file: "Controller_design_c.html",line:72,type:"var"};
this.def["rtInfF"] = {file: "Controller_design_c.html",line:73,type:"var"};
this.def["rtMinusInfF"] = {file: "Controller_design_c.html",line:74,type:"var"};
this.def["rtNaNF"] = {file: "Controller_design_c.html",line:75,type:"var"};
this.def["Controller_design.c:rtGetNaN"] = {file: "Controller_design_c.html",line:85,type:"fcn"};
this.def["Controller_design.c:rtGetNaNF"] = {file: "Controller_design_c.html",line:109,type:"fcn"};
this.def["Controller_design.c:rt_InitInfAndNaN"] = {file: "Controller_design_c.html",line:121,type:"fcn"};
this.def["Controller_design.c:rtIsInf"] = {file: "Controller_design_c.html",line:133,type:"fcn"};
this.def["Controller_design.c:rtIsInfF"] = {file: "Controller_design_c.html",line:139,type:"fcn"};
this.def["Controller_design.c:rtIsNaN"] = {file: "Controller_design_c.html",line:145,type:"fcn"};
this.def["Controller_design.c:rtIsNaNF"] = {file: "Controller_design_c.html",line:167,type:"fcn"};
this.def["Controller_design.c:rtGetInf"] = {file: "Controller_design_c.html",line:179,type:"fcn"};
this.def["Controller_design.c:rtGetInfF"] = {file: "Controller_design_c.html",line:203,type:"fcn"};
this.def["Controller_design.c:rtGetMinusInf"] = {file: "Controller_design_c.html",line:214,type:"fcn"};
this.def["Controller_design.c:rtGetMinusInfF"] = {file: "Controller_design_c.html",line:238,type:"fcn"};
this.def["Controller_design.c:look1_binlx"] = {file: "Controller_design_c.html",line:245,type:"fcn"};
this.def["Controller_design_step"] = {file: "Controller_design_c.html",line:301,type:"fcn"};
this.def["Controller_design_initialize"] = {file: "Controller_design_c.html",line:397,type:"fcn"};
this.def["RT_MODEL"] = {file: "Controller_design_h.html",line:41,type:"type"};
this.def["ConstP"] = {file: "Controller_design_h.html",line:54,type:"type"};
this.def["ExtU"] = {file: "Controller_design_h.html",line:62,type:"type"};
this.def["ExtY"] = {file: "Controller_design_h.html",line:68,type:"type"};
this.def["rtConstP"] = {file: "Controller_design_data_c.html",line:25,type:"var"};
this.def["int8_T"] = {file: "rtwtypes_h.html",line:53,type:"type"};
this.def["uint8_T"] = {file: "rtwtypes_h.html",line:54,type:"type"};
this.def["int16_T"] = {file: "rtwtypes_h.html",line:55,type:"type"};
this.def["uint16_T"] = {file: "rtwtypes_h.html",line:56,type:"type"};
this.def["int32_T"] = {file: "rtwtypes_h.html",line:57,type:"type"};
this.def["uint32_T"] = {file: "rtwtypes_h.html",line:58,type:"type"};
this.def["int64_T"] = {file: "rtwtypes_h.html",line:59,type:"type"};
this.def["uint64_T"] = {file: "rtwtypes_h.html",line:60,type:"type"};
this.def["real32_T"] = {file: "rtwtypes_h.html",line:61,type:"type"};
this.def["real64_T"] = {file: "rtwtypes_h.html",line:62,type:"type"};
this.def["real_T"] = {file: "rtwtypes_h.html",line:68,type:"type"};
this.def["time_T"] = {file: "rtwtypes_h.html",line:69,type:"type"};
this.def["boolean_T"] = {file: "rtwtypes_h.html",line:70,type:"type"};
this.def["int_T"] = {file: "rtwtypes_h.html",line:71,type:"type"};
this.def["uint_T"] = {file: "rtwtypes_h.html",line:72,type:"type"};
this.def["ulong_T"] = {file: "rtwtypes_h.html",line:73,type:"type"};
this.def["ulonglong_T"] = {file: "rtwtypes_h.html",line:74,type:"type"};
this.def["char_T"] = {file: "rtwtypes_h.html",line:75,type:"type"};
this.def["uchar_T"] = {file: "rtwtypes_h.html",line:76,type:"type"};
this.def["byte_T"] = {file: "rtwtypes_h.html",line:77,type:"type"};
this.def["pointer_T"] = {file: "rtwtypes_h.html",line:98,type:"type"};
}
CodeDefine.instance = new CodeDefine();
var testHarnessInfo = {OwnerFileName: "", HarnessOwner: "", HarnessName: "", IsTestHarness: "0"};
var relPathToBuildDir = "../ert_main.c";
var fileSep = "\\";
var isPC = true;
function Html2SrcLink() {
	this.html2SrcPath = new Array;
	this.html2Root = new Array;
	this.html2SrcPath["ert_main_c.html"] = "../ert_main.c";
	this.html2Root["ert_main_c.html"] = "ert_main_c.html";
	this.html2SrcPath["Controller_design_c.html"] = "../Controller_design.c";
	this.html2Root["Controller_design_c.html"] = "Controller_design_c.html";
	this.html2SrcPath["Controller_design_h.html"] = "../Controller_design.h";
	this.html2Root["Controller_design_h.html"] = "Controller_design_h.html";
	this.html2SrcPath["Controller_design_data_c.html"] = "../Controller_design_data.c";
	this.html2Root["Controller_design_data_c.html"] = "Controller_design_data_c.html";
	this.html2SrcPath["rtwtypes_h.html"] = "../rtwtypes.h";
	this.html2Root["rtwtypes_h.html"] = "rtwtypes_h.html";
	this.getLink2Src = function (htmlFileName) {
		 if (this.html2SrcPath[htmlFileName])
			 return this.html2SrcPath[htmlFileName];
		 else
			 return null;
	}
	this.getLinkFromRoot = function (htmlFileName) {
		 if (this.html2Root[htmlFileName])
			 return this.html2Root[htmlFileName];
		 else
			 return null;
	}
}
Html2SrcLink.instance = new Html2SrcLink();
var fileList = [
"ert_main_c.html","Controller_design_c.html","Controller_design_h.html","Controller_design_data_c.html","rtwtypes_h.html"];
