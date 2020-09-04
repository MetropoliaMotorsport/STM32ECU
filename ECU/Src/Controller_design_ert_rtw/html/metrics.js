function CodeMetrics() {
	 this.metricsArray = {};
	 this.metricsArray.var = new Array();
	 this.metricsArray.fcn = new Array();
	 this.metricsArray.var["rtInf"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 8};
	 this.metricsArray.var["rtInfF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 4};
	 this.metricsArray.var["rtM_"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 4};
	 this.metricsArray.var["rtMinusInf"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 8};
	 this.metricsArray.var["rtMinusInfF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 4};
	 this.metricsArray.var["rtNaN"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 8};
	 this.metricsArray.var["rtNaNF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 4};
	 this.metricsArray.var["rtU"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 32};
	 this.metricsArray.var["rtY"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	size: 16};
	 this.metricsArray.fcn["Controller_design.c:look1_binlx"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 20,
	stackTotal: 20};
	 this.metricsArray.fcn["Controller_design.c:rtGetInf"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 20,
	stackTotal: 24};
	 this.metricsArray.fcn["Controller_design.c:rtGetInfF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["Controller_design.c:rtGetMinusInf"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 20,
	stackTotal: 24};
	 this.metricsArray.fcn["Controller_design.c:rtGetMinusInfF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["Controller_design.c:rtGetNaN"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 20,
	stackTotal: 24};
	 this.metricsArray.fcn["Controller_design.c:rtGetNaNF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["Controller_design.c:rtIsInf"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 0,
	stackTotal: 0};
	 this.metricsArray.fcn["Controller_design.c:rtIsInfF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 0,
	stackTotal: 0};
	 this.metricsArray.fcn["Controller_design.c:rtIsNaN"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 13,
	stackTotal: 17};
	 this.metricsArray.fcn["Controller_design.c:rtIsNaNF"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["Controller_design.c:rt_InitInfAndNaN"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 0,
	stackTotal: 24};
	 this.metricsArray.fcn["Controller_design_initialize"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 0,
	stackTotal: 24};
	 this.metricsArray.fcn["Controller_design_step"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 16,
	stackTotal: 36};
	 this.metricsArray.fcn["fmax"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 0,
	stackTotal: 0};
	 this.metricsArray.fcn["fmin"] = {file: "G:\\Omat tiedostot\\Controller_design_ert_rtw\\Controller_design.c",
	stack: 0,
	stackTotal: 0};
	 this.getMetrics = function(token) { 
		 var data;
		 data = this.metricsArray.var[token];
		 if (!data) {
			 data = this.metricsArray.fcn[token];
			 if (data) data.type = "fcn";
		 } else { 
			 data.type = "var";
		 }
	 return data; }; 
	 this.codeMetricsSummary = '<a href="Controller_design_metrics.html">Global Memory: 88(bytes) Maximum Stack: 20(bytes)</a>';
	}
CodeMetrics.instance = new CodeMetrics();
